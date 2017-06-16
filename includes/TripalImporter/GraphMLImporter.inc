<?php

class GraphMLImporter extends TripalImporter {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Chado GraphML Loader';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'gml_loader';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Import a biological network from a file in GraphML format.';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = array();


  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = '';

  /**
   * The title that should appear above the upload button.
   */
  public static $upload_title = 'File Upload';

  /**
   * If the loader should require an analysis record.  To maintain provenance
   * we should always indiate where the data we are uploading comes from.
   * The method that Tripal attempts to use for this by associating upload files
   * with an analysis record.  The analysis record provides the details for
   * how the file was created or obtained. Set this to FALSE if the loader
   * should not require an analysis when loading. if $use_analysis is set to
   * true then the form values will have an 'analysis_id' key in the $form_state
   * array on submitted forms.
   */
  public static $use_analysis = TRUE;

  /**
   * If the $use_analysis value is set above then this value indicates if the
   * analysis should be required.
   */
  public static $require_analysis = TRUE;

  /**
   * Text that should appear on the button at the bottom of the importer
   * form.
   */
  public static $button_text = 'Import File';

  /**
   * Indicates the methods that the file uploader will support.
   */
  public static $methods = array(
    // Allow the user to upload a file to the server.
    'file_upload' => TRUE,
    // Allow the user to provide the path on the Tripal server for the file.
    'file_local' => TRUE,
    // Allow the user to provide a remote URL for the file.
    'file_remote' => TRUE,
  );

  /**
   * Indicates if the file must be provided.  An example when it may not be
   * necessary to require that the user provide a file for uploading if the
   * loader keeps track of previous files and makes those available for
   * selection.
   */
  public static $file_required = TRUE;


  /**
   * The array of arguments used for this loader.  Each argument should
   * be a separate array containing a machine_name, name, and description
   * keys.  This information is used to build the help text for the loader.
   */
  public static $argument_list = array();


  /**
   * See TripalImporter::form)
   */
  public function form($form, &$form_state) {

      $organisms = tripal_get_organism_select_options(FALSE);
      $form['organism_id'] = array(
        '#title'       => t('Organism'),
        '#type'        => t('select'),
        '#description' => t("Choose the organism to which the network file is associated"),
        '#required'    => TRUE,
        '#options'     => $organisms,
      );

      return $form;
  }

  /**
   * See TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state) {

    $graphML_file = $form_state['values']['graphML_file'];
    $organism_id = $form_state['values']['organism_id'];

    // check to see if the file is located local to Drupal
    $graphML_file = trim($graphML_file);
    $dfile = $_SERVER['DOCUMENT_ROOT'] . base_path() . $graphML_file;
    if (!file_exists($dfile)) {
      // if not local to Drupal, the file must be someplace else, just use
      // the full path provided
      $dfile = $graphML_file;
    }
    if (!file_exists($dfile)) {
      form_set_error('graphML_file', t("Cannot find the file on the system. Check that ".
          "the file exists or that the web server has permissions to read the file."));
    }
  }

  /**
   * see TripalImporter::run()
   */
  public function run() {
    $arguments = $this->arguments['run_args'];
    $file_path = $this->arguments['file']['file_path'];

    $graphML_file = $file_path;
    $organism_id = $arguments['organism_id'];
    $analysis_id = $arguments['analysis_id'];

    // Get the organism.
    $values = array('organism_id' => $organism_id);
    $organism = chado_select_record('organism', array('organism_id'), $values);
    if (count($organism) == 0) {
      throw new Exception(t("Cannot find organism with id: %organism_id",
          array('%organism_id' => $organism_id)));
    }
    $organism = $organism[0];

    // Load the XML file
    $this->logMessage("Opening !graphML_file...", array(':graphML_file' => $graphML_file));
    $graphml = new XMLReader();

    // first iterate through all of the nodes and count them
    $graphml->open($graphML_file);
    $total_nodes = 0;
    while ($graphml->read()){
      $total_nodes++;
    }
    $graphml->close();

    $this->setTotalItems($total_nodes);
    $this->setItemsHandled(0);

    // now reopen the GML file and parse the input
    $nodes_read = 0;
    $attributes = array();
    $graphml->open($graphML_file);
    while ($graphml->read()){
      $nodes_read++;
      if ($graphml->nodeType == XMLReader::ELEMENT) {

        // if this is a 'key' tag then get the network attributes
        if (strcmp($graphml->name, 'key')==0) {

          // get the network attribute types
          $id = $graphml->getAttribute('id');
          $for = $graphml->getAttribute('for');
          $name = $graphml->getAttribute('attr.name');
          if (!array_key_exists($for, $attributes)) {
            $attributes[$for] = array();
          }
          $attributes[$for][] = $name;
        }

        // if this is a graph then parse the graph
        elseif (strcmp($graphml->name, 'graph')==0) {
          $this->addGraph($graphml, $attributes, $organism, $nodes_read, $mod_type);
        }
      }
    }
    $graphml->close();

    // Set the final progress for loading the graph.
    $this->setItemsHandled($nodes_read);

  }


  /**
   * Imports the contents of the 'graph' section of the GraphML file.
   *
   * @param $graphml
   *   An XMLReader object of the 'graph' section.
   * @param $attributes
   *   An associative array containing the attributes of the graph.
   * @param $organism
   *   An organism object record.
   * @param $nodes_read
   *   A count of the number of nodes that have been read. This argument
   *   is passed by reference and is updated by this call.
   * @param $mod_type
   *   A cvterm object record representing the controlled vocabulary term
   *   for the network module type.
   */
  private function addGraph($graphml, $attributes,
      $organism, &$nodes_read, $mod_type) {

    // First get the graph attributes and ase the id as the default network name.
    $id = $graphml->getAttribute('id');
    $edgedefault = $graphml->getAttribute('edgedefault');
    if (!$edgedefault) {
      $edgedefault = 'undirected';
    }

    // Make sure the network exists, if not then add it.
    $network_name = $id;
    $values = array(
      'uniquename' => $network_name,
      'organism_id' => $organism->organism_id,
      'type_id' => array(
        'name' => 'co-expression',
        'cv_id' => array(
          'name' => 'network_type',
        )
      ),
    );
    $network = chado_select_record('network', array('network_id'), $values);
    if(count($network) == 0){
      // the network doesn't exist so let's add it
      $ins_values = $values;
      $ins_values['name'] = $network_name;
      $network = chado_insert_record('network', $ins_values);
      if (!array_key_exists('network_id', $result)) {
        throw new Exception(t("Cannot add network to database: %network",
            array('%network' => $network)));
      }
      $network = (object) $network;
    }
    else {
      $network = $network[0];
    }

    // Now iterate through the nodes and edges and add them to the database.
    while ($graphml->read()) {
      $nodes_read++;

      if (strcmp($graphml->name, 'node') == 0) {
        $this->addNode($graphml, $attributes, $network, $organism, $nodes_read);
      }
      elseif (strcmp($graphml->name, 'edge') == 0) {
        $this->addEdge($graphml, $attributes, $network, $organism, $nodes_read, $mod_type);
      }

      // Update our progress.
      $this->setItemsHandled($nodes_read);
    }
  }
  /**
   * Adds a node in the graph.
   *
   * @param $graphml
   *   An XMLReader object of the 'graph' section.
   * @param $attributes
   *   An associative array containing the attributes of the node.
   * @param $network
   *   An network record object.
   * @param $organism
   *   An organism record object.
   * @param $nodes_read
   *   A count of the number of nodes that have been read. This argument
   *   is passed by reference and is updated by this call.
   */
  private function addNode($graphml, $attributes,
      $default_network, $organism, &$nodes_read) {

    // First get the graph attributes.
    $id = $graphml->getAttribute('id');

    // Now iterate through data tags.
    while ($graphml->read()){
      $nodes_read++;
      if (strcmp($graphml->name, 'data')==0) {
        if ($graphml->nodeType == XMLReader::ELEMENT) {
          // get node details
        }
      }

      // If we're at the last sub node of this tree then we have all of our node
      // properties and we can break out and handle the node.
      elseif ($graphml->nodeType == XMLReader::END_ELEMENT) {
        break;
      }
    }

    // Make sure the node exists in the feature table.
    $values = array(
      'name' => $id,
      'organism_id' => $organism->organism_id,
    );
    $feature = chado_select_record('feature', array('feature_id'), $values);
    if (count($feature) == 0){
      // We couldn't find the feature by name so look for this feature
      // as a synonym.  This query is more complex so we can't use the
      // chado_select_record function.
      $sql = "
     SELECT F.feature_id
     FROM synonym S
       INNER JOIN feature_synonym FS on FS.synonym_id = S.synonym_id
       INNER JOIN feature F on FS.feature_id = F.feature_id
     WHERE S.name = :name
    ";
      $feature = chado_query($sql, array(':name' => $id));

      if (!$feature) {
        throw new Exception(t("Cannot find the node in the feature table or as a synonym: %node",
            array('%node' => $id)));
      }
    }
    elseif(count($feature) > 1) {
      throw new Exception(t("Too many features for this organism have the name of: %node. " .
          "Cannot determine which to use ",   array('%node' => $id)));
    }

    // Check if the node is present in the network node table, if not add it.
    $values = array('node' =>  $feature->feature_id);
    $result = chado_select_record('networknode', array('node'), $values);
    if (count($result) == 0) {
      $values = array(
        'network_id' => $network->network_id,
        'node' => $feature->feature_id,
      );
      $success = chado_insert_record('networknode', $values);
      if (!$success) {
        throw new Exception(t("Cannot add node, %node, to network, %network",
            array('%node' => $feature->uniquename, '%network' => $network->name)));
      }
    }
  }
  /**
   * Imports an edge.
   *
   * @param $graphml
   *   An XMLReader object of the 'graph' section.
   * @param $attributes
   *   An associative array containing the attributes of the node.
   * @param $network
   *   An network record object.
   * @param $organism
   *   An organism record object.
   * @param $nodes_read
   *   A count of the number of nodes that have been read. This argument
   *   is passed by reference and is updated by this call.
   * @param $mod_type
   *   A cvterm object record representing the controlled vocabulary term
   *   for the network module type.
   */
  private function addEdge($graphml, $attributes,
      $network, $organism, &$nodes_read, $mod_type) {

    // First get the graph attributes.
    $node1 = $graphml->getAttribute('source');
    $node2 = $graphml->getAttribute('target');

    // Now iterate through data tags and get the attributes for this edge.
    $weight = 0;
    $network_name = $default_network;
    $module = '';
    $direction = 0;
    while ($graphml->read()){
      $nodes_read++;
      if (strcmp($graphml->name, 'data')==0) {
        if ($graphml->nodeType == XMLReader::ELEMENT) {  // <data>

          // Get the proprety type (key) and its value.
          $key = $graphml->getAttribute('key');
          $graphml->read();
          $nodes_read++;
          $value = $graphml->value;

          // First look for the attributes that we support.
          if (strcmp($key, 'weight') == 0){
            $weight = $value;
          }
          if (strcmp($key, 'network') == 0){
            $network_name = $value;
          }
          if (strcmp($key, 'module') == 0){
            $module = $value;
          }
        }
      }
      // If we're at the last sub node of this tree then we have all of our
      // edge properties and we can break out and add the edges.
      elseif ($graphml->nodeType == XMLReader::END_ELEMENT) {
        break;
      }
    }

    // Get node1.
    $values = array(
      'name' => $node1,
      'organism_id' => $organism->organism_id,
    );
    $feature1 = chado_select_record('feature', array('feature_id'), $values);
    $feature1 = $feature1[0];

    // Get node2.
    $values = array(
      'name' => $node2,
      'organism_id' => $organism->organism_id,
    );
    $feature2 = chado_select_record('feature', array('feature_id'), $values);
    $feature2 = $feature2[0];

    // Now add in the edge if it doesn't already exist.
    $values = array(
      'network_id' => $network->network_id,
      'node1' => $feature1->feature_id,
      'node2' => $feature2->feature_id,
      'weight' => $weight,
      'direction' => $direction,
    );
    $netedge = chado_select_record('networkedge', array('networkedge_id'), $values);
    if (count($netedge) == 0){
      // if the edge doens't exist then add it
      $success = chado_insert_record('networkedge', $values);
      if (!$success) {
        throw new Exception("Cannot add network edge to database: %network %node1 %node2 %weight %direction",
            array('%network' => $network_name, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction));
      }
      $netedge = (object) $netedge;
    }
    else {
      $netedge = $netedge[0];
    }

    // If we have a module then add it.
    if ($module) {
      $this->addModule($network, $organism, $module,
          $mod_type, $feature1, $feature2);
    }
  }

  /**
   * Imports a module.
   *
   * Modules are specified on the edge element of the GraphML file so this
   * function is called when processing edges.
   *
   * @param $network
   *   An network record object.
   * @param $organism
   *   An organism record object.
   * @param $module.
   *   The name of the module.
   * @param $mod_type
   *   A cvterm object record representing the controlled vocabulary term
   *   for the network module type.
   * @param $feature1
   *   A feature record object for the first node of the edge
   * @param $feature2
   *   A feature record object for the second node of the edge
   */
  private function addModule($network, $organism, $module,
      $mod_type, $feature1, $feature2) {

    // Check to see if the module exists. If not then add it.
    $values = array(
      'network_id' => $network->network_id,
      'name' => $module,
      'type_id' => $mod_type->cvterm_id,
    );
    $result = chado_select_record('networkmod', array('networkmod_id'), $values);
    if (count($result) == 0) {
      // It doesn't exist so insert it.
      $result = chado_insert_record('networkmod', $values);
      if (!$result or !array_key_exists('networkmod_id', $result)) {
        throw new Exception(t("Could not add module, %module, for network, %network",
            array('%network' => $network_name, '%module' => $module)));
      }
      $networkmod = (object) $result;
    }
    else {
      $networkmod = $result[0];
    }

    // Add the nodes to the networknod_node table.
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'node' => $feature1->feature_id,
    );
    $netmodnode = chado_select_record('networkmod_node', array('networkmod_node_id'), $values);
    if (count($netmodnode) == 0){
      // If the edge doens't exist then add it.
      $result = chado_insert_record('networkmod_node', $values);
      if (!array_key_exists('networkmod_node_id', $result)) {
        throw new Exception(t("Cannot add network node to database: %network %module %node1",
            array('%network' => $network_name, '%module' => $module, '%node1' => $node1)));
      }
    }
    // Add node to the networkmod_node table.
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'node' => $feature2->feature_id,
    );
    $netmodnode = chado_select_record('networkmod_node', array('networkmod_node_id'), $values);
    if (count($netmodnode) == 0){
      // If the edge doens't exist then add it.
      $result = chado_insert_record('networkmod_node', $values);
      if (!array_key_exists('networkmod_node_id', $result)) {
        throw new Exception(t("Cannot add network node to database: %network %module %node2",
            array('%network' => $network_name, '%module' => $module, '%node2' => $node1)));
      }
    }

    // Now add the edge to the module, but first check to see if exists.
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'networkedge_id' => $netedge->networkedge_id,
    );
    $netmodedge = chado_select_record('networkmod_edge', array('networkmod_edge_id'), $values);
    if (count($netmodedge) == 0){
      // If the edge doens't exist then add it.
      $success = chado_insert_record('networkmod_edge', $values);
      if (!$success) {
        throw new Exception(t("Cannot add network edge to database: %network %module %node1 %node2 %weight %direction",
            array('%network' => $network_name, '%module' => $module, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction)));
      }
    }
  }
}
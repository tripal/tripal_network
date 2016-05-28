<?php
/**
 * @file
 * @todo Add file header description
 */

/**
 * @defgroup graphML_loader graphML Feature Loader
 * @{
 * Provides graphML loading functionality. Creates features based on their specification in a graphML file.
 * @}
 * @ingroup tripal_network
 */

/**
 * Creates the frm for the GraphML loader page.
 *
 * @return
 *   A Drupal Form API compatible array.
 */
function tripal_network_graphML_load_form($form, $form_state) {

  $form['graphML_file']= array(
    '#type'          => 'textfield',
    '#title'         => t('graphML File'),
    '#description'   => t('Please enter the full system path for the gramML
      (GML) file, or a path within the Drupal installation (e.g.
      /sites/default/files/xyz.gml).  The path must be accessible to the
      server on which this Drupal instance is running.'),
    '#required' => TRUE,
  );

  $organisms = tripal_get_organism_select_options(FALSE);
  $form['organism_id'] = array(
    '#title'       => t('Organism'),
    '#type'        => t('select'),
    '#description' => t("Choose the organism to which these sequences are associated"),
    '#required'    => TRUE,
    '#options'     => $organisms,
  );

  $form['analysis'] = array(
    '#type' => 'fieldset',
    '#title' => t('Analysis Used to Derive the Network'),
    '#collapsed' => TRUE
  );
  $form['analysis']['desc'] = array(
    '#type' => 'markup',
    '#value' => t("Why specify an analysis for a data load?  All data comes
       from some place, even if downloaded from somewhere. By specifying
       analysis details for all data uploads, it allows an end user to reproduce the
       data set, but at least indicates the source of the data."),
  );

  // TODO: we need more fields to help the user specify the unique constraint
  // for matching features.

  $analyses = tripal_get_analysis_select_options(FALSE);
  $form['analysis']['analysis_id'] = array(
   '#title'       => t('Analysis'),
   '#type'        => t('select'),
   '#description' => t("Choose the analysis to which this graph is associated"),
   '#required'    => TRUE,
   '#options'     => $analyses,
  );

  $form['button'] = array(
    '#type' => 'submit',
    '#value' => t('Import graphML file'),
  );

  return $form;
}

/**
 * The validate function for the tripal_network_graphML_load_form form.
 *
 * @param $form
 *   The Drupal form array.
 * @param $form_state
 *   The Drupal form_state array.
 */
function tripal_network_graphML_load_form_validate($form, &$form_state) {

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
 * The submit function for the tripal_network_graphML_load_form form.
 *
 * Adds a job to the Tripal Job system for importing the GraphML file.
 *
 * @param $form
 *   The Drupal form array.
 * @param $form_state
 *   The Drupal form_state array.
 */
function tripal_network_graphML_load_form_submit($form, &$form_state) {
  global $user;

  $graphML_file = $form_state['values']['graphML_file'];
  $organism_id = $form_state['values']['organism_id'];
  $analysis_id = $form_state['values']['analysis_id'];

  $args = array($graphML_file, $organism_id, $analysis_id);

  $fname = preg_replace("/.*\/(.*)/", "$1", $graphML_file);

  $includes = array(
    module_load_include('inc', 'tripal_network', 'includes/tripal_networ.grapphML_loade'),
  );
  tripal_add_job("Import graphML file: $fname", 'tripal_network',
    'tripal_network_load_graphML', $args, $user->uid, 10, $includes);

  return '';
}

/**
 * Imports the GraphML file containing a biological network.
 *
 * The nodes in the GraphML file should be the names of features in the
 * Chado feature table.
 *
 * @param $graphML_file
 *   The full path to the GraphML file on the file system.
 * @param $organism_id
 *   The ID of the organism to which this network belongs.
 * @param $analysis_id
 *   The ID of the analysis that was used to generate the network.
 * @param $job
 *   Optional. The ID of the job. This argument is provided by the Tripal
 *   Job system.
 *
 * @return
 *   TRUE on sucess, FALSE on failure.
 */
function tripal_network_load_graphML($graphML_file, $organism_id, $analysis_id, $job = NULL) {

  // TODO: add network properties to the networks and the modules
  // average degree <k>:  average_degree
  // number of nodes:     node_count
  // number of edges:     edge_count
  // average clustering coefficient <C>
  // add module expression profile graphs

  // Begin the transaction.
  $transaction = db_transaction();
  try {

    // Get the organism.
    $values = array('organism_id' => $organism_id);
    $organism = chado_select_record('organism', array('organism_id'), $values);
    if (count($organism) == 0) {
      watchdog('tripal_network', "Cannot find organism with id: %organism_id",
        array('%organism_id' => $organism_id), WATCHDOG_ERROR);
      return FALSE;
    }
    $organism = $organism[0];

    // Get the generic module type.
  //   $values = array(
  //     'name' => 'module',
  //     'cv_id' => array(
  //       'name' => 'network_modtype',
  //     ),
  //   );
  //   $options = array('statement_name' => 'sel_cvterm_nacv');
  //   $result = tripal_core_chado_select('cvterm', array('cvterm_id'), $values, $options);
  //   if (count($result) == 0) {
  //     watchdog('T_graphML_loader', "Cannot find cvterm for generic module.", array(), WATCHDOG_ERROR);
  //     return FALSE;
  //   }
  //   $mod_type = $result[0];

    // Load the XML file
    print "Opening $graphML_file...\n";
    $graphml = new XMLReader();

    // first iterate through all of the nodes and count them
    $graphml->open($graphML_file);
    $total_nodes = 0;
    while ($graphml->read()){
      $total_nodes++;
    }
    $graphml->close();

    // Set the initial progress of this job.
    tripal_network_graphML_loader_set_progress(0, $total_nodes, $job);

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
          tripal_network_load_graphML_graph($graphml, $attributes, $organism, $nodes_read, $mod_type);
        }
      }
    }
    $graphml->close();

    // Set the final progress for loading the graph.
    tripal_network_graphML_loader_set_progress($nodes_read);
  }
  catch (Exception $e) {
    print "\n"; // make sure we start errors on new line
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
    watchdog_exception('tripal_network', $e);
    return FALSE;
  }
  print "Done\n";
  return TRUE;
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
function tripal_network_load_graphML_graph($graphml, $attributes,
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
      watchog('tripal_network', "Cannot add network to database: %network",
        array('%network' => $network), WATCHDOG_ERROR);
      return FALSE;
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
      tripal_network_load_graphML_add_node($graphml, $attributes, $network, $organism, $nodes_read);
    }
    elseif (strcmp($graphml->name, 'edge') == 0) {
      tripal_network_load_graphML_add_edge($graphml, $attributes, $network, $organism, $nodes_read, $mod_type);
    }

    // Update our progress.
    tripal_network_graphML_loader_set_progress($nodes_read);
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
function tripal_network_load_graphML_add_node($graphml, $attributes,
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
      watchdog('tripal_network', "Cannot find the node in the feature table or as a synonym: %node",
          array('%node' => $id), WATCHDOG_ERROR);
      return FALSE;
    }
  }
  elseif(count($feature) > 1) {
    watchdog('tripal_network', "Too many features for this organism have the name of: %node. " .
        "Cannot determine which to use ",   array('%node' => $id), WATCHDOG_ERROR);
    return FALSE;
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
      watchdog('tripal_network', "Cannot add node, %node, to network, %network",
          array('%node' => $feature->uniquename, '%network' => $network->name), WATCHDOG_ERROR);
      return FALSE;
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
function tripal_network_load_graphML_add_edge($graphml, $attributes,
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
      watchdog('tripal_network', "Cannot add network edge to database: %network %node1 %node2 %weight %direction",
        array('%network' => $network_name, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction),
        WATCHDOG_ERROR);
      return FALSE;
    }
    $netedge = (object) $netedge;
  }
  else {
    $netedge = $netedge[0];
  }

  // If we have a module then add it.
  if ($module) {
    tripal_network_load_graphML_add_module($network, $organism, $module,
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
function tripal_network_load_graphML_add_module($network, $organism, $module,
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
      watchdog('tripal_network', "Could not add module, %module, for network, %network",
          array('%network' => $network_name, '%module' => $module), WATCHDOG_ERROR);
      return FALSE;
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
      watchdog('tripal_network', "Cannot add network node to database: %network %module %node1",
          array('%network' => $network_name, '%module' => $module, '%node1' => $node1),
          WATCHDOG_ERROR);
      return FALSE;
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
      watchdog('tripal_network', "Cannot add network node to database: %network %module %node2",
          array('%network' => $network_name, '%module' => $module, '%node2' => $node1),
          WATCHDOG_ERROR);
      return FALSE;
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
      watchdog('tripal_network', "Cannot add network edge to database: %network %module %node1 %node2 %weight %direction",
          array('%network' => $network_name, '%module' => $module, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction),
          WATCHDOG_ERROR);
      return FALSE;
    }
  }
}

/**
 * Sets the job progress for the Tripal Jobs system.
 *
 * This function has no effect if the $job argument is -1.
 *
 * @param $nodes_read
 *   The total number of nodes read.
 * @param $total
 *   The absolute total number of nodes in the file.
 * @param $job
 *   The ID of the job to update.  Defaults to -1
 */
function tripal_network_graphML_loader_set_progress($nodes_read, $total = -1, $job = -1) {

  static $total_nodes;
  static $interval;
  static $job_id;
  static $intv_read;
  static $last_read;
  static $start_time;
  static $time_to_load;

  if ($total != -1) {
    $total_nodes = $total;
    $interval = intval($total_nodes * 0.0001);
    if ($interval == 0) {
      $interval = 1;
    }
    $job_id = $job;
    $intv_read = 0;
    $last_read = 0;
    $time_to_load = 0;
    $start_time = time();
  }
  $intv_read += $nodes_read - $last_read;
  if ($intv_read >= $interval) {
    $seconds_taken = (time() - $start_time);
    $min_to_finish = ((($total_nodes / $nodes_read) * $seconds_taken) - $seconds_taken) / 60;
    $intv_read = 0;
    $percent = sprintf("%.2f", ($nodes_read / $total_nodes) * 100);
    print "Parsing element " . number_format($nodes_read) . " of " .
      number_format($total_nodes) . " (" . $percent . "%). Memory: " .
      number_format(memory_get_usage()) . " bytes.\r "; #Time Remaining: " .
#      sprintf("%d Min", $min_to_finish) . "\r";
#    $value = intval(($nodes_read / $total_nodes) * 100);
    tripal_job_set_progress($job_id, $value);
    //print number_format(memory_get_usage()) . "\r";
  }
  $last_read = $nodes_read;

}



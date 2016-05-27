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
 *
 *
 * @ingroup graphML_loader
 */
function tripal_network_graphML_load_form() {

  $form['graphML_file']= array(
    '#type'          => 'textfield',
    '#title'         => t('graphML File'),
    '#description'   => t('Please enter the full system path for the gramML (GML) file, or a path within the Drupal
                           installation (e.g. /sites/default/files/xyz.gml).  The path must be accessible to the
                           server on which this Drupal instance is running.'),
    '#required' => TRUE,
  );

  // get the list of organisms
  $sql = "SELECT * FROM {organism} ORDER BY genus, species";
  $previous_db = tripal_db_set_active('chado');  // use chado database
  $org_rset = db_query($sql);
  tripal_db_set_active($previous_db);  // now use drupal database
  $organisms = array();
  $organisms[''] = '';
  while ($organism = db_fetch_object($org_rset)) {
    $organisms[$organism->organism_id] = "$organism->genus $organism->species ($organism->common_name)";
  }
  $form['organism_id'] = array(
    '#title'       => t('Organism'),
    '#type'        => t('select'),
    '#description' => t("Choose the organism to which these sequences are associated"),
    '#required'    => TRUE,
    '#options'     => $organisms,
  );

  $form['import_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import Options'),
    '#collapsed' => TRUE
  );
  $form['import_options']['use_transaction']= array(
    '#type' => 'checkbox',
    '#title' => t('Use a transaction'),
    '#required' => FALSE,
    '#description' => t('Use a database transaction when loading the GFF file.  If an error occurs
      the entire datset loaded prior to the failure will be rolled back and will not be available
      in the database.  If this option is unchecked and failure occurs all records up to the point
      of failure will be present in the database.'),
  );
  /*
  $form['import_options']['add_only']= array(
    '#type' => 'checkbox',
    '#title' => t('Import only new nodes or edges'),
    '#required' => FALSE,
    '#description' => t('The job will skip features in the GFF file that already
                         exist in the database and import only new features.'),
    '#weight' => 2
  );
  $form['import_options']['update']= array(
    '#type' => 'checkbox',
    '#title' => t('Import all and update'),
    '#required' => FALSE,
    '#default_value' => 'checked',
    '#description' => t('Existing features will be updated and new features will be added.  Attributes
                         for a feature that are not present in the GFF but which are present in the
                         database will not be altered.'),
    '#weight' => 3
  );
  $form['import_options']['refresh']= array(
    '#type' => 'checkbox',
    '#title' => t('Import all and replace'),
    '#required' => FALSE,
    '#description' => t('Existing features will be updated and feature properties not
                         present in the GFF file will be removed.'),
    '#weight' => 4
  );
  $form['import_options']['remove']= array(
    '#type' => 'checkbox',
    '#title' => t('Delete nodes or edges'),
    '#required' => FALSE,
    '#description' => t('Features present in the GFF file that exist in the database
                         will be removed rather than imported'),
    '#weight' => 5
  );
  */

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

  // get the list of analyses
  $sql = "SELECT * FROM {analysis} ORDER BY name";
  $previous_db = tripal_db_set_active('chado');  // use chado database
  $org_rset = db_query($sql);
  tripal_db_set_active($previous_db);  // now use drupal database
  $analyses = array();
  $analyses[''] = '';
  while ($analysis = db_fetch_object($org_rset)) {
    $analyses[$analysis->analysis_id] = "$analysis->name ($analysis->program $analysis->programversion, $analysis->sourcename)";
  }
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
 *
 *
 * @ingroup graphML_loader
 */
function tripal_network_graphML_load_form_validate($form, &$form_state) {

  $graphML_file = $form_state['values']['graphML_file'];
  $organism_id = $form_state['values']['organism_id'];
  $add_only = $form_state['values']['add_only'];
  $update   = $form_state['values']['update'];
  $refresh  = $form_state['values']['refresh'];
  $remove   = $form_state['values']['remove'];
  $use_transaction   = $form_state['values']['use_transaction'];

  $update = 'Import all and update';  // until we support the other options, we'll set update == 1


  // check to see if the file is located local to Drupal
  $gff_file = trim($graphML_file);
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

  // @coder-ignore: there are no functions being called here
  // @todo: break each line of this conditional into separate variables to make more readable
  if (($add_only AND ($update OR $refresh OR $remove)) OR
      ($update AND ($add_only OR $refresh OR $remove)) OR
      ($refresh AND ($update OR $add_only OR $remove)) OR
      ($remove AND ($update OR $refresh OR $add_only))) {
      form_set_error('add_only', t("Please select only one checkbox from the import options section"));
  }
}

/**
 *
 * @ingroup graphML_loader
 */
function tripal_network_graphML_load_form_submit($form, &$form_state) {
  global $user;

  $graphML_file = $form_state['values']['graphML_file'];
  $organism_id = $form_state['values']['organism_id'];
  $add_only = $form_state['values']['add_only'];
  $update   = $form_state['values']['update'];
  $refresh  = $form_state['values']['refresh'];
  $remove   = $form_state['values']['remove'];
  $analysis_id = $form_state['values']['analysis_id'];
  $use_transaction   = $form_state['values']['use_transaction'];

  $update = 'Import all and update';  // until we support the other options, we'll set update == 1


  $args = array($graphML_file, $organism_id, $analysis_id, $add_only, $update,
    $refresh, $remove, $use_transaction);
  $type = '';
  if ($add_only) {
    $type = 'import only new features';
  }
  if ($update) {
    $type = 'import all and update';
  }
  if ($refresh) {
    $type = 'import all and replace';
  }
  if ($remove) {
    $type = 'delete features';
  }
  $fname = preg_replace("/.*\/(.*)/", "$1", $graphML_file);
  tripal_add_job("$type graphML file: $fname", 'tripal_network',
    'tripal_network_load_graphML', $args, $user->uid);

  return '';
}

/**
 * @file
 * The interpro XML parser
 */
function tripal_network_load_graphML($graphML_file, $organism_id, $analysis_id,
  $add_only = 0, $update = 0, $refresh = 0, $remove = 0, $use_transaction,
  $job = NULL) {

// TODO: add network properties to the networks and the modules
// average degree <k>:  average_degree
// number of nodes:     node_count
// number of edges:     edge_count
// average clustering coefficient <C>
// add module expression profile graphs

  // begin the transaction
  if ($use_transaction) {
    $connection = tripal_db_start_transaction();

    // if we cannot get a connection then let the user know the loading will be slow
    if (!$connection) {
       print "A persistant connection was not obtained. Loading will be slow\n";
    }
    else {
       print "\nNOTE: Loading of this graphML file is performed using a database transaction. \n" .
             "If the load fails or is terminated prematurely then the entire set of \n" .
             "insertions/updates is rolled back and will not be found in the database\n\n";
    }
  }
  else {
    $connection = tripal_db_persistent_chado();
    if (!$connection) {
       print "A persistant connection was not obtained. Loading will be slow\n";
    }
  }

  // clear out the anslysisfeature table for this analysis before getting started
  tripal_core_chado_delete('analysisfeature', array('analysis_id' => $analysis_id));

  // get the organism
  $values = array('organism_id' => $organism_id);
  $options = array('statement_name' => 'sel_organism_id');
  $organism = tripal_core_chado_select('organism', array('organism_id'), $values, $options);
  if (count($organism) == 0) {
    watchdog('T_graphML_loader', "Cannot find organism with id: %organism_id",
      array('%organism_id' => $organism_id), WATCHDOG_ERROR);
    return FALSE;
  }
  $organism = $organism[0];

  // get the generic module type
  $values = array(
    'name' => 'module',
    'cv_id' => array(
      'name' => 'network_modtype',
    ),
  );
  $options = array('statement_name' => 'sel_cvterm_nacv');
  $result = tripal_core_chado_select('cvterm', array('cvterm_id'), $values, $options);
  if (count($result) == 0) {
    watchdog('T_graphML_loader', "Cannot find cvterm for generic module.", array(), WATCHDOG_ERROR);
    return FALSE;
  }
  $mod_type = $result[0];

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
        tripal_network_load_graphML_graph ($graphml, $attributes, $organism, $nodes_read, $mod_type);
      }
    }
  }
  $graphml->close();
  tripal_network_graphML_loader_set_progress($nodes_read);

  // commit the transaction
  if ($use_transaction) {
    tripal_db_commit_transaction();
  }
  print "Done\n";
}

/*
 *
 */
function tripal_network_load_graphML_graph ($graphml, $attributes,
  $organism, &$nodes_read, $mod_type) {

  // first get the graph attributes
  $id = $graphml->getAttribute('id'); // use the id as the default network name
  $edgedefault = $graphml->getAttribute('edgedefault');
  if (!$edgedefault) {
    $edgedefault = 'undirected';
  }

  // now iterate through the nodes and edges and add them to Chado
  while ($graphml->read()) {
    $nodes_read++;

    if (strcmp($graphml->name, 'node')==0) {
      tripal_network_load_graphML_handle_node ($graphml, $attributes, $id, $organism, $nodes_read);
    }
    elseif (strcmp($graphml->name, 'edge')==0) {
      tripal_network_load_graphML_add_edge ($graphml, $attributes, $id, $organism, $nodes_read, $mod_type);
    }
    tripal_network_graphML_loader_set_progress($nodes_read);
  }
}
/*
 *
 */
function tripal_network_load_graphML_handle_node ($graphml, $attributes,
  $default_network, $organism, &$nodes_read){
  // first get the graph attributes
  $id = $graphml->getAttribute('id');

  // now iterate through data tags
  while ($graphml->read() ){
    $nodes_read++;
    if (strcmp($graphml->name, 'data')==0) {
      if ($graphml->nodeType == XMLReader::ELEMENT) { // <data>
         // get node details
      }
    }
    // if we're at the last sub node of this tree then
    // we have all of our node properties and we can
    // break out and handle the node
    elseif ($graphml->nodeType == XMLReader::END_ELEMENT) { // </node>
      break;
    }
  }
}
/*
 *
 */
function tripal_network_load_graphML_add_edge ($graphml, $attributes,
 $default_network, $organism, &$nodes_read, $mod_type) {

  // first get the graph attributes
  $node1 = $graphml->getAttribute('source');
  $node2 = $graphml->getAttribute('target');

  // now iterate through data tags and get the attributes for this edge
  $weight = 0;
  $network_name = $default_network;
  $module = '';
  $direction = 0;
  while ($graphml->read()){
    $nodes_read++;
    if (strcmp($graphml->name, 'data')==0) {
      if ($graphml->nodeType == XMLReader::ELEMENT) {  // <data>
        // get the proprety type (key) and it's value
        $key = $graphml->getAttribute('key');
        $graphml->read();
        $nodes_read++;
        $value = $graphml->value;

        // first look for the attributes that we support
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
    // if we're at the last sub node of this tree then
    // we have all of our edge properties and we can
    // break out and add the edges
    elseif ($graphml->nodeType == XMLReader::END_ELEMENT) { // </edge>
      break;
    }
  }

  // before we get started, let's prepare one SQL statement that
  // cannot use the Tripal API
  $psql = "
   PREPARE sel_synonym_graphML(text) AS
   SELECT F.feature_id
   FROM synonym S
     INNER JOIN feature_synonym FS on FS.synonym_id = S.synonym_id
     INNER JOIN feature F on FS.feature_id = F.feature_id
   WHERE S.name = $1
  ";
  tripal_core_chado_prepare('sel_synonym_graphML', $psql, array('text'));


  // make sure the network exists
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
  $sel_options = array('statement_name' => 'sel_network_unorty');
  $network = tripal_core_chado_select('network', array('network_id'), $values, $sel_options);
  if(count($network) == 0){
    // the network doesn't exist so let's add it
    $ins_values = $values;
    $ins_values['name'] = $network_name;
    $ins_options = array('statement_name' => 'ins_network_unortyna');
    $result = tripal_core_chado_insert('network', $ins_values, $ins_options);
    if (!array_key_exists('network_id', $result)) {
      watchog('T_graphML_loader', "Cannot add network to database: %network",
        array('%network' => $network), WATCHDOG_ERROR);
      return FALSE;
    }
    $network = tripal_core_chado_select('network', array('network_id'), $values, $sel_options);
  }
  $network = $network[0];


  // make sure node1 exists in the feature table
  $values = array(
    'name' => $node1,
    'organism_id' => $organism->organism_id,
  );
  $sel_options = array('statement_name' => 'sel_feature_unor');
  $feature1 = tripal_core_chado_select('feature', array('feature_id'), $values, $sel_options);

  if (count($feature1) == 0){
    // we couldn't find the feature by uniquename so look for this feature as a synonym
    $feature1 = db_fetch_object(chado_query("EXECUTE sel_synonym_graphML('%s')", $node1));

    if (!$feature1) {
      watchdog('T_graphML_loader', "Cannot find the target node in the feature table or as a synonym: %node1",
        array('%node1' => $node1), WATCHDOG_ERROR);
      return FALSE;
    }
  }
  elseif(count($feature1) > 1) {
  	 watchdog('T_graphML_loader', "Too many features for this organism have the synonym of: %node1. " .
  	    "Cannot determine which to use ",   array('%node1' => $node1), WATCHDOG_ERROR);
      return FALSE;
  }
  else {
    $feature1 = $feature1[0];
  }

  // make sure node2 exists in the feature table
  $values = array(
    'name' => $node2,
    'organism_id' => $organism->organism_id,
  );
  $sel_options = array('statement_name' => 'sel_feature_unor');
  $feature2 = tripal_core_chado_select('feature', array('feature_id'), $values, $sel_options);
  if (count($feature2) == 0){
    // we couldn't find the feature by uniquename so look for this feature as a synonym
    $feature2 = db_fetch_object(chado_query("EXECUTE sel_synonym_graphML('%s')", $node2));
    if (!$feature2){
      watchdog('T_graphML_loader', "Cannot find the target node in the feature table or as a synonym: %node2",
        array('%node2' => $node2), WATCHDOG_ERROR);
      return FALSE;
    }
  }
  elseif(count($feature2) > 1) {
     watchdog('T_graphML_loader', "Too many features for this organism have the synonym of: %node2. " .
        "Cannot determine which to use ",   array('%node2' => $node2), WATCHDOG_ERROR);
      return FALSE;
  }
  else {
    $feature2 = $feature2[0];
  }

  // check to see if node1 is present in the network node table, if not add it
  $values = array('node' =>  $feature1->feature_id);
  $options = array('statement_name' => 'sel_feature_id');
  $result = tripal_core_chado_select('networknode', array('node'), $values, $options);
  if (count($result) == 0) {
    $values = array(
      'network_id' => $network->network_id,
      'node' => $feature1->feature_id,
    );
    $options = array('statement_name' => 'ins_networknode_neno');
    $success = tripal_core_chado_insert('networknode', $values, $options);
    if (!$success) {
      watchdog('T_graphML_loader', "Cannot add node, %node, to network, %network",
        array('%node' => $feature1->uniquename, '%network' => $network->name), WATCHDOG_ERROR);
      return FALSE;
    }
  }

  // check to see if node2 is present in the network node table, if not add it
  $values = array('node' =>  $feature2->feature_id);
  $options = array('statement_name' => 'sel_feature_id');
  $result = tripal_core_chado_select('networknode', array('node'), $values, $options);
  if (count($result) == 0) {
    $values = array(
      'network_id' => $network->network_id,
      'node' => $feature2->feature_id,
    );
    $options = array('statement_name' => 'ins_networknode_neno');
    $success = tripal_core_chado_insert('networknode', $values, $options);
    if (!$success) {
      watchdog('T_graphML_loader', "Cannot add node, %node, to network, %network",
        array('%node' => $feature2->uniquename, '%network' => $network->name), WATCHDOG_ERROR);
      return FALSE;
    }
  }

  // now add in the edge if it doesn't already exists
  $values = array(
    'network_id' => $network->network_id,
    'node1' => $feature1->feature_id,
    'node2' => $feature2->feature_id,
    'weight' => $weight,
    'direction' => $direction,
  );
  $sel_options = array('statement_name' => 'sel_networkedge_uq1');
  $netedge = tripal_core_chado_select('networkedge', array('networkedge_id'), $values, $sel_options);
  if (count($netedge) == 0){
    // if the edge doens't exist then add it
    $ins_options = array('statement_name' => 'ins_networkedge_uq1');
    $success = tripal_core_chado_insert('networkedge', $values, $ins_options);
    if (!$success) {
      watchdog('T_graphML_loader', "Cannot add network edge to database: %network %node1 %node2 %weight %direction",
        array('%network' => $network_name, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction),
        WATCHDOG_ERROR);
      return FALSE;
    }
    $netedge = tripal_core_chado_select('networkedge', array('networkedge_id'), $values, $sel_options);
  }
  $netedge = $netedge[0];

  // if we have a module make sure it exists
  if ($module) {

    $values = array(
      'network_id' => $network->network_id,
      'name' => $module,
      'type_id' => $mod_type->cvterm_id,
    );
    $sel_options = array('statement_name' => 'sel_networkmod_nena');
    $result = tripal_core_chado_select('networkmod', array('networkmod_id'), $values, $sel_options);
    if (count($result) == 0) {
      // it doesn't exist so insert it
      $ins_options = array('statement_name' => 'ins_networkmod_nena');
      $result = tripal_core_chado_insert('networkmod', $values, $ins_options);
      if (!$result or !array_key_exists('networkmod_id', $result)) {
        watchdog('T_graphML_loader', "Could not add module, %module, for network, %network",
          array('%network' => $network_name, '%module' => $module), WATCHDOG_ERROR);
        return FALSE;
      }
    }
    $result = tripal_core_chado_select('networkmod', array('networkmod_id'), $values, $sel_options);
    $networkmod = $result[0];

    // add the nodes to the networknod_node table
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'node' => $feature1->feature_id,
    );
    $sel_options = array('statement_name' => 'sel_networkmodnode_neno');
    $netmodnode = tripal_core_chado_select('networkmod_node', array('networkmod_node_id'), $values, $sel_options);
    if (count($netmodnode) == 0){
      // if the edge doens't exist then add it
      $ins_options = array('statement_name' => 'ins_networkmodnode_neno');
      $result = tripal_core_chado_insert('networkmod_node', $values, $ins_options);
      if (!array_key_exists('networkmod_node_id', $result)) {
        watchdog('T_graphML_loader', "Cannot add network node to database: %network %module %node1",
          array('%network' => $network_name, '%module' => $module, '%node1' => $node1),
          WATCHDOG_ERROR);
        return FALSE;
      }
    }
    // node add node2
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'node' => $feature2->feature_id,
    );
    $sel_options = array('statement_name' => 'sel_networkmodnode_neno');
    $netmodnode = tripal_core_chado_select('networkmod_node', array('networkmod_node_id'), $values, $sel_options);
    if (count($netmodnode) == 0){
      // if the edge doens't exist then add it
      $ins_options = array('statement_name' => 'ins_networkmodnode_neno');
      $result = tripal_core_chado_insert('networkmod_node', $values, $ins_options);
      if (!array_key_exists('networkmod_node_id', $result)) {
        watchdog('T_graphML_loader', "Cannot add network node to database: %network %module %node2",
          array('%network' => $network_name, '%module' => $module, '%node2' => $node1),
          WATCHDOG_ERROR);
        return FALSE;
      }
    }


    // now add the edge to the module, but first check to see
    // if it already exists
    $values = array(
      'networkmod_id' => $networkmod->networkmod_id,
      'networkedge_id' => $netedge->networkedge_id,
    );
    $sel_options = array('statement_name' => 'sel_networkmodedge_uq1');
    $netmodedge = tripal_core_chado_select('networkmod_edge', array('networkmod_edge_id'), $values, $sel_options);
    if (count($netmodedge) == 0){
      // if the edge doens't exist then add it
      $ins_options = array('statement_name' => 'ins_networkmodedge_uq1');
      $success = tripal_core_chado_insert('networkmod_edge', $values, $ins_options);
      if (!$success) {
        watchdog('T_graphML_loader', "Cannot add network edge to database: %network %module %node1 %node2 %weight %direction",
          array('%network' => $network_name, '%module' => $module, '%node1' => $node1, '%node2' => $node2, '%weight' => $weight, '%direction' => $direction),
          WATCHDOG_ERROR);
        return FALSE;
      }
    }
  }
}
/*
 *
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



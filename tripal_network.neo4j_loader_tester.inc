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
  $analysis_id = $form_state['values']['analysis_id'];

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
    module_load_include('inc', 'tripal_network', 'tripal_network.neo4j_loader_tester.inc'),
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
    $organism = chado_select_record('organism', array('organism_id','genus','species'), $values);
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
          tripal_network_load_graphML_graph($graphml, $attributes, $organism, $nodes_read, $mod_type,$analysis_id);
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
  $organism, &$nodes_read, $mod_type,$analysis_id) {

  // First get the graph attributes and ase the id as the default network name.
  $id = $graphml->getAttribute('id');
  $edgedefault = $graphml->getAttribute('edgedefault');
  if (!$edgedefault) {
    $edgedefault = 'undirected';
  }

  // Make sure the network exists, if not then add it.
  $network_name = $id;
  //$organism->organism_id is the name of the organism for which the entry has to be made
 
  // Now iterate through the nodes and edges and add them to the database.
  while ($graphml->read()) {
    $nodes_read++;

    if (strcmp($graphml->name, 'node') == 0) {
      tripal_network_load_graphML_add_node($graphml, $attributes, $network, $organism, $nodes_read,$analysis_id);
    }
    elseif (strcmp($graphml->name, 'edge') == 0) {
      tripal_network_load_graphML_add_edge($graphml, $attributes, $network, $organism, $nodes_read, $mod_type,$network_name);
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
  $default_network, $organism, &$nodes_read,$analysis_id) {

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
     FROM {synonym} S
       INNER JOIN {feature_synonym} FS on FS.synonym_id = S.synonym_id
       INNER JOIN {feature} F on FS.feature_id = F.feature_id
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
    $nodeid = $id;
    $nodeid=(string)$nodeid;
    $species = $organism->species;
    $species = (string)$species;
    $genus = $organism->genus;
    $genus = (string)$genus;
    $organismid = $organism->organism_id;
    $feature = $feature[0];
    $feature_id = $feature->feature_id;
    

    $label = "tripal_network";

    //print_r($species);
    
    // This statement is querying if the node with the given id exists;
    $var="MATCH(u:".$label.") WHERE u.id = {id} RETURN u.id";
     $data=array(
     "query" => $var,
      "params" => array(
        "id"=>$nodeid,
      )
     );
    // This piece of code, will use the Neo4j REST API to pass the request
    $data=json_encode($data);  
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
    curl_setopt($curl, CURLOPT_POSTFIELDS,$data);

    //The result from the request gets stored in the $result1 variable

    $result1 = curl_exec($curl);

    //echo $result1;
    curl_close($curl); 
    //Converting the result into a json array
    $result=json_encode($result1);

    // The result given by the REST API request gives an empty field with "data". 
    $word="data";

    if(strpos($result,$word)!==false)
    {
      
      $pos=strpos($result,$word);
      
    }

    //Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present
    $pos=$pos+11;

    //
    if($result[$pos]=="[")
    {
      echo "";
    }

    //Here in the else statement the query for the insertion of the nodes will be there.
    else{

      // Here query chado database with the nodeid to extract the Feature.type_id
      //
      $create_query="CREATE (x:".$label." { id : {id}, genus:{genus},species:{species}, organism_id:{organism_id},analysis_id:{analysis_id},feature_id:{feature_id}}) RETURN x";
      $data=array(
       "query" => $create_query,
       "params" => array(
       "id" => $nodeid,
       "genus"=>$genus,
       "species"=>$species,
       "organism_id"=>$organismid,
       "analysis_id"=>$analysis_id,
       "feature_id"=>$feature_id,
      )
     ); 


    $data=json_encode($data);  
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
    curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
    $result1 = curl_exec($curl);
    //echo $result1;
    curl_close($curl);

    }

    //end of neo4j connection for database entry

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
  $network, $organism, &$nodes_read, $mod_type,$network_name) {

  // First get the graph attributes.

  $source= $graphml->getAttribute('source');
  $target = $graphml->getAttribute('target');
  $label = "tripal_network";
  $species = $organism->species;
  $module_rel = $species."module";

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

  $que = "MATCH(n1:".$label."{id:{id1}})-[rel:".$species."module]->(n2:".$label."{id:{id2}}) WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id";

    //This query is checking if a relationship corresponding to the source and the destination with the specific label of the relationship already exists

     $data=array(
     "query" => $que,
      "params" => array(
        "id1"=>$source,
        "id2"=>$target,
        "value"=>$module
      )
     ); 


      $data=json_encode($data);  
      $curl = curl_init();
      curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
      curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
      curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
      $result1 = curl_exec($curl);

      //echo $result1;

      curl_close($curl);


      $result=json_encode($result1);
      $word="data";
      //Search for the word data in the string returned by the REST API call

      if(strpos($result,$word)!==false){
        
        $pos=strpos($result,$word);
        
      }

      //Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present

      //Here there has to be an else statement which will handle any sort of errors
      $pos=$pos+11;


      // if "[" exists then that means there has been something that has been returned and a relationship of the desired type already exists

      if($result[$pos]=="["){
        echo "";
      }

      //
      else{
        
        //The following query will attach a relationship to two nodes with the source and target as extracted from the graphml file

          $insert_query="MATCH (u:".$label."{id:{source}}),(r:".$label."{id:{target}}) CREATE (u)-[:".$module_rel."{modulename:{mod},network:{net}}]->(r)";

          $data=array(
          "query" => $insert_query,
          "params" => array(
          "source" => $source,
          "target" =>$target,
          "mod"=>$module,
          "net"=>$network_name
        )
       ); 

          //encoding the data in json format
          $data=json_encode($data);  

          //Sending data through the Neo4j REST API to the server

          $curl = curl_init();
          curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
          curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
          curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
          curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
          curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
          $result1 = curl_exec($curl);
          //echo $result1;
          curl_close($curl);
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
    //tripal_job_set_progress($job_id, $value);
    //print number_format(memory_get_usage()) . "\r";
  }
  $last_read = $nodes_read;

}



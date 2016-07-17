<?php

/**
 *
 */
function tripal_network_viewer_page() {
  return theme('tripal_network_viewer', array());
}

/**
 *
 * @param $form
 * @param $form_state
 * @return multitype:
 */
function tripal_network_viewer_filter_form($form, &$form_state) {
  $form = array();

  $species = isset($_SESSION['tripal_network_filter_species']) ? $_SESSION['tripal_network_filter_species'] : '';
  $module = isset($_SESSION['tripal_network_filter_module']) ? $_SESSION['tripal_network_filter_module'] : '';
  if (array_key_exists('values', $form_state)) {
    $species = $form_state['values']['species'];
    $module = $form_state['values']['module'];
  }

  // If we have a species and a module then generate the miss.json file.
  if ($species and $module) {
    tripal_network_viewer_get_network($species, $module);
  }

  // Get the modules for the selected speices from Neo4j.
  $module_options = array();
  $module_options['0'] = '-- Select --';
  if ($species) {
    $que = "MATCH p=()-[r:".$species."module]->() RETURN distinct(r.modulename) ORDER BY r.modulename ASC";
    $data=array(
      "query" => $que
    );
    $data = json_encode($data);
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
    $result = curl_exec($curl);
    $result = json_decode($result, TRUE);

    // Format the modules returned from Neo4j into an array for the modules
    // form element.
    foreach ($result['data'] as $mod) {
      $module_options[$mod[0]] = $mod[0];
    }
  }

  //$options = tripal_get_organism_select_options();
  $species_options = array();
  $species_options['0'] = '-- Select --';
  $species_options['rice'] = 'Rice';
  $species_options['maize'] = 'Maize';
  $species_options['arabidopsis'] = 'Arabidopsis';
  $form['species'] = array(
    '#type' => 'select',
    '#title' => t('Species'),
    '#options' => $species_options,
    '#default_value' => $species,
    '#description' => t(''),
    '#ajax' => array(
      'callback' => "tripal_network_viewer_filter_form_ajax_callback",
      'wrapper'  => 'tripal-network-viewer-filter-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  );
  $form['module'] = array(
    '#type' => 'select',
    '#title' => t('Module'),
    '#options' => $module_options,
    '#default_value' => $module,
    '#description' => t(''),
    '#ajax' => array(
      'callback' => "tripal_network_viewer_filter_form_ajax_callback",
      'wrapper'  => 'tripal-network-viewer-filter-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  );

  $form['#prefix'] = '<div id ="tripal-network-viewer-filter-form">';
  $form['#suffix'] = '</div>';
  $form['#attributes'] = array(
  );
  return $form;
}


/**
 * Ajax function which returns the form via ajax
 */
function tripal_network_viewer_filter_form_ajax_callback($form, &$form_state) {
  // Set session variables to remember the form selections
  $_SESSION['tripal_network_filter_species'] = $form_state['values']['species'];
  $_SESSION['tripal_network_filter_module'] = $form_state['values']['module'];

  //gensas_add_ajax_command(ajax_command_invoke(NULL, "load_job_status_panel", array()));
  return $form;
}

/**
 *
 * @param unknown $species
 * @param unknown $module
 */
function tripal_network_viewer_get_network($species, $module) {
  $status = 0;
  $rel = $species . "module";
  $file_path = drupal_get_path('module', 'tripal_network');

  if (!$species or !$module) {
    $return = array(
      'nodes' => array(),
      'edges' => array(),
    );
    $fh = fopen($file_path . "/miss.json",'w') or die("Error opening file");
    fwrite($fh, json_encode($return));
    fclose($fh);
  }

  //Matching query for neo4j
  $que = "MATCH(n1:".$species.")-[rel:".$rel."]->(n2:".$species.") WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id";
  $data = array(
    "query" => $que,
    "params" => array(
      "value"=>$module
    )
  );

  // Neo4j REST API calls
  $data = json_encode($data);
  $curl = curl_init();
  curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
  curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
  $result1 = curl_exec($curl);
  curl_close($curl);

  $result1 = json_decode($result1, TRUE);
  $nodes = array();
  $edges = array();

  $i = 0;
  $e = 0;

  // Storing the nodes in an array
  // Purpose of this is to use the index for mapping of ID's
  $edge_count = 0;
  foreach ($result1['data'] as $row) {
    //Following is for the storage of names which are id's only with proper indexing
    $nodes[$i] = $row[1];
    $i++;
    $nodes[$i] = $row[2];
    $i++;
    $edge_count = $edge_count+1;
  }
  // Creating an array with unique ID's as we don't want repetition of nodes in the given array
  $nodes = array_unique($nodes);
  sort($nodes);


  $num = count($nodes);
  $n = 0;
  $node = array();

  // Storage of the nodes in the proper format with the correct indexing
  $colors = array("#009999","#666","#003366","#660066","#00FF00");
  for ($x = 0; $x < $num; $x++) {
    $temp = array();
    $temp["id"]= (string)$x;
    $temp["label"]= (string)$nodes[$x];
    $temp["x"]=mt_rand();
    $temp["y"]=mt_rand();
    $temp["size"]=mt_rand(1,3)/1000;
    $val = mt_rand(0,4);
    $temp["color"]="gray";
    //$temp["color"]="gray";

    //$temp["name"] = $nodes[$x];
    //$temp["group"] = 2;
    $node[$n] = $temp;
    $n++;
  }

  // Storage of the nodes in proper format
  foreach($result1['data'] as $row) {
    //Following is for the storage of the relationships with source and targets mapped to proper indexes
    $temp = array();
    $temp["id"] = (string)$e;
    $key = array_search($row[1],$nodes);
    $temp["source"] = (string)$key;
    $source = $key;
    $key = array_search($row[2],$nodes);
    $temp["target"]=(string)$key;
    $target=$key;
    $temp["size"]=mt_rand();
    $temp["type"]="straight";
    $temp["color"]="#ccc";
    $temp["hover_color"]="#A0A0A0";
    $edges[$e]=$temp;
    $e++;
  }

  $return = array(
    'nodes' => $node,
    'edges' => $edges
  );
  $jsonarray = json_encode($return);

  // Write the JSON array to the 'miss.json' file
  $fh = fopen($file_path . "/miss.json",'w') or die("Error opening file");
  fwrite($fh, $jsonarray);
  fclose($fh);

}


/**
 *
 * @param $species
 */
function tripal_network_viewer_ajax_retrieve($species) {
  //$species = $_GET["q"];

  $que = "MATCH p=()-[r:".$species."module]->() RETURN distinct(r.modulename)";
  $data=array(
    "query" => $que
  );

  $data = json_encode($data);
  $curl = curl_init();
  curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($curl, CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
  curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
  $result1 = curl_exec($curl);
  curl_close($curl);

  //When TRUE returned objects will be converted into associative arrays
  $result1 = json_decode($result1,TRUE);
  error_log('retrieve.php: ' . print_r($result1, TRUE), 3, "/tmp/php_log");
  $string = "<select>";

  foreach ($result1['data'] as $row) {
    $string .= "<option>". $row[0]."</option>";
  }

  $string = $string + "</select>";

}
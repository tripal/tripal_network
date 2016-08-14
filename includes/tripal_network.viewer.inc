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
  // Now that our form is updated we want to reload the network.  The network
  // is handled by JavaScript so we need to force the JS code to call our
  // tripal_network_viewer_ajax_retrieve() function.  The following code
  // will instruct Drupal to call a Javascript function named 'retreiveNetwork'
  // which will then make an AJAX call to update the network.
  $data = array(
    'species' => $form_state['values']['species'],
    'module' => $form_state['values']['module'],
  );
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "retrieveNetwork", array($data)));
  return $form;
}
/**
 *
 * @param unknown $species
 * @param unknown $module
 */
function tripal_network_viewer_ajax_retrieve() {
  // Initialize our network data array.
  $network_data = array(
    'nodes' => array(),
    'edges' => array(),
  );
  // Get the fitering variables requested by the user.
  $species = $_GET['species'];
  $module = $_GET['module'];
  // Make sure we have required filters.
  if (!$species or !$module) {
    drupal_json_output($network_data);
  }
  // Matching query for neo4j
  $rel = $species . "module";
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
  curl_setopt($curl, CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
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
  $network_data = array(
    'nodes' => $node,
    'edges' => $edges
  );
  drupal_json_output($network_data);
}
/**
 * An AJAX callback for retrieving funcational informtion for a set of nodes.
 *
 * This function expects the following parameters in the $_GET array (as
 * provided in an HTML GET).
 *
 * - nodes: An associative array with the following keys:
 *    - nodes: An array containing the list of node names.
 *    - genus: The genus to which the node belongs
 *    - species:  The species to which the node belongs
 *    - type:  The feature 'type' of the node (e.g. 'gene', 'mRNA'). The
 *        type should be a valid name of a Sequence Ontology term.
 *
 * @return
 *   An associative array where the key is the node name and the value is an
 *   array of key / value paris where the key is the term accession from a
 *   controlled vocabulary, and the value is the term name.
 */
function tripal_network_viewer_ajax_function() {
  $nodes = $_POST['nodes'];
  $genus = $_POST['genus'];
  $species = $_POST['species'];
  $type = $_POST['type'];
  // Get the list of terms for this feature.
  // TODO: using the 'name' will have problems when trying to find unique
  // features. It's not part of the unqiue constraint, but rather the uniquename
  // field is.  If we could have feature_id's passed in rather than node
  // names that would ensure we get unique features.
  $sql = "
    SELECT
      F.name as node_name, DB.name as dbname, DBX.accession,
      CVT.name as term_name, CVT.definition
    FROM {feature} F
      INNER JOIN {feature_cvterm} FCVT ON F.feature_id = FCVT.feature_id
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = FCVT.cvterm_id
      INNER JOIN {dbxref} DBX ON DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB ON DBX.db_id = DB.db_id
      INNER JOIN {organism} O on O.organism_id = F.organism_id
      INNER JOIN {cvterm} FCVT on FCVT.cvterm_id = F.type_id
    WHERE
      F.name IN :name
      O.genus = :genus
      O.species = :species
      FCVT.name = :type
    ORDER BY DB.name, CVT.name
  ";
  $args = array(
    ':name' => $nodes,
    ':genus' => $genus,
    ':species' => $species,
    ':type' => $type,
  );
  $query = chado_query($sql, $args);
  $results = array();
  while ($record = $query->fetchObject()) {
    $accession = $record->accession;
    if (is_numeric($accession)) {
      $accession = $record->dbname . ":" . $accession;
    }
    $results[$record->node_name][] = array(
      $accession => $term->term_name
    );
  }
  return drupal_json($results);
}
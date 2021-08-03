<?php

/**
 *
 */
function tripal_network_viewer_page() {
  return theme('tripal_network_viewer', array());
}

/**
 *
 */
function tripal_network_viewer_form($form, &$form_state, $network_id = NULL, $feature_id = NULL) {

  // Get the list of networks
  $sql = '';
  $args = [':network_id' => $network_id];
  if ($feature_id) {
    $sql = "
      SELECT *
      FROM {network} N
        INNER JOIN {network_node} NN ON NN.network_id = N.network_id
      WHERE N.network_id = :network_id AND NN.feature_id = :feature_id
      ORDER BY N.name
    ";
    $args[':feature_id'] = $feature_id;
  }
  else {
    $sql = "
      SELECT *
      FROM {network} N
      WHERE N.network_id = :network_id
      ORDER BY N.name
    ";
  }
  $networks = [
    0 => '-- Select --',
  ];
  $results = chado_query($sql, $args);
  while ($record = $results->fetchObject()) {
    $networks[$record->network_id] = $record->name;
  }

  $form['feature_id'] = [
    '#type' => 'value',
    '#value' => $feature_id,
  ];
  $form['network_id'] = [
    '#type' => 'select',
    '#title' => 'Network',
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('select a network for viewing.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

  $form['data_panel'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-network-viewer-data-panel">Data Panel</div>',
    '#prefix' => '<div id="tripal-network-viewer">',
  ];
  $form['graph_viewer'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-network-viewer-graph-container"></div>',
    '#suffix' => '</div>',
  ];


  // Make sure this path is always included in the form. This normally
  // happens but for some reason when the form is added via the
  // sio__network_diagram field it doesn't happen.
  form_load_include($form_state, 'inc', 'tripal_network', 'includes/tripal_network.viewer');

  return $form;
}

/**
 *
 */
function tripal_network_viewer_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];
  $feature_id = $form_state['values']['feature_id'];

  $data = [
    'network_id' => $network_id,
  ];
  if ($feature_id) {
    $data['feature_id'] = $feature_id;
  }


  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "retrieveNetwork", array($data)));

  return $form;
}

/**
 * Retreives details about a node when selected in the viewer.
 */
function tripal_network_viewer_ajax_node_retrieve() {

  $feature_id = $_GET['feature_id'];
  $feature = chado_generate_var('feature', ['feature_id' => $feature_id]);

  $headers = [];
  $rows = [];
  $rows[] = [
    [
      'data' => 'Name',
      'header' => TRUE,
      'width' => '20%',
    ],
    $feature->name
  ];

  $rows[] = [
    [
      'data' => 'Unique Name',
      'header' => TRUE,
      'width' => '20%',
    ],
    $feature->uniquename
  ];

  $rows[] = [
    [
      'data' => 'Type',
      'header' => TRUE,
    ],
    $feature->type_id->name,
  ];

  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'class' => 'tripal-data-table'
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  $content = [
    '#type' => 'item',
    '#title' => 'Node Details',
    '#markup' => theme_table($table),
  ];

  $node_data = [
    'html' => drupal_render($content),
  ];

  drupal_json_output($node_data);
}

/**
 *
 * @param $species
 * @param $module
 */
function tripal_network_viewer_ajax_retrieve() {

  // Initialize our network data array.
  $network_data = array(
    'nodes' => array(),
    'edges' => array(),
  );

  // Get the fitering variables requested by the user.
  $network_id = $_GET['network_id'];
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : '';

  // Make sure we have required filters.
  if (!$network_id) {
    drupal_json_output($network_data);
    return;
  }

  // Get the list of edges and rows that match the query.
  $nodes = [];
  $edges = [];
  $params = [':network_id' => $network_id];

  // Get the edges from the network
  $select = "
    SELECT
      FS.feature_id as node1_id, FS.name as node1,
      FT.feature_id as node2_id, FT.name as node2,
      NE.network_edge_id
   ";
  $from = "
    FROM {network_edge} NE
      INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
      INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
      INNER JOIN {feature} FS on FS.feature_id = NNS.feature_id
      INNER JOIN {feature} FT on FT.feature_id = NNT.feature_id
  ";
  $where = "
    WHERE NE.network_id = :network_id
  ";
  $order = "";
  $limit = "
    LIMIT 1000 OFFSET 0
  ";


  if ($feature_id) {
    $where .= " AND (FS.feature_id = :feature_id OR FS.feature_id = :feature_id) ";
    $params[':feature_id'] = $feature_id;
  }

  $sql = $select . $from . $where . $order . $limit;

  // Execute the query and return the parsed results.
  $results = chado_query($sql, $params);
  while ($record = $results->fetchObject()) {
    $nodes[$record->node1_id] = array(
      'id' => $record->node1_id,
      'label' => $record->node1,
      'x' => mt_rand(),
      'y' => mt_rand(),
      'size' => mt_rand(1, 3) / 1000,
      'color' => "gray",
    );
    $nodes[$record->node2_id] = array(
      'id' => $record->node2_id,
      'label' => $record->node2,
      'x' => mt_rand(),
      'y' => mt_rand(),
      'size' => mt_rand(1, 3) / 1000,
      'color' => "gray",
    );
    $edges[$record->network_edge_id] = array(
      'id' => $record->network_edge_id,
      'source' => $record->node1_id,
      'target' => $record->node2_id,
      'size' => mt_rand(),
      'type' => 'straight',
      'color' => '#CCCCC',
      'hover_color' => '#A0A0A0',
    );
  }
  $network_data = array(
    'nodes' => array_values($nodes),
    'edges' => array_values($edges),
  );

  drupal_json_output($network_data);
}

/**
 *
 * @param $species
 * @param $module
 */
function tripal_network_viewer_neo4j_ajax_retrieve() {

  // Initialize our network data array.
  $network_data = array(
    'nodes' => array(),
    'edges' => array(),
  );

  // Get the fitering variables requested by the user.
  $species = $_GET['species'];
  $genes = $_GET['genes'];
  $prop_filters = $_GET['properties'];
  $filter_cond = $_GET['filter_cond'];

  // Make sure we have required filters.
  if (!$species) {
    drupal_json_output($network_data);
    return;
  }

  if (count($genes) == 0 and count(array_keys($prop_filters)) == 0){
    drupal_json_output($network_data);
    return;
  }

  // Get the list of edges and rows that match the query.
  $nodes = array();
  $edges = array();
  $params = array();

  // Make sure we only have OR or AND filter conditional.
  if ($filter_cond != 'OR' and $filter_cond != 'AND') {
    $filter_cond = 'AND';
  }

  // Build the initial query to search for nodes by taxonomy ID.
  tripal_neo4j_connect("bolt", "bolt://neo4j:testing123@localhost:7687");
  $where = array();
  $query = 'MATCH(n1:Transcript)-[r:CO]-(n2:Transcript)
    WHERE (n1.data__1179 = $taxId OR n2.data__1179 = $taxId) ';
  $params['taxId'] =(int) $species;

  // If the user is filtering by gene name then add in that statement
  if (count($genes) > 0) {
    $where[] = '(n1.schema__name IN $genes OR n2.schema__name IN $genes))';
    $params['genes'] = $genes;
  }

  // If the user is filtering by properties then add in those statements.
  if (count($prop_filters) > 0) {
    foreach ($prop_filters as $id => $filter) {
      if ($filter['value'] and $filter['value'] !== 0) {
        $where[] =  '(r.' . $filter['type'] . ' ' . $filter['op'] . ' $prop' . $id .  ')';
        $params['prop' . $id] = (float) $filter['value'];
      }
    }
  }

  if (count($where > 0)) {
    $query .= 'AND (';
    $query .= $where[0];
    for ($i = 1; $i < count($where); $i++) {
      $query .= ' ' . $filter_cond . ' ' . $where[$i];
    }
    $query .= ')';
  }


  // Finish off the query with the return statement.
  $query .= '
    RETURN r, n1, n2
  ';

  // Execute the query and return the parsed results.
  $results = tripal_neo4j_query($query, $params);
  foreach ($results->getRecords() as $record) {
    $edge = $record->get('r');
    $source = $record->get('n1');
    $target = $record->get('n2');
    $nodes[$source->identity()] = array(
      'id' => $source->identity(),
      'label' => $source->value('schema__name'),
      'x' => mt_rand(),
      'y' => mt_rand(),
      'size' => mt_rand(1, 3) / 1000,
      'color' => "gray",
    );
    $nodes[$target->identity()] = array(
      'id' => $target->identity(),
      'label' => $target->value('schema__name'),
      'x' => mt_rand(),
      'y' => mt_rand(),
      'size' => mt_rand(1, 3) / 1000,
      'color' => "gray",
    );
    $edges[$edge->identity()] = array(
      'id' => $edge->identity(),
      'source' => $source->identity(),
      'target' => $target->identity(),
      'size' => mt_rand(),
      'type' => 'straight',
      'color' => '#CCCCC',
      'hover_color' => '#A0A0A0',
      'sc' => $edge->value('Correlation_Method__sc')
    );
  }
  $network_data = array(
    'nodes' => array_values($nodes),
    'edges' => array_values($edges),
    'query' => $query,
    'params' => $params,
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


function tripal_network_viewer_neo4j_ajax_function() {
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
      INNER JOIN {cvterm} FCVT2 on FCVT2.cvterm_id = F.type_id
    WHERE
      F.name IN (:name) and
      O.genus = :genus and
      O.species = :species and
      FCVT2.name = :type
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
      $accession => $record->term_name
    );
  }
  return drupal_json_output($results);
}


/**
 * Theme function for the property filters.
 */
function theme_tripal_network_viewer_property($variables){

  $form = $variables['element'];

  $headers = array();
  $rows = array();
  foreach ($form as $element_key => $element) {
    if (preg_match('/^property_(\d+)_type$/', $element_key, $matches)){
      $id = $matches[1];

      $title = $element['#value'];
      if (preg_match('/__/', $title)) {
        list($property, $subprop) = preg_split('/__/', $title);
        $title = $property . ": " . $subprop;
      }
      $rows[($id * 2)][0] = array(
        'data' => $title,
        'colspan' => 4,
      );

      $rows[($id * 2) + 1][0]['data'] = drupal_render($element);
    }
    if (preg_match('/^property_(\d+)_op$/', $element_key, $matches)){
      $id = $matches[1];
      $rows[($id * 2) + 1][1]['data'] = drupal_render($element);
    }
    if (preg_match('/^property_(\d+)_value$/', $element_key, $matches)){
      $id = $matches[1];
      $rows[($id * 2) + 1][2]['data'] = drupal_render($element);
    }
    if (preg_match('/^property_(\d+)_rm$/', $element_key, $matches)){
      $id = $matches[1];
      $rows[($id * 2) + 1][3]['data'] = drupal_render($element);
    }
  }
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => array('tripal-network-viewer-prop-filters-table')
    ),
    'sticky' => TRUE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  return theme_table($table);
}

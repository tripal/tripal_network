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
function tripal_network_viewer_form($form, &$form_state, $network_id = NULL) {
 
  $module_id = 0;
  if (array_key_exists('values', $form_state)) { 
    $module_id = $form_state['values']['module_id'];
  }

  // Get the list of modules
  $sql = "
    SELECT * FROM {networkmod} NM 
    WHERE NM.network_id = :network_id
    ORDER BY NM.name
  ";
  $modules = [
    0 => '-- Select --',
  ];
  $results = chado_query($sql, [':network_id' => $network_id]); 
  while ($record = $results->fetchObject()) {
    $modules[$record->networkmod_id] = $record->name;
  }

  $form['network_id'] = [
    '#type' => 'value', 
    '#value' => $network_id,
  ];
  $form['module_id'] = [
    '#type' => 'select',
    '#title' => 'Module', 
    '#options' => $modules,
    '#description' => t('select a module to adjust the network view to show only that module.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

/*
  $sql = "
    SELECT NMT.trait_dbxref_id, NMT.trait_name, NMT.feature_type
    FROM {networkmod_traits} NMT
    WHERE network_id = :network_id
    ORDER BY NMT.feature_type, NMT.trait_name
  ";
  $traits = [
    0 => '-- Select --',
  ];
  $results = chado_query($sql, [':network_id' => $network_id]);
  while ($record = $results->fetchObject()) {
    $traits[$record->feature_type][$record->trait_dbxref_id] = $record->trait_name;
  }
  $form['trait_id'] = [
    '#type' => 'select',
    '#title' => 'Trait', 
    '#options' => $traits,
    '#description' => t('select a trait to adjust the network view to show only edges putatively associated with the trait.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#multiple' => FALSE,
  ];
*/
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
  $module_id = $form_state['values']['module_id'];
  $trait_id = $form_state['values']['trait_id'];

  $data = [
    'network_id' => $network_id,
  ];
  if ($module_id) {
    $data['module_id'] = $module_id;
  }
  if ($trait_id) {
    $data['trait_id'] = $trait_id;
  }

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "retrieveNetwork", array($data)));

  return $form;
}

/**
 *
 */
function tripal_network_viewer_filter_neo4j_form($form, &$form_state) {
  $form = array();

  tripal_neo4j_connect("bolt", "bolt://neo4j:testing123@localhost:7687");

  // Get the species from the session.
  $species = array_key_exists('values', $form_state) ? $form_state['values']['species'] : '';

  // Get the species that have network data and match species in Chado.
  // This requires the sepcies have the NCBI Taxonomy ID in the
  // organism_dbxref table to match the organisms.
  $query = "MATCH (n:Transcript) RETURN DISTINCT n.data__1179 AS taxId";
  $results = tripal_neo4j_query($query);
  $taxIds = array();
  foreach ($results->getRecords() as $record) {
    $taxIds[] = $record->value('taxId');
  }
  $sql = "
    SELECT O.*, DBX.accession as taxid
    FROM {organism} O
      INNER JOIN {organism_dbxref} ODBX on ODBX.organism_id = O.organism_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = ODBX.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE DB.name = 'NCBITaxon' AND DBX.accession IN (:taxIds)
  ";
  $results = chado_query($sql, array(':taxIds' => $taxIds));
  $species_options = array();
  $species_options['0'] = '-- Select --';
  while ($organism = $results->fetchObject()) {
    $species_options[$organism->taxid] = tripal_get_organism_scientific_name($organism);
  }

  // Construct the Species drop-down.
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
  // If the user identified the species then get the listof networks
  // for this species.
  if ($species) {
    $genes = array_key_exists('genes', $form_state['values']) ? $form_state['values']['genes'] : '';
    $prop_filters = array();
    $remove_filter = FALSE;
    if (array_key_exists('values', $form_state)) {
      // Find any existing filters.
      foreach (element_children($form_state['values']) as $element_key) {
        $matches = array();

        if (preg_match('/^property_(\d+)_type$/', $element_key, $matches)){
          $id = $matches[1];
          $prop_filters[$id]['type'] = $form_state['values'][$element_key];
        }
        if (preg_match('/^property_(\d+)_value$/', $element_key, $matches)){
          $id = $matches[1];
          $prop_filters[$id]['value'] = $form_state['values'][$element_key];
        }
        if (preg_match('/^property_(\d+)_op$/', $element_key, $matches)){
          $id = $matches[1];
          $prop_filters[$id]['op'] = $form_state['values'][$element_key];
        }
      }
      // Store the type for the new property.
      if (array_key_exists('new_property', $form_state['values'])) {
        $id = count($prop_filters);
        $prop_filters[$id]['type'] = $form_state['values']['new_property'];
      }
    }

    // Construct the Species drop-down.
    $form['genes'] = array(
      '#type' => 'textarea',
      '#title' => t('Genes'),
      '#default_value' => $genes,
      '#description' => t('Enter a list of gene, transcript or protein identifiers. Separate by spaces or new lines.'),
      '#cols' => 30,
      '#rows' => 5,
    );

    // Get the list of properties.
    $query = '
      MATCH (:Transcript)-[r:CO]-(:Transcript)
      WHERE r.data__1179 = $taxId
      RETURN DISTINCT KEYS(r) AS properties
    ';
    $params = array('taxId' => (int) $species);
    $results = tripal_neo4j_query($query, $params);

    $records =  $results->getRecords();
    $props = $records[0]->value('properties');
    $prop_options = array();
    $prop_options['0'] = '-- Select --';
    foreach ($props as $propname) {
      // Skip that taxonomy ID property as we've used that for flitering.
      if ($propname == 'data__1179') {
        continue;
      }
      // Only allow searching on properly formed property names. These names
      // must be vocabulary terms with the database and term separated
      // by two underscores.
      if (preg_match('/__/', $propname)) {
        list($property, $subprop) = preg_split('/__/', $propname);
        $prop_options[$property][$propname] = $property . ": " . $subprop;
      }
    }

    $form['property_filters'] = array(
      '#theme' => array('tripal_network_viewer_property')
    );
    foreach ($prop_filters as $id => $filter) {
      $propname = $filter['type'];
      $form['property_filters']['property_' . $id . '_type'] = array(
        '#type' => 'hidden',
        '#value' => $propname,
      );
      $form['property_filters']['property_' . $id . '_op'] = array(
        '#type' => 'select',
        '#options' => array(
          '=' => '=',
          '>=' => '>=',
          '<=' => '<=',
        ),
        '#default_value' => array_key_exists('op', $filter[$id]) ? $filter[$id]['op'] : '<=',
      );
      $form['property_filters']['property_' . $id . '_value'] = array(
        '#type' => 'textfield',
        '#default_value' => array_key_exists('value', $filter[$id]) ? $filter[$id]['value'] : '1e-4',
        '#size' => 10,
      );
      if ($id == count($prop_filters) - 1) {
        $form['property_filters']['property_' . $id . '_rm'] = array(
          '#type' => 'submit',
          '#value' => 'X',
          '#name' => 'prop_filter_remove',
          '#ajax' => array(
            'callback' => "tripal_network_viewer_filter_form_ajax_callback",
            'wrapper'  => 'tripal-network-viewer-filter-form',
            'effect'   => 'fade',
            'method'   => 'replace',
          ),
        );
      }
    }
    $form['new_property'] = array(
      '#type' => 'select',
      '#title' => t('Add a Property Filter'),
      '#options' => $prop_options,
      '#default_value' => '',
      '#ajax' => array(
        'callback' => "tripal_network_viewer_filter_form_ajax_callback",
        'wrapper'  => 'tripal-network-viewer-filter-form',
        'effect'   => 'fade',
        'method'   => 'replace',
      ),
    );

    $form['filter_cond'] = array(
      '#title' => 'Filter Conditional',
      '#type' => 'select',
      '#options' => array('AND' => 'AND', 'OR' => 'OR'),
      '#default_value' => 'AND',
    );

    $form['filter'] = array(
      '#type' => 'submit',
      '#value' => 'Load Network',
      '#name' => 'filter',
      '#ajax' => array(
        'callback' => "tripal_network_viewer_filter_form_ajax_callback",
        'wrapper'  => 'tripal-network-viewer-filter-form',
        'effect'   => 'fade',
        'method'   => 'replace',
      ),
    );

  }

  $form['#prefix'] = '<div id ="tripal-network-viewer-filter-form">';
  $form['#suffix'] = '</div>';
  $form['#attributes'] = array();
  return $form;
}
/**
 * Validate function for the tripal_network_viewer_filter_form form.
 */
function tripal_network_viewer_filter_neo4j_form_validate($form, &$form_state) {
  // If the remove filter button was clicked then we need to remove the
  // last filter from the form element and move the [X] button down to the
  // next element.
  if ($form_state['triggering_element']['#name'] == 'prop_filter_remove') {
    $rm_id = 0;
    foreach (element_children($form_state['values']) as $element_key) {
      $matches = array();
      if (preg_match('/^property_(\d+)_rm$/', $element_key, $matches)){
        $rm_id = $matches[1];
      }
    }
    unset($form_state['values']['property_' . $rm_id . '_op']);
    unset($form_state['values']['property_' . $rm_id . '_value']);
    unset($form_state['values']['property_' . $rm_id . '_type']);
    unset($form_state['values']['property_' . $rm_id . '_rm']);
    unset($form_state['values']['new_property']);
    $form_state['rebuild'] = TRUE;
  }
}


/**
 * Ajax function which returns the form via ajax
 */
function tripal_network_viewer_filter_neo4j_form_ajax_callback($form, $form_state) {
  $species = $form_state['values']['species'];

  // Get the genes, if specified.
  $genes = array_key_exists('genes', $form_state['values']) ? $form_state['values']['genes'] : '';
  if ($genes) {
    $genes = array_unique(preg_split('/[\s\n]+/', $genes));
  }

  // Get the filtering by properties if specified.
  $prop_filters = array();;
  if (array_key_exists('values', $form_state)) {
    // Find any existing filters.
    foreach (element_children($form_state['values']) as $element_key) {
      $matches = array();
      $rm_id = 0;

      if (preg_match('/^property_(\d+)_type$/', $element_key, $matches)){
        $id = $matches[1];
        $prop_filters[$id]['type'] = $form_state['values'][$element_key];
      }
      if (preg_match('/^property_(\d+)_value$/', $element_key, $matches)){
        $id = $matches[1];
        $prop_filters[$id]['value'] = $form_state['values'][$element_key];
      }
      if (preg_match('/^property_(\d+)_op$/', $element_key, $matches)){
        $id = $matches[1];
        $prop_filters[$id]['op'] = $form_state['values'][$element_key];
      }
    }
  }

  // Now that our form is updated we want to reload the network.  The network
  // is handled by JavaScript so we need to force the JS code to call our
  // tripal_network_viewer_ajax_retrieve() function.  The following code
  // will instruct Drupal to call a Javascript function named 'retreiveNetwork'
  // which will then make an AJAX call to update the network.
  if ($species) {
    $data = array(
      'species' => $species,
    );
    if (is_array($genes) and count($genes) > 0) {
      $data['genes'] = array_values($genes);
    }
    if (count($prop_filters) > 0) {
      $data['properties'] = $prop_filters;
    }
    $data['filter_cond'] = $form_state['values']['filter_cond'];

    if ($form_state['triggering_element']['#name'] == 'filter') {
      tripal_network_add_ajax_command(ajax_command_invoke(NULL, "retrieveNetwork", array($data)));
    }
  }
  return $form;
}

/**
 * Retreives details about a node when selected in the viewer.
 */
function tripal_network_viewer_ajax_node_retrieve() {

  $node_id = $_GET['node_id'];
  $feature = chado_generate_var('feature', ['feature_id' => $node_id]);

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
  $module_id = array_key_exists('module_id', $_GET) ? $_GET['module_id'] : '';
  $trait_id = array_key_exists('trait_id', $_GET) ? $_GET['trait_id'] : '';

  // Make sure we have required filters.
  if (!$network_id) {
    drupal_json_output($network_data);
    return;
  }

  // Get the list of edges and rows that match the query.
  $nodes = [];
  $edges = [];
  $params = [
    ':network_id' => $network_id
  ];

  // Get the edges from the network
  $select = "
    SELECT 
      F1.feature_id as node1_id, F1.name as node1, 
      F2.feature_id as node2_id, F2.name as node2, 
      NE.networkedge_id, NE.weight
   "; 
  $from = "
    FROM {networkedge} NE
      INNER JOIN {feature} F1 on F1.feature_id = NE.node1
      INNER JOIN {feature} F2 on F2.feature_id = NE.node2
  ";
  $where = "
    WHERE NE.network_id = :network_id
  ";
  $order = "
    ORDER BY NE.weight DESC
  ";
  $limit = "
    LIMIT 1000 OFFSET 0
  ";

  if ($module_id) {
    $from .= " INNER JOIN {networkmod_edge} NME ON NE.networkedge_id = NME.networkedge_id ";
    $where .= " AND NME.networkmod_id = :networkmod_id ";
    $params[':networkmod_id'] = $module_id;
  }

  if ($trait_id) {
    $from .= " INNER JOIN {networkmod_exp_traits} NMET ON NE.network_id = NMET.network_id ";
    $where .= "  AND NMET.trait_dbxref_id = :trait_id ";
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
    $edges[$record->networkedge_id] = array(
      'id' => $record->networkedge_id,
      'source' => $record->node1_id, 
      'target' => $record->node2_id,
      'size' => mt_rand(),
      'type' => 'straight',
      'color' => '#CCCCC',
      'hover_color' => '#A0A0A0',
      'sc' => $record->weight,
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

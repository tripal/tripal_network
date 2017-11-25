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
    $properties = array();
    $property_values = array();
    $property_ops = array();
    if (array_key_exists('values', $form_state)) {
      foreach (element_children($form_state['values']) as $element_key) {
        $matches = array();

        if (preg_match('/^property_(\d+)$/', $element_key, $matches)){

          $id = $matches[1];
          $properties[$id] = $form_state['values'][$element_key];
        }
        if (preg_match('/^property_(\d+)_value$/', $element_key, $matches)){
          $id = $matches[1];
          $property_values[$id] = $form_state['values'][$element_key];
        }
        if (preg_match('/^property_(\d+)_op$/', $element_key, $matches)){
          $id = $matches[1];
          $property_ops[$id] = $form_state['values'][$element_key];
        }
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
      MATCH ()-[r:CO]-()
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

   foreach ($properties as $id => $propname) {
     $form['property_' . $id] = array(
       '#type' => 'hidden',
       '#value' => $propname,
       // It doesn't make sense to put an #ajax callback on a hidden
       // element, but we need it because of the way we're re-using the
       // name of this hidden field. It used to be a non-hidden field,
       // but if we don't preseve the ajax settings then it confuses Drupal.
       '#ajax' => array(
         'callback' => "tripal_network_viewer_filter_form_ajax_callback",
         'wrapper'  => 'tripal-network-viewer-filter-form',
         'effect'   => 'fade',
         'method'   => 'replace',
       ),
     );
     $title = $propname;
     if (preg_match('/__/', $propname)) {
       list($property, $subprop) = preg_split('/__/', $propname);
       $title = $property . ": " . $subprop;
     }
     $form['property_' . $id . '_op'] = array(
       '#type' => 'select',
       '#options' => array(
         '=' => '=',
         '>=' => '>=',
         '<=' => '<=',
       ),
       '#default_value' => array_key_exists($id, $property_ops) ? $property_ops[$id] : '',
     );
     $form['property_' . $id . '_value'] = array(
       '#type' => 'textfield',
       '#title' => $title,
       '#default_value' => array_key_exists($id, $property_values) ? $property_values[$id] : '',
       '#size' => 10,
     );
   }
   $form['property_' . count($properties)] = array(
     '#type' => 'select',
     '#title' => t('Add a Property Filter'),
     '#options' => $prop_options,
     '#default_value' => $property,
     '#description' => t(''),
     '#ajax' => array(
       'callback' => "tripal_network_viewer_filter_form_ajax_callback",
       'wrapper'  => 'tripal-network-viewer-filter-form',
       'effect'   => 'fade',
       'method'   => 'replace',
     ),
   );

   $form['filter'] = array(
     '#type' => 'submit',
     '#value' => 'Perform Filter',
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
 * Ajax function which returns the form via ajax
 */
function tripal_network_viewer_filter_form_ajax_callback($form, &$form_state) {
  $species = $form_state['values']['species'];

  // Get the genes, if specified.
  $genes = array_key_exists('genes', $form_state['values']) ? $form_state['values']['genes'] : '';
  if ($genes) {
    $genes = array_unique(preg_split('/[\s\n]+/', $genes));
  }

  // Get the filtering by properties if specified.
  $properties = array();
  $property_values = array();
  $property_ops = array();
  foreach (element_children($form_state['values']) as $element_key) {
    $matches = array();
    if (preg_match('/^property_(\d+)$/', $element_key, $matches)){
      $id = $matches[1];
      $properties[$id] = $form_state['values'][$element_key];
    }
    if (preg_match('/^property_(\d+)_value$/', $element_key, $matches)){
      $id = $matches[1];
      $property_values[$id] = $form_state['values'][$element_key];
    }
    if (preg_match('/^property_(\d+)_op$/', $element_key, $matches)){
      $id = $matches[1];
      $property_ops[$id] = $form_state['values'][$element_key];
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
    $retreive_network = FALSE;
    if (is_array($genes) and count($genes) > 0) {
      $data['genes'] = array_values($genes);
      $retrieve_network = TRUE;
    }
    if (count($properties) > 0) {
      foreach($properties as $id => $propname) {
        if ($property_values[$id]) {
          $data['properties'][$propname] = array(
            'value' => $property_values[$id],
            'op' => $property_ops[$id],
          );
        }
      }
      $retrieve_network = TRUE;
    }
    if ($retrieve_network == TRUE) {
      tripal_network_add_ajax_command(ajax_command_invoke(NULL, "retrieveNetwork", array($data)));
    }
  }
  return $form;
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
  $species = $_GET['species'];
  $genes = $_GET['genes'];
  $properties = $_GET['properties'];

  // Make sure we have required filters.
  if (!$species) {
    drupal_json_output($network_data);
    return;
  }

  if (count($genes) == 0 and count(array_keys($properties)) == 0){
    drupal_json_output($network_data);
    return;
  }

  // Get the list of edges and rows that match the query.
  $nodes = array();
  $edges = array();
  $params = array();

  // Build the initial query to search for nodes by taxonomy ID.
  tripal_neo4j_connect("bolt", "bolt://neo4j:testing123@localhost:7687");
  $query = 'MATCH(n1:Transcript)-[r:CO]-(n2:Transcript)
    WHERE (n1.data__1179 = $taxId OR n2.data__1179 = $taxId) ';
  $params['taxId'] =(int) $species;

  // If the user is filtering by gene name then add in that statement
  if (count($genes) > 0) {
    $query .= 'AND (n1.schema__name IN $genes OR n2.schema__name IN $genes))';
    $params['genes'] = $genes;
  }

  // If the user is filtering by properties then add in those statements.
  if (count($properties) > 0) {
    $id = 0;
    foreach ($properties as $propname => $details) {
      $query .= ' AND (r.' . $propname . ' ' . $details['op'] . ' $prop' . $id .  ')';
      $params['prop' . $id] = (float) $details['value'];
      $id++;
    }

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
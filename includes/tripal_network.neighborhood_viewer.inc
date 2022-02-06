<?php 

/**
 *
 */
function tripal_network_neighborhood_viewer_ajax_retrieve() {
  
  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;
  
  // Create the network instance
  $network = new TripalNetwork([
    'organism_id' => $organism_id, 
    'network_ids' => $network_id,
    'feature_ids' => [$feature_id]
  ]);
  
  // Load the network data
  $network->loadNetwork();  
  
  // Generate the plotly data.
  $viewer = new TripalNetworkViewerPlotly($network);
  $plotly_data = $viewer->getNetworkPlot();
  $plotly_data['layout']['title'] = 'Connected Neighbors';      
  drupal_json_output($plotly_data);
}

/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $feature_id
 * @param unknown $network_id
 * @return unknown
 */
function tripal_network_neighborhood_viewer_form($form, &$form_state, $feature_id = NULL) {
  
  global $user;
  
  $feature = new ChadoRecord('feature');
  $feature->setValue('feature_id', $feature_id);
  $feature->find();
  
  // Get the list of modules
  $sql = '';
  $args = [':feature_id' => $feature_id];
  $sql = "
    SELECT N.network_id, N.name
    FROM {network} N
      INNER JOIN {network_node} NN on NN.network_id = N.network_id
      INNER JOIN {network_feature} NF on NF.network_node_id = NN.network_node_id
    WHERE NF.feature_id = :feature_id
    ORDER BY N.name
  ";
  $networks = [
    0 => '-- Select --',
  ];
  $results = chado_query($sql, $args);
  while ($record = $results->fetchObject()) {
    // Show networks that have been published and that the user has acces to.
    $entity_id = chado_get_record_entity_by_table('network', $record->network_id);
    if ($entity_id){
      $entity = tripal_load_entity('TripalEntity', [$entity_id]);
      if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
        $networks[$record->network_id] = $record->name;
      }
    }
  }
  
  $form['organism_id'] = [
    '#type' => 'value',
    '#value' => $feature->getValue('organism_id'),
    '#prefix' => '<div id="tripal-network-neighborhood-viewer-form">',
  ];
  $form['feature_id'] = [
    '#type' => 'value',
    '#value' => $feature_id,
  ];
  $form['network_id'] = [
    '#type' => 'select',
    '#title' => 'Network',
    '#options' => $networks,
    '#description' => t('Select the network to view connections for this feature.'),
    '#ajax' => array(
      'callback' => 'tripal_network_neighborhood_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-neighborhood-viewer-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];
  $form['view_link'] = [
    '#type' => 'markup',
    '#markup' => '',
    '#suffix' => '</div>',
  ];
  return $form;
}

/**
 *
 */
function tripal_network_neighborhood_viewer_form_ajax_callback($form, $form_state) {
  $organism_id = $form_state['values']['organism_id'];
  $network_id = $form_state['values']['network_id'];
  $feature_id = $form_state['values']['feature_id'];
  
  $args = [
    'feature_id' => $feature_id,
    'network_id' => $network_id,
    'organism_id' => $organism_id,
    'viewer_id' => 'tripal-network-neighborhood-viewer'
  ];
  
  // Tell Drupal to update the network.
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetworkNeighborhood", [$args]));
  
  // Add a link to explore the full network.
  if ($network_id) {
    $form['view_link']['#markup'] = l('Explore the full network','networks/viewer',
        ['attributes' => ['target' => '_blank'],
          'html' => FALSE,
          'query' => ['organism_id' => $organism_id, 'network_id' => $network_id, 'feature_id' => $feature_id]
        ]);
  }
  else {
    $form['view_link']['#markup'] = '';
  }
  return $form;
}
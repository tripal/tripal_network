<?php

/**
 *
 */
function tripal_network_viewer_page($organism_id = NULL, $network_id = NULL) {
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'drupal.form');
  $args = ['organism_id' => $organism_id, 'network_id' => $network_id];
  return theme('tripal_network_viewer', $args);
}

function tripal_network_viewer_ajax_init() {

  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;

  $init_state = [
    'network_id' => $network_id,
    'organism_id' => $organism_id,
    'feature_id' => $feature_id,
    'selected_node' => null,
    'selected_edge' => null,
    'layer_by' => 'SWO:0000425',
    'color_by' => 'SWO:0000425',
    'limit' => 250,
    'limit_by' => 'SWO:0000425'
  ];
  drupal_json_output($init_state);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_layers_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_layers_form', $organism_id, $network_id, $state);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_filter_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $limit = array_key_exists('limit', $_GET) ? $_GET['limit'] : 10;
  $limit_by = array_key_exists('limit_by', $_GET) ? $_GET['limit_by'] : NULL;
  
  $form = drupal_get_form('tripal_network_viewer_filter_form', $organism_id, $network_id, $limit, $limit_by);
  $form = drupal_render($form);
  
  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_details() {
  $node_id = array_key_exists('node_id', $_GET) ? $_GET['node_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_node_details_form', $node_id, $state);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_network_details_form($form, &$form_state, $network_details = NULl) {
  
  $details = 'Please select a network to view details.';
  if ($network_details) {
    $rows = [];
    $headers = [];
    $rows[] = [
      [
        'data' => 'Number of Edges',
        'header' => TRUE,
        'width' => '70%',
      ],
      count(array_keys($network_details['edges'])),
    ];
    $rows[] = [
      [
        'data' => 'Number of Nodes',
        'header' => TRUE,
        'width' => '70%',
      ],
      count(array_keys($network_details['nodes'])),
    ];
    $headers = [];
    $summary_table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [],
      'caption' => '',
      'sticky' => TRUE,
      'colgroups' => [],
      'empty' => 'No attributes for this node',
    ];
    $details = theme_table($summary_table);
  }
  
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
  ];
  return $form;
}

/**
 *
 */
function tripal_network_viewer_node_details_form($form, &$form_state, $node_id = NULL, $state = NULL) {

  $default_field = 'summary';
  if ($state and array_key_exists('node_field_name', $state)) {
    $default_field = $state['node_field_name'];
  }
  if (array_key_exists('values', $form_state)) {
    $node_id = array_key_exists('node_id', $form_state['values']) ? $form_state['values']['node_id'] : $node_id;
    $default_field = array_key_exists('field_name', $form_state['values']) ? $form_state['values']['field_name'] : $default_field;
  }

  $table_fields = [];
  $other_fields = [];
  $selectable_fields = [
    'summary' => 'Node Summary'
  ];
  $entity = NULL;

  $node_name = 'Click a node to view its details';
  if ($node_id) {
    $feature = tripal_network_get_nodes([$node_id])[$node_id];
    
    $entity_id = chado_get_record_entity_by_table('feature', $feature->feature_id);
    $entities = tripal_load_entity('TripalEntity', [$entity_id]);
    field_attach_load('TripalEntity', $entities);
    $entity = NULL;
    if (count($entities) == 1) {
      $entity = $entities[$entity_id];
      $bundle = tripal_load_bundle_entity(['name' => $entity->bundle]);

      // Get information about the fields attached to this bundle and sort them
      // in the order they were set for the display.
      $instances = field_info_instances('TripalEntity', $bundle->name);
      foreach ($instances as $field_name => $instance) {

        // Skip hidden fields.
        if ($instance['display']['default']['type'] == 'hidden') {
          continue;
        }
        // Skip the network local viewer field
        if ($field_name == 'sio__network_diagram') {
          continue;
        }

        // Skip fields with no value
        $field = field_info_field($field_name);
        $field_items = field_get_items('TripalEntity', $entity, $field_name);
        if (tripal_field_is_empty($field, $field_items)) {
         // continue;
        }

        // Add this field for display in the summary table.
        if ($field['cardinality'] == 1) {
          $table_fields[$instance['display']['default']['weight']] = $instance;
        }
        else {
          $selectable_fields[$field_name] = $instance['label'];
          $other_fields[$field_name] = $instance;
        }
      }

      // Set the title for the panel.
      $node_name = $entity->title;
    }

    $details = '';
    if ($default_field == 'summary') {
      $rows = [];
      foreach ($table_fields as $instance) {
        $field_name  = $instance['field_name'];
        $display = $instance['display']['default'];
        $value = $entity->$field_name['und'][0]['value'];
        if ($instance['entity_type'] == 'TripalEntity') {
          $field = field_info_field($field_name);
          $items = field_get_items('TripalEntity', $entity, $field_name);
          $formatter_class = $display['type'];
          $element = [];
          if (tripal_load_include_field_class($formatter_class)) {
            $element=[];
            $formatter = new $formatter_class($field, $instance);
            $formatter->view($element, 'TripalEntity', $entity, 'und', $items, $display);
            $value = drupal_render($element);
          }
        }
        $rows[] = [
          [
            'data' => $instance['label'],
            'header' => TRUE,
            'width' => '20%',
          ],
          $value,
        ];
      }
      $headers = ['Attribute', 'Value'];
      $summary_table = [
        'header' => $headers,
        'rows' => $rows,
        'attributes' => [],
        'caption' => '',
        'sticky' => TRUE,
        'colgroups' => [],
        'empty' => 'No attributes for this node',
      ];
      $details = theme_table($summary_table);
    }
    else {
      $field = field_info_field($default_field);
      $instance = $other_fields[$default_field];
      $items = field_get_items('TripalEntity', $entity, $default_field);
      $display = $instance['display']['default'];
      $formatter_class = $display['type'];
      $element = [];
      if (tripal_load_include_field_class($formatter_class)) {
        $element = [];
        $formatter = new $formatter_class($field, $instance);
        $formatter->view($element, 'TripalEntity', $entity, 'und', $items, $display);
        $details = drupal_render($element);
      }
    }


    $form['node_id'] = [
      '#type' => 'value',
      '#value' => $node_id,
      '#prefix' => '<div id="tripal-network-viewer-node-details-form">',
    ];
    $form['feature_name'] = [
      '#type' => 'item',
      '#title' => 'Node Name',
      '#markup' => $node_name,
    ];
    $form['field_name'] = [
      '#type' => 'select',
      '#title' => 'Node Information',
      '#options' => $selectable_fields,
      '#default_value' => $default_field,
      '#description' => t('Select the type of information to show about this node.'),
      '#ajax' => array(
        'callback' => 'tripal_network_viewer_node_details_form_ajax_callback',
        'wrapper'  => 'tripal-network-viewer-node-details-form',
        'effect'   => 'fade',
        'method'   => 'replace',
      ),
    ];


    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
      '#prefix' => '<div id="tripal-network-viewer-node-details-details">',
      '#suffix' => '</div>',
    ];
    $link = '';
    if ($entity) {
      $link = l("View full node page", 'bio_data/'. $entity->id, ['attributes' => ['target' => '_blank']]);
    }
    $form['link'] = [
      '#type' => 'markup',
      '#markup' => '<div id="tripal-network-viewer-node-link">' . $link  . '</div>',
      '#suffix' => '</div>',
    ];
  }
  else {
    $form['feature_name'] = [
      '#type' => 'markup',
      '#markup' => $node_name,
    ];
  }


  return $form;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_edge_details() {
  $node_id = array_key_exists('edge_id', $_GET) ? $_GET['edge_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_edge_details_form', $node_id);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_edge_details_form($form, &$form_state, $edge_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $edge_id = array_key_exists('edge_id', $form_state['values']) ? $form_state['values']['edge_id'] : $edge_id;
  }

  $edge_name = 'Click an edge to view its details';
  $net_name = '';
  $details = '';
  if ($edge_id) {
    $sql = "
      SELECT N.name as net_name, FS.name as source, FT.name as target
      FROM {network_edge} NE
        INNER JOIN {network} N on NE.network_id = N.network_id
        INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
        INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
        INNER JOIN {network_feature} NFS on NNS.network_node_id = NFS.network_node_id
        INNER JOIN {network_feature} NFT on NNT.network_node_id = NFT.network_node_id
        INNER JOIN {feature} FS on FS.feature_id = NFS.feature_id
        INNER JOIN {feature} FT on FT.feature_id = NFT.feature_id
     WHERE NE.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);
    $edge = $results->fetchObject();
    $edge_name = $edge->source . ' (-) ' . $edge->target;
    $net_name = $edge->net_name;

    $sql = "
      SELECT CVT.name, NEP.value, DBX.accession, DB.name as db_name
      FROM {network_edgeprop} NEP
        INNER JOIN {cvterm} CVT ON CVT.cvterm_id = NEP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE NEP.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);

    $rows = [];
    while ($prop = $results->fetchObject()) {
      if ($prop->db_name . ':' . $prop->accession == 'local:KINC_sample_string') {
        continue;
      }
      $rows[] = [
        [
          'data' => ucfirst($prop->name),
          'header' => TRUE,
        ],
        $prop->value,
      ];
    }
    $headers = ['Attribute', 'Value'];
    $summary_table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [],
      'caption' => '',
      'sticky' => TRUE,
      'colgroups' => [],
      'empty' => 'No attributes for this edge',
    ];
    $details = theme_table($summary_table);
  }

  $form['edge_id'] = [
    '#type' => 'value',
    '#value' => $edge_id,
  ];
  $form['edge_name'] = [
    '#type' => 'item',
    '#title' => 'Edge Name',
    '#markup' => $edge_name,
  ];
  $form['net_name'] = [
    '#type' => 'item',
    '#title' => 'Network Name',
    '#markup' => $net_name,
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
    '#prefix' => '<div id="tripal-network-viewer-edge-details-details">',
    '#suffix' => '</div>',
  ];
  return $form;
}


function tripal_network_viewer_has_access($organism_id = NULL, $network_id = NULL) {
  global $user;

  if ($organism_id) {
    // Don't show an organism that the user doesn't have access to or isn't
    // published.
    $entity_id = chado_get_record_entity_by_table('organism', $organism_id);
    if (!$entity_id){
      return FALSE;
    }
    $entity = tripal_load_entity('TripalEntity', [$entity_id]);
    if (!tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
      return FALSE;
    }
  }

  if ($network_id) {
    // Don't show a network that the user doesn't have access to or isn't
    // published.
    $entity_id = chado_get_record_entity_by_table('network', $network_id);
    if (!$entity_id){
      return FALSE;
    }
    $entity = tripal_load_entity('TripalEntity', [$entity_id]);
    if (!tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
      return FALSE;
    }
  }
  return TRUE;
}
/**
 *
 */
function tripal_network_viewer_ajax_retrieve() {

  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;
  $limit = array_key_exists('limit', $_GET) ? $_GET['limit'] : 250;
  $limit_by = array_key_exists('limit_by', $_GET) ? $_GET['limit_by'] : NULL;
  $features = array_key_exists('features', $_GET) ? $_GET['features'] : NULL;
  $vocab_terms = array_key_exists('vocab_terms', $_GET) ? $_GET['vocab_terms'] : NULL;

  $layout = tripal_network_get_plotly_default_layout();

  // TODO: this shoulnd't be hardcoded.
  if (!$layer_by) {
    $layer_by = 'SWO:0000425';
  }
  if (!$color_by) {
    $color_by = 'SWO:0000425';
  }

  if (!tripal_network_viewer_has_access($organism_id, $network_id)) {
    drupal_set_message('Permission denied', 'error');
    $organism_id = 0;
  }

  // If we don't have a organism ID then return an empty scene.
  $plotly_data = NULL;
  if (!$organism_id) {
    $plotly_data = tripal_network_get_plotly_data(NULL, $layout, $layer_by, $color_by, TRUE);
    $details = drupal_render(drupal_get_form('tripal_network_viewer_network_details_form'));    
    drupal_json_output(['data' => $plotly_data, 'layout' => $layout, 'details' => $details]);
    return;
  }

  // If teh viewer was pre-loaded with a feature then add it to the list. 
  $feature_ids = [];
  if ($feature_id) {
    $feature_ids[] = $feature_id;
  }
  
  // If the user has request some feature names then limit the network
  // to only those features.  
  if ($features) {
    $features = preg_split('/\s+/', $features);
    $sql = "
      SELECT DISTINCT F.feature_id
      FROM {feature} F
        INNER JOIN {network_feature} NN on NN.feature_id = F.feature_id
        LEFT JOIN {feature_synonym} FS on FS.feature_id = F.feature_id
        LEFT JOIN {synonym} S on S.synonym_id = FS.synonym_id
      WHERE
        lower(F.name) = :name OR
        lower(F.uniquename) = :name OR
        lower(S.name) = :name
    ";
    foreach ($features as $fname) {
      $args = [':name' => strtolower($fname)];
      $feature = chado_query($sql, $args)->fetchObject();
      if ($feature) {
        $feature_ids[] = $feature->feature_id;
      }
    }
  }

  $term_ids = [];
  if ($vocab_terms) {
    $terms = preg_split('/\s+/', $vocab_terms);
    foreach ($terms as $term) {
      // TODO: add support for children temr
      $cvterm = chado_get_cvterm(['id' => $term]);
      if ($cvterm) {
        $term_ids[] = $cvterm->cvterm_id;
      }
    }
  }


  // Get the network Plotly data.
  $network_items = tripal_network_get_network_elements([
    'organism_id' => $organism_id, 'network_id' => $network_id,
    'feature_ids' => $feature_ids, 'term_ids' => $term_ids,
    'limit_by' => $limit_by, 'limit' => $limit]);
  
  
  // Add in the degree to the nodes.
  tripal_network_calculate_degree($network_items);
  
  // Generate the plotly data.
  $plotly_data = tripal_network_get_plotly_data($network_items, $layout, $layer_by, $color_by);
  
  $details = drupal_render(drupal_get_form('tripal_network_viewer_network_details_form', $network_items));

  drupal_json_output([
    'data' => $plotly_data, 
    'layout' => $layout,
    'details' => $details
  ]);
}

/**
 *
 */
function tripal_network_viewer_network_form($form, &$form_state, $organism_id = NULL,
    $network_id = NULL, $feature_id = NULl) {

  global $user;
  
  $features = '';
  if ($feature_id) {
    $feature = new ChadoRecord('feature');
    $feature->setValue('feature_id', $feature_id);
    if ($feature->find()) {
      $features = $feature->getValue('name');
    }
  }

  
  $vocab_terms = '';

  if (array_key_exists('values', $form_state)) {
    $features = array_key_exists('features', $form_state['values']) ? $form_state['values']['features'] : $features;
    $vocab_terms = array_key_exists('vocab_terms', $form_state['values']) ? $form_state['values']['vocab_terms'] : $vocab_terms;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
  }
  if (array_key_exists('input', $form_state)) {
    $features = array_key_exists('features', $form_state['input']) ? $form_state['input']['features'] : $features;
    $vocab_terms = array_key_exists('vocab_terms', $form_state['input']) ? $form_state['input']['vocab_terms'] : $vocab_terms;
    $network_id = array_key_exists('network_id', $form_state['input']) ? $form_state['input']['network_id'] : $network_id;
    $organism_id = array_key_exists('organism_id', $form_state['input']) ? $form_state['input']['organism_id'] : $organism_id;
  }
  
  $organism = NULL;
  if ($organism_id) {
    $organism = new ChadoRecord('organism');
    $organism->setValues(['organism_id' => $organism_id]);
    $organism->find();
  }
  
  $networks = [];
  $organisms = [
    0 => '-- Select --',
  ];
  
  // Get the list of organisms with networks.
  $args = [];
  $sql = "
      SELECT O.organism_id, O.genus, O.species
      FROM {network} N
        INNER JOIN {organism} O on O.organism_id = N.organism_id
      ORDER BY O.genus, O.species
  ";
  $results = chado_query($sql, $args);
  while ($record = $results->fetchObject()) {
    // Show networks that have been published and that the user has acces to.
    $entity_id = chado_get_record_entity_by_table('organism', $record->organism_id);
    if ($entity_id){
      $entity = tripal_load_entity('TripalEntity', [$entity_id]);
      if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
        $organisms[$record->organism_id] = $record->genus . ' ' . $record->species;
      }
    }
  }

  // Get the list of all networks
  if ($organism) {    
    $sql = '';
    $args = [];
    $sql = "
      SELECT N.name, N.network_id, CVT.name as cv_name
      FROM {network} N
        INNER JOIN {cvterm} CVT on CVT.cvterm_id = N.type_id
      WHERE N.organism_id = :organism_id
      ORDER BY N.name
    ";
    $args[':organism_id'] = $organism->getID();
    $results = chado_query($sql, $args);
    while ($record = $results->fetchObject()) {
      // Show networks that have been published and that the user has acces to.
      $entity_id = chado_get_record_entity_by_table('network', $record->network_id);
      if ($entity_id){
        $entity = tripal_load_entity('TripalEntity', [$entity_id]);
        if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
          $networks[$record->cv_name][$record->network_id] = $record->name;
        }
      }
    }
  }

  $form['organism_id'] = [
    '#type' => 'select',
    '#title' => 'Species',
    '#options' => $organisms,
    '#default_value' => $organism_id,
    '#description' => t('Select the orgnism to view networks.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_network_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-network-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#prefix' => '<div id="tripal-network-viewer-network-form">',
  ];

  $title = 'Available Networks';
  if ($organism) {
    $title = 'Available ' . $organism->getValue('genus') . ' ' . $organism->getValue('species') . ' Networks';
  }

  $form['features'] = [
    '#type' => 'textarea',
    '#rows' => 3,
    '#title' => 'Feature Names',
    '#default_value' => $features,
    '#description' => t('Limit the edges to only those that connect
       features (e.g., gene, transcript, or protein names). Separate each
       name by a space or new line.'),
    '#multiple' => TRUE,
  ];
  $form['vocab_terms'] = [
    '#type' => 'textarea',
    '#rows' => 3,
    '#title' => 'Vocabulary Terms',
    '#default_value' => $vocab_terms,
    '#description' => t('Limit the edges to only those whose nodes are
       annotated with sepcific controlled vocabulary terms (e.g. GO Terms,
       IPR, KEGG ,etc).'),
    '#multiple' => TRUE,
  ];

  $form['network_id'] = [
    '#type' => 'select',
    '#title' => $title,
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('Limit the edges to only the selected networks.'),
    '#multiple' => TRUE,
    '#size' => 5,
  ];
  
  $form['load_network_button'] = [
    '#type' => 'button',
    '#value' => 'Load Network',
    '#name' => 'load_network_button',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_network_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-network-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

  $form['done'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];
  $organism_id = $form_state['values']['organism_id'];
  $features = $form_state['values']['features'];
  $vocab_terms = $form_state['values']['vocab_terms'];

  $args = [
    'organism_id' => NULL,
    'network_id' => NULL,
    'feature_id' => NULL,
    'features' => NULL,
    'vocab_terms' => NULL,
  ];
  if ($organism_id) {
    $args['organism_id'] = $organism_id;
  }

  if ($network_id) {
    $args['network_id'] = implode('|', $network_id);
  }
  if ($features) {
    $args['features'] = $features;
  }
  if ($vocab_terms) {
    $args['vocab_terms'] = $vocab_terms;
  }

  // Tell Drupal to update the network
  if ($form_state['triggering_element']['#name'] == 'load_network_button') {
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "updateFilterForm"));
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "updateLayersForm"));
  }

  return $form;
}


/**
 *
 */
function tripal_network_viewer_filter_form($form, &$form_state, $organism_id = NULL, 
    $network_id = NULL, $limit = 250, $limit_by = NULL) {
  
  $details = 'Please select a network to view details.';
  if (!$organism_id) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
    ];
  } 
  else {

    if (array_key_exists('values', $form_state)) {
      $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : $limit;
      $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
    }
    
    $props = [];
    if ($organism_id) {
      $props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, FALSE, TRUE, FALSE);
    }
  
    $form['limit'] = [
      '#type' => 'textfield',
      '#title' => 'Limit Number of Edges',
      '#size' => 10,
      '#default_value' => $limit,
      '#description' => t('Limit the number of edges displayed.'),
    ];
    $form['limit_by'] = [
      '#type' => 'select',
      '#title' => 'Limit By',
      '#options' => $props,
      '#default_value' => $limit_by,
      '#description' => t('Select a property to limit the best edges (only works for numerical properties).'),
    ];
  
    $form['network_filter_button'] = [
      '#type' => 'button',
      '#name' => 'network_filter_button',
      '#value' => 'Perform Filter',
      '#ajax' => array(
        'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
        'wrapper' => 'tripal-network-viewer-filter-form',
        'effect' => 'fade',
        'method' => 'replace',
      ),
      '#suffix' => '</div>',
    ];
  }

  return $form;
}

/**
 *
 */
function tripal_network_viewer_layers_form($form, &$form_state,
    $organism_id = NULl, $network_id = NULL, $state = NULL) {

  $layer_by = '';
  $color_by = '';

  if (is_array($state)){
    $layer_by = array_key_exists('layer_by', $state) ? $state['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $state) ? $state['color_by'] : $color_by;
  }

  if (array_key_exists('values', $form_state)) {
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : $color_by;
  }
  $props = [];
  if ($organism_id) {
    $props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, TRUE, TRUE, FALSE);
  }

  $form['network_id'] = [
    '#type' => 'value',
    '#value' =>  $network_id,
    '#prefix' => '<div id="tripal-network-viewer-layers-form">',
  ];
  $form['layer_by'] = [
    '#type' => 'select',
    '#title' => 'Layer Edges By',
    '#options' => $props,
    '#default_value' => $layer_by,
    '#description' => t('Select a property to layer edges.'),
  ];

  $form['color_by'] = [
    '#type' => 'select',
    '#title' => 'Color Edges By',
    '#options' => $props,
    '#default_value' => $color_by,
    '#description' => t('Select a property to color edges.'),
  ];

  $form['network_layer_button'] = [
    '#type' => 'button',
    '#value' => 'Update Display',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_layers_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-layers-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#suffix' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_node_details_form_ajax_callback($form, $form_state){

  $field_name = NULL;
  if (array_key_exists('values', $form_state)) {
    $field_name = array_key_exists('field_name', $form_state['values']) ? $form_state['values']['field_name'] : NULL;
  }
  $args = [
    'node_field_name' => $field_name,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "setState", [$args]));
  return $form;
}

/**
 *
 */
function tripal_network_viewer_filter_form_ajax_callback($form, $form_state) {
  $limit = NULL;
  $limit_by = NULL;
  if (array_key_exists('values', $form_state)) {
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : NULL;
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : NULL;
  }

  $args = [
    'limit' => $limit,
    'limit_by' => $limit_by,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));

  return $form;
}
/**
 *
 */
function tripal_network_viewer_layers_form_ajax_callback($form, $form_state) {

  $layer_by = NULL;
  $color_by = NULL;
  if (array_key_exists('values', $form_state)) {
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : NULL;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : NULL;
  }

  $args = [
    'layer_by' => $layer_by,
    'color_by' => $color_by,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));

  return $form;
}
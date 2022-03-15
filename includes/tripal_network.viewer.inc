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

/**
 * 
 */
function tripal_network_viewer_ajax_init() {

  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;
  $network_session_id = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  if (!$network_session_id) {
    $network_session_id = drupal_random_key(22);
  }
  
  $init_state = [
    'network_session_id' => $network_session_id,
    'network_id' => $network_id,
    'organism_id' => $organism_id,
    'feature_id' => $feature_id,
    'selected_node' => null,
    'selected_edge' => null,
    'layer_by' => 'NCIT:C48834',
    'color_by' => 'NCIT:C48834',
    'filters' => [
      0 => [
        'limit' => 500,
        'limit_by' => 'NCIT:C48834',
        'order_by' => 'DESC',
        'use_abs' => 1
      ],
    ]
  ];
  drupal_json_output($init_state);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_neighbors() {
  $network_session_id = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  $node_id = array_key_exists('node_id', $_GET) ? $_GET['node_id'] : NULL;
  $network = tripal_network_viewer_get_session_network($network_session_id);
  $edges = $network->getEdges();
    
  $neighbors = [$node_id];
  foreach ($edges as $edge_id => $edge) {
    if ($edge['source_node_id'] == $node_id) {
      $neighbors[] = $edge['target_node_id'];
    }
    else if ($edge['target_node_id'] == $node_id) {
      $neighbors[] = $edge['source_node_id'];
    }
  }
  drupal_json_output(array_unique($neighbors));
}

/**
 * 
 */
function tripal_network_viewer_ajax_set_node_selection() {
  $network_session_id = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  $selected_nodes = array_key_exists('selected_nodes', $_GET) ? $_GET['selected_nodes'] : NULL;
  
  // Set the selected nodes for the network.
  $network = tripal_network_viewer_get_session_network($network_session_id);  
  $network->setSelectedNodes($selected_nodes);
  tripal_network_viewer_set_session_network($network, $network_session_id);

  drupal_json_output(TRUE);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_layers_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_layers_form', $network_session_id, 
      $organism_id, $network_id, $layer_by, $color_by);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_filter_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $filters = array_key_exists('filters', $_GET) ? $_GET['filters'] : [];
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  
 
  $form = drupal_get_form('tripal_network_viewer_filter_form', $network_session_id, 
      $organism_id, $network_id, $filters);
  $form = drupal_render($form);
  
  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_analysis_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  
  
  $form = drupal_get_form('tripal_network_viewer_analysis_form', $network_session_id,
      $organism_id, $network_id);
  $form = drupal_render($form);
  
  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_edge_data_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $filters = array_key_exists('filters', $_GET) ? $_GET['filters'] : [];
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  $page = array_key_exists('page', $_GET) ? $_GET['page'] : 0;
  $edata_includes = array_key_exists('edata_includes', $_GET) ? $_GET['edata_includes'] : [];
  $sort_by = array_key_exists('sort_by', $_GET) ? $_GET['sort_by'] : 'source_name';
  $sort_dir = array_key_exists('sort_dir', $_GET) ? $_GET['sort_dir'] : 'asc';
    
  $form = drupal_get_form('tripal_network_viewer_edge_data_form', $network_session_id,
      $organism_id, $network_id, $filters, $page, $edata_includes, $sort_by, $sort_dir);
  $form = drupal_render($form);
  
  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_data_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $filters = array_key_exists('filters', $_GET) ? $_GET['filters'] : [];
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  $page = array_key_exists('page', $_GET) ? $_GET['page'] : 0;
  $ndata_includes = array_key_exists('ndata_includes', $_GET) ? $_GET['ndata_includes'] : [];
  $sort_by = array_key_exists('sort_by', $_GET) ? $_GET['sort_by'] : 'node_name';
  $sort_dir = array_key_exists('sort_dir', $_GET) ? $_GET['sort_dir'] : 'asc';
  
  $form = drupal_get_form('tripal_network_viewer_node_data_form', $network_session_id,
      $organism_id, $network_id, $filters, $page, $ndata_includes, $sort_by, $sort_dir);
  $form = drupal_render($form);
  
  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_details() {
  $node_id = array_key_exists('node_id', $_GET) ? $_GET['node_id'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_node_details_form', $node_id, $state, $network_session_id);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_network_details_form($form, &$form_state, 
    TripalNetwork $network = NULl) {
  
  
  if (!$network) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => 'Please select a network to view details.',
    ];
    return $form;
  }
  
  $selection = $network->getSelection();
  
  $organism = chado_generate_var('organism', ['organism_id' => $selection['organism_id']]);
  $form['network_details_organism'] = [
    '#type' => 'item',
    '#title' => t('Organism'),
    '#markup' => chado_get_organism_scientific_name($organism)
  ];
  
  $edges = $network->getEdges();
  $nodes = $network->getNodes();
  
  $rows = [];
  $headers = [];
  $rows[] = [
    [
      'data' => 'Number of Edges',
      'header' => TRUE,
      'width' => '70%',
    ],
    count(array_keys($edges)),
  ];
  $rows[] = [
    [
      'data' => 'Number of Nodes',
      'header' => TRUE,
      'width' => '70%',
    ],
    count(array_keys($nodes)),
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
  $form['network_details'] = [
    '#type' => 'item',
    '#title' => 'Network Stats',
    '#markup' => theme_table($summary_table)
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => '',
  ];
  
  $form['network_details_degree_dist_plot'] = [
    '#type' => 'item',
    '#title' => 'Node Degree Distribution',
    '#markup' => '<br><div id="tripal-network-degree-dist-plot"></div>',    
  ];
  
  $degplot = new TripalNetworkPlotlyDegreeDist($network);
  $degplot_data = $degplot->data();
  
  $form['network_details_degree_dist_plot_layout'] = [
    '#type' => 'hidden',
    '#attributes' => ['id' => 'tripal-network-degree-dist-plot-layout'],
    '#value' => json_encode($degplot_data['layout']),
  ];
  $form['network_details_degree_dist_plot_data'] = [
    '#type' => 'hidden',
    '#attributes' => ['id' => 'tripal-network-degree-dist-plot-data'],
    '#value' => json_encode($degplot_data['data']),
  ];
  
  return $form;
}

/**
 *
 */
function tripal_network_viewer_node_details_form($form, &$form_state, $node_id = NULL, $state = NULL, 
    $network_session_id = NULL) {
  
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
  
  if (!$node_id) {
    $form['feature_name'] = [
      '#type' => 'markup',
      '#markup' => 'Click a node to view its details',
    ];
    return $form;
  }
  
  $network = tripal_network_viewer_get_session_network($network_session_id);
   
  $node_name = '';
  $feature = tripal_network_get_nodes([$node_id])[$node_id];  
  $entity_id = chado_get_record_entity_by_table('feature', $feature->feature_id);
  $entities = tripal_load_entity('TripalEntity', [$entity_id], FALSE, []);
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

      // Skip fields with no value that are auto loaded
      $field = field_info_field($field_name);
      $field_items = field_get_items('TripalEntity', $entity, $field_name);
      $instance_settings = $instance['settings'];
      if (array_key_exists('auto_attach', $instance['settings']) and
          $instance_settings['auto_attach'] == TRUE) {
        if (tripal_field_is_empty($field, $field_items)) {
           continue;
        }
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
    
    // Add in any network attributes for the node
    $node = $network->getNode($node_id); 
    $nattrs = $network->getNodeAttrs();
    foreach ($nattrs as  $term_id => $network_ids) {
      foreach ($network_ids as $network_id => $attr_details) {
        $rows[] = [
          [
            'data' => ucfirst($attr_details['attr']->attr_id->name),
            'header' => TRUE,
            'width' => '20%',
          ],
          $node[$term_id],
        ];
      }
    }
    array_multisort($attributes);
    
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
    field_attach_load($entity->type, [$entity->id => $entity],
        FIELD_LOAD_CURRENT, ['field_id' => $field['id']]);
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



  return $form;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_edge_details() {
  $edge_id = array_key_exists('edge_id', $_GET) ? $_GET['edge_id'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  
  $form = drupal_get_form('tripal_network_viewer_edge_details_form', $edge_id, $network_session_id);
  $form = drupal_render($form);

  ajax_deliver($form);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_edge_expression() {
  $edge_id = array_key_exists('edge_id', $_GET) ? $_GET['edge_id'] : NULL;
  $z_axis = array_key_exists('z_axis', $_GET) ? $_GET['z_axis'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;
  
  $network = tripal_network_viewer_get_session_network($network_session_id);
  $expplot = new TripalNetworkPlotly3DExpression($network, $edge_id, $z_axis);
  $expplot_data = $expplot->data();
  
  drupal_json_output($expplot_data);
}

/**
 *
 */
function tripal_network_viewer_edge_details_form($form, &$form_state, $edge_id = NULL, 
    $network_session_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $edge_id = array_key_exists('edge_id', $form_state['values']) ? $form_state['values']['edge_id'] : $edge_id;
  }

  $edge_name = 'Click an edge to view its details';
  $net_name = '';
  $details = '';
  
  if ($edge_id) {
    $network = tripal_network_viewer_get_session_network($network_session_id);
    $expplot = new TripalNetworkPlotly3DExpression($network, $edge_id);
    $expplot_data = $expplot->data();
    $edge = $expplot->edge();
    $edge_name = $edge->edgeName();
    $net_name = $edge->netName();
    $attrs = $edge->attributes();
    $attr_values = $edge->attributeValues();
        
    $rows = [];
    foreach ($attrs as $term_id => $attr) { 
      $term_id = $attr['attr']->attr_id->dbxref_id->db_id->name . ":" . 
                 $attr['attr']->attr_id->dbxref_id->accession;
      
      $value = $attr_values[$term_id];
      if ($term_id == 'local:KINC_sample_string') {
        $value = '<pre>' . wordwrap($value, 30, "\n", TRUE) . '</pre>';
      }
      $rows[] = [
        [
          'data' => ucfirst($attr['attr']->attr_id->name),
          'header' => TRUE,
        ],
        $value
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

    $form['edge_id'] = [
      '#type' => 'hidden',
      '#value' => $edge_id,
      '#attributes' => ['id' => 'tripal-network-edge-id'],
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
    
    $form['network_details_expresssion_plot'] = [
      '#type' => 'item',
      '#title' => 'Edge Expression',
      '#markup' => '<br><div id="tripal-network-edge-expression-plot"></div>',
    ];
    
    $samples = $expplot->samples();
    $sample_attribute_names = $expplot->sampleAttrNames();
    $form['network_details_expresssion_plot_color_by'] = [
      '#type' => 'select',
      '#description' => 'Color the nodes in the scatterplot by the selected attribute.',
      '#options' => $sample_attribute_names,
      '#attributes' => ['id' => 'tripal-network-edge-expression-plot-color-by'],
    ];
        
    $form['network_details_expresssion_plot_layout'] = [
      '#type' => 'hidden',
      '#attributes' => ['id' => 'tripal-network-edge-expression-plot-layout'],
      '#value' => json_encode($expplot_data['layout']),
    ];
    $form['network_details_expresssion_plot_data'] = [
      '#type' => 'hidden',
      '#attributes' => ['id' => 'tripal-network-edge-expression-plot-data'],
      '#value' => json_encode($expplot_data['data']),
    ];
  }
    
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
  $filters = array_key_exists('filters', $_GET) ? $_GET['filters'] : [];
  $features = array_key_exists('features', $_GET) ? $_GET['features'] : NULL;
  $vocab_terms = array_key_exists('vocab_terms', $_GET) ? $_GET['vocab_terms'] : NULL;
  $network_session_id  = array_key_exists('network_session_id', $_GET) ? $_GET['network_session_id'] : NULL;

  if (!tripal_network_viewer_has_access($organism_id, $network_id)) {
    drupal_set_message('Permission denied', 'error');
    $organism_id = 0;
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
        LEFT JOIN {feature_synonym} FS on FS.feature_id = F.feature_id
        LEFT JOIN {synonym} S on S.synonym_id = FS.synonym_id
      WHERE
        (lower(F.name) = :name OR
         lower(F.uniquename) = :name OR
         lower(S.name) = :name) AND
        F.organism_id = :organism_id
    ";
    foreach ($features as $fname) {
      $args = [
        ':name' => strtolower(trim($fname)),
        ':organism_id' => $organism_id
      ];
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
 
  // Create the network instance
  $network = new TripalNetwork([
    'organism_id' => $organism_id, 'network_ids' => $network_id,
    'feature_ids' => $feature_ids, 'term_ids' => $term_ids    
  ]);
  
  // Set the filters
  $network->resetFilters();
  foreach ($filters as $filter) {
    $network->addFilter($filter);
  }
  
  // Load the network data
  $network->loadNetwork();  
   
  // Generate the plotly data.
  $network_plot = new TripalNetworkPlotlyNetwork($network, 'fa2');
  if ($layer_by) {
    $network_plot->layerBy($layer_by);
  }
  if ($color_by) {
    $network_plot->ColorBy($color_by);
  }
  $plotly_data = $network_plot->data();

  // Build the network details content.
  $details = drupal_get_form('tripal_network_viewer_network_details_form', $network);
  $details = drupal_render($details);
  
  $plotly_data['details'] = $details;
  $plotly_data['network_session_id'] = $network_session_id;
  
  
  // Save this network to the session variable so we don't have to look it up again.
  tripal_network_viewer_set_session_network($network, $network_session_id); 
  

  drupal_json_output($plotly_data);
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
  $organism_id_orig = $organism_id;
  if (array_key_exists('values', $form_state)) {
    $features = array_key_exists('features', $form_state['values']) ? $form_state['values']['features'] : $features;
    $vocab_terms = array_key_exists('vocab_terms', $form_state['values']) ? $form_state['values']['vocab_terms'] : $vocab_terms;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
    $organism_id_orig = $form_state['values']['organism_id_orig'];
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
  
  $form['load_network_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-network-viewer-network-form">',
  ];

  $form['organism_id_orig'] = [
    '#type' => 'value',
    '#value' => $organism_id_orig,
  ];
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

  $form['load_network_end'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_form_validate($form, &$form_state) {
  $organism_id = $form_state['values']['organism_id'];
  if (!$organism_id) {
    form_set_error('organism_id', t('Please specify an organism.'));
  }
}

/**
 *
 */
function tripal_network_viewer_network_form_ajax_callback($form, $form_state) {
  
  // If there are errors, just return the form
  if (form_get_errors()) {
    return $form;
  }
  
  $network_id = $form_state['values']['network_id'];
  $organism_id = $form_state['values']['organism_id'];
  $organism_id_orig = $form_state['values']['organism_id_orig'];
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
  
  // If the ruser changed the organizm, then reset the session.
  if ($organism_id != $organism_id_orig) {
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "initViewer", [$args]));
  }

  // Tell Drupal to update the network
  if ($form_state['triggering_element']['#name'] == 'load_network_button') {
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "closedSidebar"));
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));
  }

  return $form;
}

/**
 *
 */
function tripal_network_viewer_filter_nodes_form(&$form, &$form_state, $network_session_id,
    $organism_id = NULL, $network_id = NULL) {

}
/**
 *
 */
function tripal_network_viewer_filter_edges_cat_form(&$form, &$form_state, $network_session_id,
    $organism_id = NULL, $network_id = NULL) {
  
  $limit_by = '';
  $categories = [];
  
  // Refill the values in the form if an input was provided but the form failed to submit.
  if (array_key_exists('values', $form_state)) {
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
    $categories = array_key_exists('categories', $form_state['values']) ? $form_state['values']['categories'] : $categories;
  }
  
  $network = tripal_network_viewer_get_session_network($network_session_id);
  $edges = $network->getEdges();
  $props = tripal_network_get_edgeprops(array_keys($edges), $limit_by);
  $cat_list = [0 => '--Select--'];
  foreach ($props as $prop) {
    $cat_list[$prop] = $prop;
  }
  array_multisort($cat_list);
  $form['categories'] = [
    '#type' => 'select',
    '#title' => 'Category',
    '#multiple' => TRUE,
    '#options' => $cat_list,
    '#default_values' => $categories,
    '#description' => t('Select one or more categories to limit edges. Only categories for edges in the current network are listed.')
  ];
}
/**
 * 
 */
function tripal_network_viewer_filter_edges_quant_form(&$form, &$form_state, $network_session_id,
    $organism_id = NULL, $network_id = NULL) {
        
  $limit = '';
  $min_value = '';
  $max_value = '';
  $order_by = '';
  $use_abs = '';
  
  // Refill the values in the form if an input was provided but the form failed to submit.
  if (array_key_exists('values', $form_state)) {
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : $limit;
    $min_value = array_key_exists('min_value', $form_state['values']) ? $form_state['values']['min_value'] : $min_value;
    $max_value = array_key_exists('max_value', $form_state['values']) ? $form_state['values']['max_value'] : $max_value;
    $order_by = array_key_exists('order_by', $form_state['values']) ? $form_state['values']['order_by'] : $order_by;
    $use_abs = array_key_exists('use_abs', $form_state['values']) ? $form_state['values']['use_abs'] : $use_abs;
  }
  
  $form['limit'] = [
    '#type' => 'textfield',
    '#title' => 'Number of Edges',
    '#size' => 10,
    '#default_value' => $limit,
    '#description' => t('Limit the number of edges displayed.'),
  ];
  $form['order_by'] = [
    '#type' => 'select',
    '#title' => 'Order By',
    '#options' => [
      '' => '--Select--',
      'ASC' => 'Ascending',
      'DESC' => 'Descending'
    ],
    '#default_value' => $order_by,
    '#description' => t('Sort the edegs using the property specified above. If a limit is provided then the first set of edges up to the limit indicated will be retreived.'),
  ];
  $form['use_abs'] = [
    '#type' => 'checkbox',
    '#title' => 'Use the absolute value',
    '#default_value' => $use_abs,
    '#description' => t('Use the absolute value of the property when sorting'),
  ];
  $form['max_value'] = [
    '#type' => 'textfield',
    '#title' => 'Maximum Value',
    '#size' => 4,
    '#default_value' => $max_value,
    '#description' => t('The maximum value allowed for the property. Edges with a greater value will be excluded.'),
  ];
  $form['min_value'] = [
    '#type' => 'textfield',
    '#title' => 'Minimum Value',
    '#size' => 4,
    '#default_value' => $min_value,
    '#description' => t('The minimum value allowed for the property. Edges with a smaller value will be excluded.'),
  ]; 
}

/**
 *
 */
function tripal_network_viewer_filter_form($form, &$form_state, $network_session_id,
    $organism_id = NULL, $network_id = NULL, $filters = []) {
  
  $details = 'Please select a network to view details.';
  if (!$organism_id) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
    ];
    return $form;
  }
  
  $limit_by = '';
  
  // Refill the values in the form if an input was provided but the form failed to submit.
  if (array_key_exists('values', $form_state)) {
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
  }
  
  // Refill the values in the form if an input was provided but the form failed to submit.
  if (array_key_exists('values', $form_state)) {
    $filters = array_key_exists('filters', $form_state['values']) ? $form_state['values']['filters'] : $filters;
  }
  $form['filters_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-network-viewer-filter-form">'    
  ];
  
  $edge_filters_collapsed = TRUE;
  if ($limit_by) {
    $edge_filters_collapsed = FALSE;
  }
  $form['edge_filters'] = [
    '#type' => 'fieldset',
    '#title' => 'Edge Filters',
    '#collapsible' => TRUE,
    '#collapsed' => $edge_filters_collapsed,
    '#description' => t('Use the following fields to limit which edges are included in the network.')
  ];
  $quant_props = [];
  if ($organism_id) {
    $quant_props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, FALSE, TRUE, FALSE);
  }
  $cat_props = [];
  if ($organism_id) {
    $cat_props = tripal_network_get_attribute_options($organism_id, $network_id, FALSE, TRUE, TRUE, FALSE);
  }
  $props = $quant_props + $cat_props;
  array_multisort($props);
  
  $form['edge_filters']['limit_by'] = [
    '#type' => 'select',
    '#title' => 'Property',
    '#options' => $props,
    '#default_value' => $limit_by,
    '#description' => t('Select a property to limit the best edges (only works for numerical properties). Only properties for the current edges are listed.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-filter-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];
    
  if (in_array($limit_by, array_keys($quant_props), TRUE)) {
    tripal_network_viewer_filter_edges_quant_form($form['edge_filters'], $form_state, $network_session_id, $organism_id, $network_id);
  }
  if (in_array($limit_by, array_keys($cat_props), TRUE)) {
    tripal_network_viewer_filter_edges_cat_form($form['edge_filters'], $form_state, $network_session_id, $organism_id, $network_id);
  }
  $form['edge_filters']['network_add_edge_filter_button'] = [
    '#type' => 'button',
    '#name' => 'network_add_edge_filter_button',
    '#value' => 'Add Edge Filter',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-filter-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];
  
  $form['node_filters'] = [
    '#type' => 'fieldset',
    '#title' => 'Node Filters',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Use the following fields to limit which nodes are included in the network.')
  ];
  tripal_network_viewer_filter_nodes_form($form['node_filters'], $form_state, $network_session_id, $organism_id, $network_id);  
  
  // Add buttons for the filter list.
  $form['filters_list'] = [
    '#theme' => 'tripal_network_viewer_filter_form_list'
  ];  
  $form['filters_list']['filters'] = [
    '#type' => 'value',
    '#value' => $filters,
  ];
  foreach ($filters as $fi => $filter) {
    $form['filters_list']["filter_${fi}_remove_button"] = [
      '#type' => 'button',
      '#name' => "filter_${fi}_remove_button",
      '#value' => 'Del',
      '#ajax' => array(
        'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
        'wrapper' => 'tripal-network-viewer-filter-form',
        'effect' => 'fade',
        'method' => 'replace',
      ),
    ];
  }

  $form['network_filter_button'] = [
    '#type' => 'button',
    '#name' => 'network_filter_button',
    '#value' => 'Apply Filters',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-filter-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];   
  
  $form['filters_wrapper_end'] = [
    '#type' => 'markup',
    '#markup' => '</div>'
  ];


  return $form;
}
/**
 *
 */
function theme_tripal_network_viewer_filter_form_list(&$variables) {
  $element = $variables['element'];
  $filters = $element['filters']['#value'];
  //print_r($element);
  
  $rows = [];
  $headers = ['Limit by', 'Constraint', ''];
  foreach ($filters as $fi => $filter) {
    $term_name = '';
    if (array_key_exists('limit_by', $filter) and $filter['limit_by']) {
      $limit_by_term = chado_get_cvterm(['id' => $filter['limit_by']]);
      $term_name = $limit_by_term->name;
    }
    $order_by = '';
    if (array_key_exists('order_by', $filter) and $filter['order_by']) {
      if ($term_name) {
        $order_by .= '<br>';
      }
      $order_by .=  'Order: ' . $filter['order_by'];
      if (array_key_exists('use_abs', $filter) and $filter['use_abs'] == 1) {
        $order_by .= ' (absolute value)';
      }
    }
    $constraint = '';
    if (array_key_exists('limit', $filter) and $filter['limit'] > 0) {
      $constraint .= 'Num Edges: ' . $filter['limit'];
    }
    if (array_key_exists('categories', $filter) and !empty($filter['categories'])) {
      $constraint .= $constraint ? '<br>' : '';
      $constraint .= 'Categories: ' . implode(',<br>', $filter['categories']);
    }
    if (array_key_exists('min_value', $filter) and $filter['min_value']) {
      $constraint .= $constraint ? '<br>' : '';
      $constraint .= 'Min Value: ' . $filter['min_value'];
    }
    if (array_key_exists('max_value', $filter) and $filter['max_value']) {
      $constraint .= $constraint ? '<br>' : '';
      $constraint .= 'Max Value: ' . $filter['max_value'];
    }
    $rows[] = [
      $term_name . $order_by,
      $constraint,      
      drupal_render($element["filter_${fi}_remove_button"]),
    ];
  }
  $filters_table = [
    '#type' => 'item',
    '#title' => t('Applied Filters'),
    '#markup' => theme_table([
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [],
      'caption' => '',
      'sticky' => TRUE,
      'colgroups' => [],
      'empty' => 'There are no current filters'
     ])
  ];
  return drupal_render($filters_table);
}
/**
 *
 */
function tripal_network_viewer_filter_form_validate($form, &$form_state) {
  $limit = '';
  $limit_by = '';
  $min_value = '';
  $max_value = '';
  $order_by = '';
  $use_abs = '';
  $categories = [];
  $filters = [];
  $clear_fields = FALSE;
  
  if (array_key_exists('values', $form_state)) {
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : $limit;
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
    $min_value = array_key_exists('min_value', $form_state['values']) ? $form_state['values']['min_value'] : $min_value;
    $max_value = array_key_exists('max_value', $form_state['values']) ? $form_state['values']['max_value'] : $max_value;
    $order_by = array_key_exists('order_by', $form_state['values']) ? $form_state['values']['order_by'] : $order_by;
    $use_abs = array_key_exists('use_abs', $form_state['values']) ? $form_state['values']['use_abs'] : $use_abs;
    $filters = array_key_exists('filters', $form_state['values']) ? $form_state['values']['filters'] : $filters;
    $categories = array_key_exists('categories', $form_state['values']) ? $form_state['values']['categories'] : $categories;    
  }
  
  if (($max_value or $min_value) and !$limit_by) {
    form_set_error('limit_by', t('Please specify a value to limit by if a min or max value is specified.'));
  }

  // If the user wants to remove a filter.
  $matches = [];
  if (preg_match('/filter_(\d+)_remove_button/', $form_state['triggering_element']['#name'], $matches)) {
    $form_state['rebuild'] = TRUE;
    unset($form_state['values']['filters'][$matches[1]]);
  }
  
  // If a new filter was specified then add it to the list.
  if ('network_add_edge_filter_button' == $form_state['triggering_element']['#name']) {    
    $form_state['values']['filters'][] = [
      'limit' => $limit,
      'limit_by' => $limit_by,
      'min_value' => $min_value,
      'max_value' => $max_value,
      'order_by' => $order_by,
      'use_abs' => $use_abs,
      'categories' => $categories,
    ];
    $clear_fields = TRUE;
   
  }
  
  // Clear out any fields if the apply filters button is clicked.
  if ('network_filter_button' == $form_state['triggering_element']['#name']) {
    $clear_fields = TRUE;
  }  
  
  // Now that we've added the filter, remove the values for the fields.
  if ($clear_fields) {    
    $form_state['values']['limit'] = '';
    $form_state['values']['limit_by'] = '';
    $form_state['values']['min_value'] = '';
    $form_state['values']['max_value'] = '';
    $form_state['values']['order_by'] = '';
    $form_state['values']['categories'] = [];
    $form_state['values']['use_abs'] = FALSE;
    
    $form_state['input']['limit'] = '';
    $form_state['input']['limit_by'] = '';
    $form_state['input']['min_value'] = '';
    $form_state['input']['max_value'] = '';
    $form_state['input']['order_by'] = '';
    $form_state['input']['categories'] = [];
    $form_state['input']['use_abs'] = FALSE;
    $form_state['rebuild'] = TRUE;
  }
}

/**
 *
 */
function tripal_network_viewer_layers_form($form, &$form_state,
    $network_session_id, $organism_id = NULl, $network_id = NULL, $layer_by = NULL,
    $color_by = NULL) {

  $details = 'Please select a network to view details.';
  if (!$organism_id) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
    ];
    return $form;
  } 
  if (array_key_exists('values', $form_state)) {
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : $color_by;
  }
  $props = [];
  if ($organism_id) {
    $props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, TRUE, TRUE, FALSE);
  }
  array_multisort($props);
  
  $form['layers_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-network-viewer-layers-form">'
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
  ];
  
  $form['layers_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '</div">'
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_node_details_form_ajax_callback($form, $form_state){

  // If there are errors, just return the form
  if (form_get_errors()) {
    return $form;
  }
  
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
function tripal_network_viewer_filter_form_ajax_callback($form, &$form_state) {
  
  // If there are errors, just return the form
  if (form_get_errors()) {
    return $form;
  }
  
  $limit = '';
  $limit_by = '';
  $min_value = '';
  $max_value = '';
  $order_by = '';
  $use_abs = '';
  $filters = [];
  $categories = [];
  if (array_key_exists('values', $form_state)) {
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : $limit;
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
    $min_value = array_key_exists('min_value', $form_state['values']) ? $form_state['values']['min_value'] : $min_value;
    $max_value = array_key_exists('max_value', $form_state['values']) ? $form_state['values']['max_value'] : $max_value;
    $order_by = array_key_exists('order_by', $form_state['values']) ? $form_state['values']['order_by'] : $order_by;
    $use_abs = array_key_exists('use_abs', $form_state['values']) ? $form_state['values']['use_abs'] : $use_abs;
    $categories = array_key_exists('categories', $form_state['values']) ? $form_state['values']['categories'] : $categories;
    $filters = array_key_exists('filters', $form_state['values']) ? $form_state['values']['filters'] : $filters;
  }  
  
  // Tell Drupal to update the network if the apply button was pressed.
  if ($form_state['triggering_element']['#name'] == 'network_filter_button') {
    $args = [
      'filters' => $filters,
    ];
    tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));
  }

  return $form;
}
/**
 *
 */
function tripal_network_viewer_layers_form_ajax_callback($form, $form_state) {

  // If there are errors, just return the form
  if (form_get_errors()) {
    return $form;
  }
  
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

/**
 * 
 */
function tripal_network_viewer_analysis_form($form, $form_state,  $network_session_id,
    $organism_id = NULL, $network_id = NULL) {
  
      
  $network = tripal_network_viewer_get_session_network($network_session_id);
  
  if (!$network) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => t('Please select a network to view details.'),
    ];
    return $form;
  }
  
  // Get the included columns.
  $pvalue_threshold = 0.001;
  $analysis_type = 0;
  if (array_key_exists('values', $form_state)) {
    $analysis_type = array_key_exists('analysis_type', $form_state['values']) ? $form_state['values']['analysis_type'] : $analysis_type;
    $pvalue_threshold = array_key_exists('pvalue_threshold', $form_state['values']) ? $form_state['values']['pvalue_threshold'] : $pvalue_threshold;
  }
  
  $form['analysis_tab_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id ="tripal-network-viewer-analysis-form">'
  ];
  
  $analysis_types = [
    0 => '--Select--',
    'funce' => 'Functional Enrichment',
  ];
  $form['analysis_type'] = [
    '#type' => 'select',
    '#title' => 'Analysis to Perform',
    '#options' => $analysis_types,
    '#default_value' => $analysis_type,
    '#attributes' => ['class' => ['tripal-network-analysis-select']],    
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_analysis_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-analysis-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];
  
  
  if ($analysis_type == 'funce') {
    $form['pvalue_threshold'] = [
      '#type' => 'textfield',
      '#title' => 'p-value Threshold',
      '#default_value' => $pvalue_threshold,
      '#description' => 'Enter the p-value threshold (e.g. 0.001)'
    ]; 
    
    $form['funce_submit'] = [
      '#type' => 'submit',
      '#value' => 'Run Analysis',
      '#name' => 'funce_submit_button',
      '#ajax' => array(
        'callback' => 'tripal_network_viewer_analysis_form_ajax_callback',
        'wrapper' => 'tripal-network-viewer-analysis-form',
        'effect' => 'fade',
        'method' => 'replace',
      ),
    ];
  }
  
  $form['analysis_tab_wrapper_end'] = [
    '#type' => 'markup',
    '#markup' => '</div>'
  ];
  
  return $form;
      
}

function tripal_network_viewer_analysis_form_ajax_callback($form, $form_state) {
  
  // If there are errors, just return the form
  if (form_get_errors()) {
    return $form;
  }
    
  return $form;
  
}

/**
 *
 **/
function tripal_network_viewer_edge_data_form($form, $form_state,  $network_session_id,
    $organism_id = NULL, $network_id = NULL, $filters = [], $page = 0, $edata_includes = [],
    $sort_by = 'source_name', $sort_dir = 'asc') {

  $network = tripal_network_viewer_get_session_network($network_session_id);
  $details = 'Please select a network to view details.';
  if (!$network) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
    ];
    return $form;
  }
         
  // Get the included columns.
  if (array_key_exists('values', $form_state)) {
    $edata_includes = array_key_exists('edata_includes', $form_state['values']) ? $form_state['values']['edata_includes'] : $edata_includes;
  }
  
  $form['edge_tab_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id ="tripal-network-viewer-edge-data-form">'
  ];
  
  $attributes = [];
  $eattrs = $network->getEdgeAttrs();
  foreach ($eattrs as  $term_id => $networks) {
    foreach ($networks as $attr_details) {
      $attributes[$term_id] = ucfirst($attr_details['attr']->attr_id->name);
    }
  }
  array_multisort($attributes);
  
  $form['edata_includes'] = [
    '#type' => 'select',
    '#title' => 'Columns to Include',
    '#options' => $attributes,
    '#multiple' => TRUE,
    '#size' => 10,
    '#default_value' => $edata_includes,
    '#description' => t('Select Properties to include in the table.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_edge_data_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-edge-data-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];
  
  // Loade the included properties
  foreach ($edata_includes as $term_id) {
    $network->retrieveEdgeProp($term_id);
  }
  
  // Determine how to order the nodes, then flip it
  // so that the table headers have the opposite direction.
  $header_sort_dir = $sort_dir;
  if ($sort_dir == 'desc') {
    $network->sortEdges($sort_by, SORT_DESC);
    $header_sort_dir = 'asc';
  }
  else if ($sort_dir == 'asc') {
    $network->sortEdges($sort_by, SORT_ASC);
    $header_sort_dir = 'desc';
  }  
  
  $edges = $network->getEdges();
  $edge_indexes = array_keys($edges);
  
  // Initialize the pager
  $per_page = 15;
  pager_default_initialize(count($edge_indexes), $per_page);
  $pager = theme('pager', ['quantity', count($edge_indexes)]);
  
  // Fix the pager so it works with ajax
  $pager = preg_replace('/href="\/.+\?page=(\d+).*"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateEdgeDataForm($1, JSON.parse(JSON.stringify('. json_encode($edata_includes) . ')))\'', $pager);
  $pager = preg_replace('/href="\/.+"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateEdgeDataForm(0, JSON.parse(JSON.stringify(' . json_encode($edata_includes) . ')))\'', $pager);
  
  // Build the headers array.
  $headers = [
    '<a href="header:source_name">Source</a>', 
    '<a href="header:target_name">Target</a>',    
  ];
  foreach ($edata_includes as $term_id) {
    $headers[] = '<a href="header:' . $term_id . '">' . $attributes[$term_id] . '</a>';
  }
  
  // Build the rows of the table.
  $rows = [];
  for ($i = $page * $per_page; $i < min(($page+1) * $per_page, count($edge_indexes)); $i++) {
    $row = [
      '<a href="node-select:' . $edges[$edge_indexes[$i]]['source_node_id'] . '">' . $edges[$edge_indexes[$i]]['source_name'] . '</a>',
      '<a href="node-select:' . $edges[$edge_indexes[$i]]['target_node_id'] . '">' . $edges[$edge_indexes[$i]]['target_name'] . '</a>'
    ];
    foreach ($edata_includes as $term_id) {
      $row[] = $edges[$edge_indexes[$i]][$term_id];
    }
    $rows[] = $row;
  }
  
  // Create the table.
  $data_table = [
    'header' => $headers,
    'rows' => $rows,
    'attributes' => ['id' => 'tripal-network-data-edge-table'],
    'caption' => '',
    'sticky' => TRUE,
    'colgroups' => [],
    'empty' => 'No edges',
  ];
  $data_table  = theme_table($data_table);
  
  // Fix the links in the table to work with ajax
  $data_table = preg_replace('/href="header:(.+?)"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateEdgeDataForm('. $page . ', JSON.parse(JSON.stringify('. json_encode($edata_includes) . ')), "$1", "' . $header_sort_dir . '")\'', $data_table);
  $data_table = preg_replace('/href="node-select:(.+?)"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.selectNodeByID("$1")\'', $data_table);
  
  
  $form['data_table'] = [
    '#type' => 'item',
    '#title' => 'Edges',
    '#markup' =>  $data_table . $pager
  ];  
  
  $form['edge_tab_wrapper_end'] = [
    '#type' => 'markup',
    '#markup' => '</div>'
  ];
  
  return $form;
}

/**
 * 
 */
function tripal_network_viewer_node_data_form($form, $form_state,  $network_session_id,
    $organism_id = NULL, $network_id = NULL, $filters = [], $page = 0, $ndata_includes = [], 
    $sort_by = 'node_name', $sort_dir = 'asc') {
      
  $network = tripal_network_viewer_get_session_network($network_session_id);
  $details = 'Please select a network to view details.';
  if (!$network) {
    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
    ];
    return $form;
  }
      
  // Get the included columns.
  if (array_key_exists('values', $form_state)) {
    $ndata_includes = array_key_exists('ndata_includes', $form_state['values']) ? $form_state['values']['ndata_includes'] : $ndata_includes;
  }
  
  $form['node_tab_wrapper_start'] = [
    '#type' => 'markup',
    '#markup' => '<div id ="tripal-network-viewer-node-data-form">'
  ];
      
  $attributes = [];
  $nattrs = $network->getNodeAttrs();
  foreach ($nattrs as  $term_id => $network_ids) {
    foreach ($network_ids as $network_id => $attr_details) {
      $attributes[$term_id] = ucfirst($attr_details['attr']->attr_id->name);
    }
  }
  array_multisort($attributes);
  
  $form['ndata_includes'] = [
    '#type' => 'select',
    '#title' => 'Columns to Include',
    '#options' => $attributes,
    '#multiple' => TRUE,
    '#default_value' => $ndata_includes,
    '#description' => t('Select Properties to include in the table.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_node_data_form_ajax_callback',
      'wrapper' => 'tripal-network-viewer-node-data-form',
      'effect' => 'fade',
      'method' => 'replace',
    ),
  ];
  
  // Load the included properties
  foreach ($ndata_includes as $term_id) {
    $network->retrieveNodeProp($term_id);
  }
  
  // Determine how to order the nodes, then flip it 
  // so that the table headers have the opposite direction.
  $header_sort_dir = $sort_dir;
  if ($sort_dir == 'desc') {
    $network->sortNodes($sort_by, SORT_DESC);
    $header_sort_dir = 'asc';
  } 
  else if ($sort_dir == 'asc') {
    $network->sortNodes($sort_by, SORT_ASC);
    $header_sort_dir = 'desc';
  }  
  
  $nodes = $network->getNodes();  
  $node_indexes = array_keys($nodes);
  
  // Initialize the pager
  $per_page = 15;
  pager_default_initialize(count($node_indexes), $per_page);
  $pager = theme('pager', ['quantity', count($node_indexes)]);
  
  // Fix the pager so it works with ajax
  $pager = preg_replace('/href="\/.+\?page=(\d+).*"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateNodeDataForm($1, JSON.parse(JSON.stringify('. json_encode($ndata_includes) . ')), "' . $sort_by . '", "' . $sort_dir . '")\'', $pager);
  $pager = preg_replace('/href="\/.+"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateNodeDataForm(0, JSON.parse(JSON.stringify(' . json_encode($ndata_includes) . ')), "' . $sort_by . '", "' . $sort_dir . '")\'', $pager);
  
  // Build the headers array.
  $headers = ['<a href="header:node_name">Node Name</a>'];
  foreach ($ndata_includes as $term_id) {
    $headers[] = '<a href="header:' . $term_id . '">' . $attributes[$term_id] . '</a>';
  }
    
  // Build the rows of the table.
  $rows = [];
  for ($i = $page * $per_page; $i < min(($page+1) * $per_page, count($node_indexes)); $i++) {
    $row = [
      '<a href="node-select:' . $node_indexes[$i] . '">' . $nodes[$node_indexes[$i]]['node_name'] . '</a>'
    ];
    foreach ($ndata_includes as $term_id) {
      $row[] = $nodes[$node_indexes[$i]][$term_id] ;
    }
    $rows[] = $row;
  }
  
  // Create the table.
  $data_table = [
    'header' => $headers,
    'rows' => $rows,
    'attributes' => ['id' => 'tripal-network-data-node-table'],
    'caption' => '',
    'sticky' => TRUE,
    'colgroups' => [],
    'empty' => 'No nodes',
  ];
  
  $data_table = theme_table($data_table);
  
  // Fix the links in the table to work with ajax
  $data_table = preg_replace('/href="header:(.+?)"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.updateNodeDataForm('. $page . ', JSON.parse(JSON.stringify('. json_encode($ndata_includes) . ')), "$1", "' . $header_sort_dir . '")\'', $data_table);
  $data_table = preg_replace('/href="node-select:(.+?)"/', 'href="javascript:void(0)" onclick=\'jQuery.fn.selectNodeByID("$1")\'', $data_table);
  
  $form['data_table'] = [
    '#type' => 'item',
    '#title' => 'Nodes',
    '#markup' =>  $data_table . $pager
  ];  
  
  $form['node_tab_wrapper_end'] = [
    '#type' => 'markup',
    '#markup' => '</div>'
  ];
  
  return $form;
}

function tripal_network_viewer_set_session_network($network, $network_session_id){
  
  // Save this network to the session variable so we don't have to look it up again.
  $_SESSION['tripal_network_current'][$network_session_id] = [
    'network' => serialize($network),
  ];
  return $network_session_id;
}
/**
 * 
 */
function tripal_network_viewer_get_session_network($network_session_id) {
  if (!$network_session_id) {
    return NULL;
  }
  if (array_key_exists('tripal_network_current', $_SESSION)) {
    if (array_key_exists($network_session_id, $_SESSION['tripal_network_current'])){
      return unserialize($_SESSION['tripal_network_current'][$network_session_id]['network']);
    }
  }
  return NULL;
}

/**
 *
 **/
function tripal_network_viewer_edge_data_form_ajax_callback($form, $form_state) {
    
  return $form;
}

function tripal_network_viewer_node_data_form_ajax_callback($form, $form_state) {
  
  return $form;
}

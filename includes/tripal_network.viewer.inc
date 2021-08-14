<?php

/**
 *
 */
function tripal_network_viewer_page($network_id = NULL) {
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'drupal.form');
  return theme('tripal_network_viewer', ['network_id' => $network_id]);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_display_form() {
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_display_form', $network_id, $layer_by, $color_by);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_network_details_form() {
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_network_details_form', $network_id);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_details() {
  $node_id = array_key_exists('node_id', $_GET) ? $_GET['node_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_node_details_form', $node_id);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

function tripal_network_viewer_node_details_form($form, &$form_state, $node_id = NULL) {

  $default_field = 'summary';
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
    $entity_id = chado_get_record_entity_by_table('feature', $node_id);
    $entities = tripal_load_entity('TripalEntity', [$entity_id]);
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
          continue;
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
      $node_name = '<h3>Node: ' . $entity->title . '</h3>';
    }
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


  $form['feature_id'] = [
    '#type' => 'value',
    '#value' => $node_id,
    '#prefix' => '<div id="tripal-network-viewer-node-details-form">',
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

  $form['feature_name'] = [
    '#type' => 'markup',
    '#markup' => $node_name,
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
  $node_id = array_key_exists('edge_id', $_GET) ? $_GET['edge_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_edge_details_form', $node_id);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_edge_details_form($form, &$form_state, $edge_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $edge_id = array_key_exists('edge_id', $form_state['values']) ? $form_state['values']['edge_id'] : $edge_id;
  }

  $edge_name = 'Click an edge to view its details';
  $details = '';
  if ($edge_id) {
    $sql = "
      SELECT FS.name as source, FT.name as target
      FROM {network_edge} NE
        INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
        INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
        INNER JOIN {feature} FS on FS.feature_id = NNS.feature_id
        INNER JOIN {feature} FT on FT.feature_id = NNT.feature_id
     WHERE NE.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);
    $edge = $results->fetchObject();
    $edge_name = '<h3>Edge: ' . $edge->source . ' (-) ' . $edge->target . '</h3>';

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
    '#type' => 'markup',
    '#markup' => $edge_name,
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
    '#prefix' => '<div id="tripal-network-viewer-edge-details-details">',
    '#suffix' => '</div>',
  ];
  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_details_form($form, &$form_state, $network_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
  }

  $network_name = 'Select a network to view its details';
  $details = '';

  if ($network_id) {
    $network_name = 'Network Attributes';
    $sql = "
      SELECT CVT.name, NP.value, DBX.accession, DB.name as db_name
      FROM {networkprop} NP
        INNER JOIN {cvterm} CVT ON CVT.cvterm_id = NP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE NP.network_id = :network_id
    ";
    $args[':network_id'] = $network_id;
    $results = chado_query($sql, $args);

    $rows = [];
    while ($prop = $results->fetchObject()) {
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

  $form['network_id'] = [
    '#type' => 'value',
    '#value' => $network_id,
  ];
  $form['network_name'] = [
    '#type' => 'markup',
    '#markup' => $network_name,
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
    '#prefix' => '<div id="tripal-network-viewer-network-details-details">',
    '#suffix' => '</div>',
  ];
  return $form;
}

function tripal_network_viewer_create_edge_trace($name) {
  return [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => [],
      'width' => 2,
      'opacity' => 0.9,
    ],
    'name' => $name,
    'ids' => [],
    'hovertemplate'=> "%{text}",
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];
}
/**
 *
 */
function tripal_network_viewer_ajax_retrieve() {
  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;

  // Set the Plotly layout sepcifications.
  $layout = [
    'autosize' => TRUE,
    'paper_bgcolor' => "#FFFFFF",
    'showlegend' => TRUE,
    'legend' => [
      'font' => [
        'color' => "#000000",
      ],
    ],
    'scene' => [
      'aspectmode' => 'manual',
      'aspectratio' => [
        'x' => 1,
        'y' => 1,
        'z' => 1
      ],
      'camera' => [
        'eye' => [
          'x' => 0,
          'y' => 0,
          'z' => 2
        ],

        'projection' => [
          'type' => 'orthographic'
        ]
      ],
      'xaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showbackground' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#000000"
      ],
      'yaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#000000"
      ],
      'zaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => TRUE,
        'showspikes' => FALSE,
        'title' => '',
        'color'=> "#000000"
      ]
    ],
    'title' => '',
  ];

  if (!$layer_by) {
    $layer_by = 'SWO:0000425';
  }
  if (!$color_by) {
    $color_by = 'SWO:0000425';
  }

  // Don't show a network that the user doesn't have access to or isn't
  // published.
  $entity_id = chado_get_record_entity_by_table('network', $network_id);
  if (!$entity_id){
    $network_id = 0;
  }
  $entity = tripal_load_entity('TripalEntity', [$entity_id]);
  if (!tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
    $network_id = 0;
  }

  // Make sure we have required filters.
  $plotly_data = NULL;
  if ($network_id == 0) {
    $plotly_data = tripal_network_make_plotly_data(NULL, $layer_by, $color_by, TRUE);
  }
  else {
    $network = tripal_network_get_network_elements($network_id);
    $plotly_data = tripal_network_make_plotly_data($network, $layer_by, $color_by);
  }

  drupal_json_output([
    'data' => [
      $plotly_data['edges_trace'],
      $plotly_data['nodes_trace']
    ],
    'layout' => $layout]
  );
}


/**
 *
 */
function tripal_network_viewer_network_form($form, &$form_state, $network_id = NULL) {
  global $user;

  // Get the list of modules
  $sql = '';
  $args = [];
  $sql = "
    SELECT * FROM {network} N
      INNER JOIN {organism} O on O.organism_id = N.organism_id
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
        $networks[$record->genus . ' ' . $record->species][$record->network_id] = $record->name;
      }
    }
  }

  $form['network_id'] = [
    '#type' => 'select',
    '#title' => 'Network',
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('Select the network to view connections for this feature.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_network_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-network-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#prefix' => '<div id="tripal-network-viewer-network-form">',
    '#suffix' => '</div>',
  ];
  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];

  $args = [
    'network_id' => $network_id,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));

  return $form;
}

/**
 *
 */
function tripal_network_viewer_display_form($form, &$form_state,
    $network_id = NULL, $layer_by = NULL, $color_by = NULL) {

  if (array_key_exists('values', $form_state)) {
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : $color_by;
  }
  if (!$layer_by) {
    $layer_by = 'SWO:0000425';
  }
  if (!$color_by) {
    $color_by = 'SWO:0000425';
  }
  $props = [];
  if ($network_id) {

    $sql = "
      SELECT DISTINCT CVT.cvterm_id, CVT.name, DBX.accession, DB.name as db_name
      FROM {network_edgeprop} NEP
        INNER JOIN {network_edge} NE on NE.network_edge_id = NEP.network_edge_id
        INNER JOIN {cvterm} CVT on CVT.cvterm_id = NEP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE NE.network_id = :network_id
      ORDER BY CVT.name
    ";
    $args = [':network_id' => $network_id];
    $results = chado_query($sql, $args);
    while ($record = $results->fetchObject()) {
      $term = $record->db_name . ':' . $record->accession;
      $props[$term] = ucfirst($record->name);
    }
  }

  $form['network_id'] = [
    '#type' => 'value',
    '#value' =>  $network_id,
    '#prefix' => '<div id="tripal-network-viewer-display-form">',
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
  $form['button'] = [
    '#type' => 'submit',
    '#value' => 'Update Display',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_display_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-display-form',
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
  return $form;
}
/**
 *
 */
function tripal_network_viewer_display_form_ajax_callback($form, $form_state) {

  $network_id = NULL;
  $layer_by = NULL;
  $color_by = NULL;
  if (array_key_exists('values', $form_state)) {
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : NULL;
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : NULL;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : NULL;
  }

  $args = [
    'network_id' => $network_id,
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
function tripal_network_neighborhood_viewer_ajax_retrieve() {
  // Get the fitering variables requested by the user.
  $network_id = $_GET['network_id'];
  $feature_id = $_GET['feature_id'];

  // Set the Plotly layout sepcifications.
  $layout = [
    'autosize' => TRUE,
    'height' => 500,
    'width' => 500,
    'paper_bgcolor' => "#FFFFFF",
    'showlegend' => TRUE,
    'scene' => [
      'aspectmode' => 'manual',
      'aspectratio' => [
        'x' => 1,
        'y' => 1,
        'z' => 1
      ],
      'camera' => [
        'eye' => [
          'x' => 0,
          'y' => 0,
          'z' => 2
        ],

        'projection' => [
          'type' => 'orthographic'
        ]
      ],
      'xaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showbackground' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE
      ],
      'yaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE
      ],
      'zaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => TRUE,
        'showspikes' => FALSE,
        'title' => 'Similarity Score'
      ]
    ],
    'title' => 'Connected Neighbors',
  ];

  $layer_by = 'SWO:0000425';
  $color_by = 'SWO:0000425';

  // Make sure we have required filters.
  $plotly_data = NULL;
  if ($network_id == 0) {
    $plotly_data = tripal_network_make_plotly_data(NULL, TRUE);
  }
  else {
    $network = tripal_network_get_network_elements($network_id, $feature_id);
    $plotly_data = tripal_network_make_plotly_data($network, $layer_by, $color_by);
  }

  drupal_json_output([
    'data' => [
      $plotly_data['edges_trace'],
      $plotly_data['nodes_trace']
    ],
    'layout' => $layout]
      );
}

/**
 *
 */
function tripal_network_make_plotly_data($network, $layer_by, $color_by, $is_empty = FALSE) {

  $edges_trace = [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => [],
      'width' => 2,
      'opacity' => 0.9,
    ],
    'name' => 'Edge',
    'ids' => [],
    'hovertemplate'=> "%{text}",
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];

  $nodes_trace = [
    'mode'  => 'markers',
    'type' => 'scatter3d',
    'marker' => [
      'symbol' => 'circle',
      'color' => [],
      'size' => [],
      'sizemin' => 5,
      'opacity' => 0.9,
      'line' => [
        'width' => 2,
        'color' => 'rgb(100, 100, 100)',
      ],
    ],
    'hovertemplate'=> "%{text}",
    'name' => 'Node',
    'ids' => [],
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];

  if ($is_empty) {
    return [
      'edges_trace' => $edges_trace,
      'nodes_trace' => $nodes_trace
    ];
  }

  $edges = $network['edges'];
  $nodes = $network['nodes'];

  // Get properties of the nodes used for the viewer.
  $x_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network['network_id'], 'SIO:000400');
  $y_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network['network_id'], 'SIO:000401');
  $degree = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network['network_id'], 'OBI:0200130');

  // Calculate teh x,y,z coordianates for nodes.
  $node_coords = [];
  foreach ($nodes as $node_id => $node_name) {
    $x = array_key_exists($node_id, $x_coords) ? $x_coords[$node_id] : mt_rand(0, 10);
    $y = array_key_exists($node_id, $y_coords) ? $y_coords[$node_id] : mt_rand(0, 10);
    $node_coords[$node_id] = [
      'x' => $x,
      'y' => $y,
      'z' => 0
    ];
  }

  // Get the layer bins for the z-coordinate and the color
  $layers = tripal_network_layer_network_edges(array_keys($edges), $layer_by, $color_by);

  // Create the edges trace.
  foreach ($edges as $network_edge_id => $edge) {

    // Get the z-coord.
    $z = array_key_exists($network_edge_id, $layers['z']['bins']) ? $layers['z']['bins'][$network_edge_id] : 0;

    $source_feature_id = $edge['source_feature_id'];
    $target_feature_id = $edge['target_feature_id'];

    // Set the line x,y,z coordinates
    $edges_trace['x'][] = $node_coords[$source_feature_id]['x'];
    $edges_trace['y'][] = $node_coords[$source_feature_id]['y'];
    $edges_trace['z'][] = $z;
    $edges_trace['x'][] = $node_coords[$target_feature_id]['x'];
    $edges_trace['y'][] = $node_coords[$target_feature_id]['y'];
    $edges_trace['z'][] = $z;

    // Add an empty entry to keep Plotly from adding an extra line.
    $edges_trace['x'][] = NULL;
    $edges_trace['y'][] = NULL;
    $edges_trace['z'][] = NULL;

    // Set the line text.
    $text = $edge['source_name'] . ' (-) ' . $edge['target_name'];
    $edges_trace['text'][] = $text;
    $edges_trace['text'][] = $text;
    $edges_trace['text'][] = $text;

    // Set the line color.
    $color = array_key_exists($network_edge_id, $layers['colors']['bins']) ? $layers['colors']['bins'][$network_edge_id] : 0;
    $edges_trace['line']['color'][] = $color;
    $edges_trace['line']['color'][] = $color;
    $edges_trace['line']['color'][] = $color;

    // Set the line ids.
    $edges_trace['ids'][] = $network_edge_id;
    $edges_trace['ids'][] = $network_edge_id;
    $edges_trace['ids'][] = $network_edge_id;

    // Move the node's z-coordinate to the highest value score.
    if ($layers['z']['is_quantitative'] === TRUE) {
      $node_coords[$source_feature_id]['z'] = max($z, $node_coords[$source_feature_id]['z']);
      $node_coords[$target_feature_id]['z'] = max($z, $node_coords[$target_feature_id]['z']);

      // If the layer by is "rank" then put nodes at the lowest rank they
      // first appear. Otherwise, use the highest.
      if ($layer_by == 'NCIT:C48904' and $node_coords[$source_feature_id]['z'] != 0) {
        $node_coords[$source_feature_id]['z'] = min($z, $node_coords[$source_feature_id]['z']);
        $node_coords[$target_feature_id]['z'] = min($z, $node_coords[$target_feature_id]['z']);
      }
    }
  }

  // Create the node trace
  foreach ($nodes as $node_id => $node) {
    $nodes_trace['x'][] = $node_coords[$node_id]['x'];
    $nodes_trace['y'][] = $node_coords[$node_id]['y'];
    $nodes_trace['z'][] = $node_coords[$node_id]['z'];
    $nodes_trace['ids'][] = $node_id;
    $nodes_trace['text'][] = $node['node_name'];
    $nodes_trace['marker']['size'][] = max(round(log10($degree[$node_id])*10), 5);
    $nodes_trace['marker']['color'][] = $node_id == $network['feature_id'] ? '#FF0000': '#AAAAAA';
  }

  return [
    'edges_trace' => $edges_trace,
    'nodes_trace' => $nodes_trace
  ];
}


function tripal_network_neighborhood_viewer_form($form, &$form_state,
    $feature_id = NULL, $network_id = NULL) {

  global $user;

  // Get the list of modules
  $sql = '';
  $args = [':feature_id' => $feature_id];
  $sql = "
    SELECT * FROM {network} N
      INNER JOIN {network_node} NN on NN.network_id = N.network_id
    WHERE NN.feature_id = :feature_id
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

  $form['feature_id'] = [
    '#type' => 'value',
    '#value' => $feature_id,
    '#prefix' => '<div id="tripal-network-neighborhood-viewer-form">',
  ];
  $form['network_id'] = [
    '#type' => 'select',
    '#title' => 'Network',
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('Select the network to view connections for this feature.'),
    '#ajax' => array(
      'callback' => 'tripal_network_neighborhood_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-neighborhood-viewer-form',
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
function tripal_network_neighborhood_viewer_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];
  $feature_id = $form_state['values']['feature_id'];

  $args = [
    'feature_id' => $feature_id,
    'network_id' => $network_id,
    'viewer_id' => 'tripal-network-neighborhood-viewer'
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetworkNeighborhood", [$args]));

  return $form;
}
<?php

/**
 *
 */
function tripal_network_viewer_page($network_id = NULL) {
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'drupal.form');
  return theme('tripal_network_viewer', ['network_id' => $network_id]);
}

function tripal_network_viewer_layout(){
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
  return $layout;
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
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : 'SWO:0000425';
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : 'SWO:0000425';

  $layout = tripal_network_viewer_layout();

  $edges_trace = [];

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

  // Make sure we have required filters.
  if ($network_id == 0) {
    drupal_json_output([
      'data' => [$edges_trace, $nodes_trace],
      'layout' => $layout,
    ]);
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
  $where = " WHERE NE.network_id = :network_id ";
  $order = " ";
  $limit = " ";


  $sql = $select . $from . $where . $order . $limit;

  // Execute the query and return the parsed results.
  $results = chado_query($sql, $params);
  $nodes = [];
  $edges = [];
  while ($record = $results->fetchObject()) {
    $nodes[$record->node1_id] = $record->node1;
    $nodes[$record->node2_id] = $record->node2;
    $edges[$record->network_edge_id] = $record;
  }

  // Get properties of the nodes used for the viewer.
  $x_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network_id, 'SIO:000400');
  $y_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network_id, 'SIO:000401');
  $degree = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id', $network_id, 'OBI:0200130');

  $layerprop = tripal_network_get_edgeprops(array_keys($edges), $layer_by);
  if ($layer_by != $color_by) {
    $colorprop = tripal_network_get_edgeprops(array_keys($edges), $color_by);
  }
  else {
    $colorprop = $layerprop;
  }

  // For quantitative properties get min/max.
  $layer_max = -INF;
  $layer_min = INF;
  $layer_categories = [];
  if ($layer_by == 'NCIT:C48904' or $layer_by == 'SWO:0000425' or
      $layer_by == 'local:KINC_pvalue' or $layer_by == 'local:KINC_rsquared') {
    foreach ($layerprop as $value) {
      if ($value > $layer_max) {
        $layer_max = $value;
      }
      if ($value < $layer_min) {
        $layer_min = $value;
      }
    }
  }
  else {
    foreach ($layerprop as $value) {
      $layer_categories[$value] = 1;
    }
  }
  $layer_categories = array_keys($layer_categories);
  asort($layer_categories);

  // For quantitative properties get min/max.
  $color_max = -INF;
  $color_min = INF;
  $color_categories = [];
  if ($color_by == 'NCIT:C48904' or $color_by == 'SWO:0000425' or
      $color_by == 'local:KINC_pvalue' or $color_by == 'local:KINC_rsquared') {
    foreach ($colorprop as $value) {
      if ($value > $color_max) {
        $color_max = $value;
      }
      if ($value < $color_min) {
        $color_min = $value;
      }
    }
  }
  else {
    foreach ($colorprop as $value) {
      $color_categories[$value] = 1;
    }
  }
  $color_categories = array_keys($color_categories);
  asort($color_categories);

  // Calculate the node size.
  $sizes = [];
  foreach ($degree as $node_id => $value) {
    $sizes[$node_id] = max(round(log10($value)*10), 5);
  }

  // Set the x,y coordianates for nodes and edges.
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

  $edge_coords = [];
  foreach ($edges as $network_edge_id => $edge) {
    $z = array_key_exists($network_edge_id, $layerprop) ? $layerprop[$network_edge_id] : 0;
    if ($layer_by == 'SWO:0000425') {
      $z = round(abs($z), 2);
    }
    if ($layer_by == 'NCIT:C48904') {
      $num_rank_bins = $layer_max / 100;
      $z = 100 - round($z/$num_rank_bins);
    }
    if ($layer_by == 'local:KINC_pvalue') {
      $z = -log10($z);
    }
    if ($layer_by == 'local:KINC_test_name') {
      $z = array_search($z, $layer_categories);
      $num_cat_bins = count($layer_categories) / 100;
      $z = round($z/$num_cat_bins);
      $z = $z - ($z % 5);
    }

    $edge_coords[$network_edge_id] = [
      'source' => [
        'x' => $node_coords[$edge->node1_id]['x'],
        'y' => $node_coords[$edge->node1_id]['y'],
        'z' => $z,
      ],
      'target' => [
        'x' => $node_coords[$edge->node2_id]['x'],
        'y' => $node_coords[$edge->node2_id]['y'],
        'z' => $z,
      ],
    ];
    // Move the node's z-coordinate to the highest value score.
    $node_coords[$edge->node1_id]['z'] = $z;
    $node_coords[$edge->node2_id]['z'] = $z;
  }

  // Create the node trace
  foreach ($nodes as $node_id => $node_name) {
    $nodes_trace['x'][] = $node_coords[$node_id]['x'];
    $nodes_trace['y'][] = $node_coords[$node_id]['y'];
    $nodes_trace['z'][] = $node_coords[$node_id]['z'];
    $nodes_trace['ids'][] = $node_id;
    $nodes_trace['text'][] = $node_name;
    $nodes_trace['marker']['size'][] = $sizes[$node_id];
    $nodes_trace['marker']['color'][] = $node_id == $feature_id ? '#FF0000': '#AAAAAA';
  }

  // We want to color the edges using a gradient and unlike Python/R
  // this is the easiest way to get the colors in PHP.
  // https://waldyrious.net/viridis-palette-generator/
  $virdis_colors = [
    '100' => 'rgb(253, 231, 37)',
    '95' => 'rgb(223, 227, 24)',
    '90' => 'rgb(189, 223, 38)',
    '85' => 'rgb(155, 217, 60)',
    '80' => 'rgb(122, 209, 81)',
    '75' => 'rgb(94, 201, 98)',
    '70' => 'rgb(68, 191, 112)',
    '65' => 'rgb(47, 180, 124)',
    '60' => 'rgb(34, 168, 132)',
    '55' => 'rgb(30, 156, 137)',
    '50' => 'rgb(33, 145, 140)',
    '45' => 'rgb(37, 132, 142)',
    '40' => 'rgb(42, 120, 142)',
    '35' => 'rgb(47, 108, 142)',
    '30' => 'rgb(53, 95, 141)',
    '25' => 'rgb(59, 82, 139)',
    '20' => 'rgb(65, 68, 135)',
    '15' => 'rgb(70, 52, 128)',
    '10' => 'rgb(72, 36, 117)',
    '5' => 'rgb(71, 19, 101)',
    '0' => 'rgb(68, 1, 84)',
  ];


  $tick_vals = [];
  foreach ($edges as $network_edge_id => $edge) {
    $ci = $colorprop[$network_edge_id];
    $trace_name = '';
    if ($color_by == 'SWO:0000425') {
      if (!is_numeric($ci)) {
        $ci = 0;
      }
      $ci = abs($ci);
      $ci = strval(round(($ci * 2), 1) / 2) * 100;
      $trace_name = $ci;
    }
    if ($color_by == 'NCIT:C48904') {
      if (!is_numeric($ci)) {
        $ci = 0;
      }
      $num_rank_bins = $color_max / 100;
      $ci = 100 - round(round($ci)/$num_rank_bins);
      $ci = $ci - ($ci % 5);
      $trace_name = $ci;
    }
    if ($color_by == 'local:KINC_pvalue') {
      if (!is_numeric($ci)) {
        $ci = 0;
      }
      $num_pval_bins = round(-log10($color_min)) / 100;
      $ci = round(round(-log10($ci))/$num_pval_bins);
      $ci = $ci - ($ci % 5);
      $trace_name = $ci;
    }
    if ($color_by == 'local:KINC_test_name') {
      $ci = array_search($ci, $color_categories);
      $num_cat_bins = count($color_categories) / 100;
      $ci = round($ci/$num_cat_bins);
      $ci = $ci - ($ci % 5);
      $trace_name = $colorprop[$network_edge_id];
    }

    $trace_name = strval($trace_name);
    if (!array_key_exists($trace_name, $edges_trace)) {
      if (!is_scalar($trace_name)){
        continue;
      }
      $edges_trace[$trace_name] = tripal_network_viewer_create_edge_trace($trace_name);
    }

    $tick_vals[$edge_coords[$network_edge_id]['target']['z']] = 1;
    $edges_trace[$trace_name]['x'][] = $edge_coords[$network_edge_id]['source']['x'];
    $edges_trace[$trace_name]['y'][] = $edge_coords[$network_edge_id]['source']['y'];
    $edges_trace[$trace_name]['z'][] = $edge_coords[$network_edge_id]['source']['z'];
    $edges_trace[$trace_name]['x'][] = $edge_coords[$network_edge_id]['target']['x'];
    $edges_trace[$trace_name]['y'][] = $edge_coords[$network_edge_id]['target']['y'];
    $edges_trace[$trace_name]['z'][] = $edge_coords[$network_edge_id]['target']['z'];
    // Add an empty entry to keep Plotly from adding an extra line.
    $edges_trace[$trace_name]['x'][] = NULL;
    $edges_trace[$trace_name]['y'][] = NULL;
    $edges_trace[$trace_name]['z'][] = NULL;
    $edges_trace[$trace_name]['ids'][] = $edge->network_edge_id;
    $edges_trace[$trace_name]['ids'][] = $edge->network_edge_id;
    $edges_trace[$trace_name]['ids'][] = $edge->network_edge_id;
    $edges_trace[$trace_name]['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $edges_trace[$trace_name]['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $edges_trace[$trace_name]['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $edges_trace[$trace_name]['line']['color'][] = $virdis_colors[$ci];
    $edges_trace[$trace_name]['line']['color'][] = $virdis_colors[$ci];
    $edges_trace[$trace_name]['line']['color'][] = $virdis_colors[$ci];
  }

  $layer_term = chado_get_cvterm(['id' => $layer_by]);
  $layout['scene']['zaxis']['title'] = ucfirst($layer_term->name);
  if ($color_by == 'local:KINC_test_name') {
    $tick_vals = array_keys($tick_vals);
    arsort($tick_vals);
    $layout['scene']['zaxis']['tickmode'] = 'array';
    $layout['scene']['zaxis']['tickvals'] = $tick_vals;
    $layout['scene']['zaxis']['ticktext'] = array_keys($edges_trace);
  }
  else {
    $layout['scene']['zaxis']['tickmode'] = 'auto';
  }

  krsort($edges_trace);
  $data = array_merge([$nodes_trace], array_values($edges_trace));
  drupal_json_output(['data' => $data, 'layout' => $layout]);
}


/**
 *
 */
function tripal_network_viewer_get_z() {

}
/**
 *
 */
function tripal_network_viewer_network_form($form, &$form_state, $network_id = NULL) {

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
    $networks[$record->genus . ' ' . $record->species][$record->network_id] = $record->name;
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


  $allowed_layers = [
    // 'OBI:0200130',  // degree
    'SWO:0000425', // similarity score
    'local:KINC_test_name',
    'local:KINC_pvalue',
    'local:KINC_rsquared',
    'NCIT:C48904', // rank
    //'local:KINC_WAnova_Max',
    //'local:KINC_WAnova_Min',
    //'local:KINC_Missing_Ttest',
  ];
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
      // We only support specific fields
      if (in_array($term, $allowed_layers)) {
        $props[$term] = $record->name;
      }
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



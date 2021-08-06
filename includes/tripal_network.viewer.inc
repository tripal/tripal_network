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
        'title' => 'Similarity Score',
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
  return $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_retrieve() {
  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;

  $layout = tripal_network_viewer_layout();

  $edges_trace = [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => [],
      'width' => 2,
      'opacity' => 0.9,
    ],
    'name' => 'Edge',
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
  $simscores = tripal_network_get_edgeprops(array_keys($edges), 'SWO:0000425');

  // Calculate the node size.
  $sizes = [];
  foreach ($degree as $node_id => $value) {
    $sizes[$node_id] = max(round(log10($value)*10), 5);
  }

  // Calculate teh x,y,z coordianates for nodes and edges.
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
    $z = array_key_exists($network_edge_id, $simscores) ? $simscores[$network_edge_id] : 0;
    $z = abs($z);
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
    $node_coords[$edge->node1_id]['z'] = max($z, $node_coords[$edge->node1_id]['z']);
    $node_coords[$edge->node2_id]['z'] = max($z, $node_coords[$edge->node2_id]['z']);
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
    '1' => 'rgb(253, 231, 37)',
    '0.95' => 'rgb(223, 227, 24)',
    '0.9' => 'rgb(189, 223, 38)',
    '0.85' => 'rgb(155, 217, 60)',
    '0.8' => 'rgb(122, 209, 81)',
    '0.75' => 'rgb(94, 201, 98)',
    '0.7' => 'rgb(68, 191, 112)',
    '0.65' => 'rgb(47, 180, 124)',
    '0.6' => 'rgb(34, 168, 132)',
    '0.55' => 'rgb(30, 156, 137)',
    '0.5' => 'rgb(33, 145, 140)',
    '0.45' => 'rgb(37, 132, 142)',
    '0.4' => 'rgb(42, 120, 142)',
    '0.35' => 'rgb(47, 108, 142)',
    '0.3' => 'rgb(53, 95, 141)',
    '0.25' => 'rgb(59, 82, 139)',
    '0.2' => 'rgb(65, 68, 135)',
    '0.15' => 'rgb(70, 52, 128)',
    '0.1' => 'rgb(72, 36, 117)',
    '0.05' => 'rgb(71, 19, 101)',
    '0' => 'rgb(68, 1, 84)',
  ];


  foreach ($edges as $network_edge_id => $edge) {
    $edges_trace['x'][] = $edge_coords[$network_edge_id]['source']['x'];
    $edges_trace['y'][] = $edge_coords[$network_edge_id]['source']['y'];
    $edges_trace['z'][] = $edge_coords[$network_edge_id]['source']['z'];
    $edges_trace['x'][] = $edge_coords[$network_edge_id]['target']['x'];
    $edges_trace['y'][] = $edge_coords[$network_edge_id]['target']['y'];
    $edges_trace['z'][] = $edge_coords[$network_edge_id]['target']['z'];
    // Add an empty entry to keep Plotly from adding an extra line.
    $edges_trace['x'][] = NULL;
    $edges_trace['y'][] = NULL;
    $edges_trace['z'][] = NULL;
    $edges_trace['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $edges_trace['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $edges_trace['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
    $ci = $edge_coords[$network_edge_id]['target']['z'];
    $ci = strval(round(($ci * 2),1) / 2);
    $edges_trace['line']['color'][] = $virdis_colors[$ci];
    $edges_trace['line']['color'][] = $virdis_colors[$ci];
    $edges_trace['line']['color'][] = $virdis_colors[$ci];
  }

  drupal_json_output(['data' => [$edges_trace, $nodes_trace], 'layout' => $layout]);
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
    'viewer_id' => 'tripal-network-viewer'
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
      // We only support specific fields
      if (in_array($record->db_name . ':' . $record->accession, $allowed_layers)) {
        $props[$record->cvterm_id] = $record->name;
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
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_display_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-display-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

  $form['color_by'] = [
    '#type' => 'select',
    '#title' => 'Color Edges By',
    '#options' => $props,
    '#default_value' => $color_by,
    '#description' => t('Select a property to color edges.'),
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

<?php 

/**
 *
 * @return
 */
function tripal_network_get_plotly_default_layout(){
  return [
    'autosize' => TRUE,
    'paper_bgcolor' => "#000000",
    'margin' => ['l' => 30, 'r' => 20, 't' => 50, 'b' => 20],
    'showlegend' => TRUE,
    'legend' => [
      'font' => [
        'color' => "#FFFFFF",
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
        'showgrid' => TRUE  ,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#FFFFFF"
      ],
      'yaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => TRUE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#FFFFFF"
      ],
      'zaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => TRUE,
        'showticklabels' => TRUE,
        'showspikes' => FALSE,
        'title' => '',
        'color'=> "#FFFFFF"
      ]
    ],
    'title' => '',
  ];
}

/**
 *
 */
function tripal_network_get_plotly_data($network_items, &$layout, $layer_by, $color_by, $is_empty = FALSE) {
  
  $edge_trace_defaults = [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => [],
      'width' => 4,
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
  
  $node_trace_defaults = [
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
  
  if ($is_empty or !$network_items or empty($network_items['edges'])) {
    return [$edge_trace_defaults, $node_trace_defaults];
  }
  
  $network_id = array_key_exists('network_id', $network_items) ? $network_items['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $network_items) ? $network_items['organism_id'] : NULL;
  $edges = $network_items['edges'];
  $nodes = $network_items['nodes'];
  
  
  module_load_include("inc", "tripal_network", "api/tripal_network.fa2.api");
  $fa2_options = [
    'ticks' => 100,
    'repulsion_coefficient' => 2,
    'gravity_coefficient' => 0.5,
    'barnes_hut_optimize' => TRUE,
    'barnes_hut_theta' => 1.2,
    'edge_weight_influence' => 1.0,
    'distribute_attraction' => TRUE,
    'jitter_tolerance' => 1.0,
    
  ];
  
  // If this is a single network then we may already have the x,y coordinates
  // loaded, in which case, let's use those. Otherwise, we need to 
  // create a new graph layout.
  if ($network_id) {
    $x_coords = tripal_network_get_nodeprops(array_keys($nodes), 'network_node_id',
        'SIO:000400', $organism_id, $network_id);
    $y_coords = tripal_network_get_nodeprops(array_keys($nodes), 'network_node_id',
        'SIO:000401', $organism_id, $network_id);
    $degree = tripal_network_get_nodeprops(array_keys($nodes), 'network_node_id',
        'OBI:0200130', $organism_id, $network_id);
    if (empty($x_coords)) {
      tripal_network_apply_fa2_layout($nodes, $edges, $fa2_options);
    }
    else {
      foreach ($nodes as $node_id => $node) {
        $x = array_key_exists($node_id, $x_coords) ? $x_coords[$node_id] : mt_rand(0, 10);
        $y = array_key_exists($node_id, $y_coords) ? $y_coords[$node_id] : mt_rand(0, 10);
        $deg = array_key_exists($node_id, $degree) ? $degree[$node_id] : 1;
        $nodes[$node_id]['degree'] = $deg;
        $nodes[$node_id]['x'] = $x;
        $nodes[$node_id]['y'] = $y;
      }
    }
  }
  else {
    tripal_network_apply_fa2_layout($nodes, $edges, $fa2_options);
  }
       
  
  
  // Calculate teh x,y,z coordianates for nodes.
  $node_coords = [];
  foreach ($nodes as $node_id => $node) {
    $node_coords[$node_id] = [
      'x' => $node['x'],
      'y' => $node['y'],
      'z' => 0
    ];
    
  }
  
  // Get the layer and colors for the selected properties.
  $edge_ids = array_keys($edges);
  $layers = tripal_network_layer_edges($organism_id, $network_id, $edge_ids, $layer_by);
  $colors = tripal_network_color_edges($organism_id, $network_id, $edge_ids, $color_by);
  
  // Create the edges trace.
  $traces = [];
  foreach ($edges as $network_edge_id => $edge) {
    
    // Get the z-coord, bin and color for this edge.
    $layer = array_key_exists($network_edge_id, $layers['item_layer']) ? $layers['item_layer'][$network_edge_id] : 0;
    $color = array_key_exists($network_edge_id, $colors['item_color']) ? $colors['item_color'][$network_edge_id] : 0;
    $layer_name = $colors['labels'][$color];
    
    $z = 0;
    if ($layers['data_class'] == 'quantitative') {
      $z = array_key_exists($network_edge_id, $layers['adj_values']) ? $layers['adj_values'][$network_edge_id] : 0;
    }
    else {
      $z = $layer;
    }
    
    // Add a new trace if we haven't seen this bin yet.
    if (!array_key_exists($layer_name, $traces)) {
      $traces[$layer_name] = $edge_trace_defaults;
      $traces[$layer_name]['name'] = $layer_name;
      $traces[$layer_name]['line']['color'] = $color;
    }
    
    $source_node_id = $edge['source_node_id'];
    $target_node_id = $edge['target_node_id'];
    
    // Set the line x,y,z coordinates
    $traces[$layer_name]['x'][] = $node_coords[$source_node_id]['x'];
    $traces[$layer_name]['y'][] = $node_coords[$source_node_id]['y'];
    $traces[$layer_name]['z'][] = $z;
    $traces[$layer_name]['x'][] = $node_coords[$target_node_id]['x'];
    $traces[$layer_name]['y'][] = $node_coords[$target_node_id]['y'];
    $traces[$layer_name]['z'][] = $z;
    
    // Add an empty entry to keep Plotly from adding an extra line.
    $traces[$layer_name]['x'][] = NULL;
    $traces[$layer_name]['y'][] = NULL;
    $traces[$layer_name]['z'][] = NULL;
    
    // Set the line text.
    $text = $edge['source_name'] . ' (-) ' . $edge['target_name'];
    $traces[$layer_name]['text'][] = $text;
    $traces[$layer_name]['text'][] = $text;
    $traces[$layer_name]['text'][] = $text;
    
    // Set the line ids.
    $traces[$layer_name]['ids'][] = $network_edge_id;
    $traces[$layer_name]['ids'][] = $network_edge_id;
    $traces[$layer_name]['ids'][] = $network_edge_id;
    
    // Move the node's z-coordinate to the highest value score.
    if ($layers['data_class'] == 'quantitative') {
      $node_coords[$source_node_id]['z'] = max($z, $node_coords[$source_node_id]['z']);
      $node_coords[$target_node_id]['z'] = max($z, $node_coords[$target_node_id]['z']);
      $node_coords[$source_node_id]['layer'] = $layer;
      $node_coords[$target_node_id]['layer'] = $layer;
      
      // If the layer by is "rank" then put nodes at the lowest rank they
      // first appear. Otherwise, use the highest.
      if ($layer_by == 'NCIT:C48904' and $node_coords[$source_node_id]['z'] != 0) {
        $node_coords[$source_node_id]['z'] = min($z, $node_coords[$source_node_id]['z']);
        $node_coords[$target_node_id]['z'] = min($z, $node_coords[$target_node_id]['z']);
        $node_coords[$source_node_id]['layer'] = $layer;
        $node_coords[$target_node_id]['layer'] = $layer;
      }
    }
    else {
      $node_coords[$source_node_id]['z'] = $z;
      $node_coords[$target_node_id]['z'] = $z;
      $node_coords[$source_node_id]['layer'] = $layer;
      $node_coords[$target_node_id]['layer'] = $layer;
    }
  }
  
  if ($colors['data_class'] == 'categorical') {
    ksort($traces);
  }
  else {
    ksort($traces, SORT_NUMERIC);
  }
  
  // Create the node trace
  $traces['nodes'] = $node_trace_defaults;
  foreach ($nodes as $node_id => $node) {
    
    // The bin for the node is the z coordinate assigned to the node.
    $layer = $node_coords[$node_id]['layer'];
    $traces['nodes']['x'][] = $node_coords[$node_id]['x'];
    $traces['nodes']['y'][] = $node_coords[$node_id]['y'];
    $traces['nodes']['z'][] = $node_coords[$node_id]['z'];
    $traces['nodes']['ids'][] = $node_id;
    $traces['nodes']['text'][] = $node['node_name'];
    $traces['nodes']['marker']['size'][] = max(round(log10($nodes[$node_id]['degree'])*10), 5);
    $traces['nodes']['marker']['color'][] = in_array($node_id, $network_items['feature_ids']) ? '#FFFF00AA': '#AAAAAAAA';
  }
  
  // Now set the tick labels
  if ($layers['data_class'] == 'categorical') {
    $layout['scene']['zaxis']['tickmode'] = "array";
    $layout['scene']['zaxis']['tickvals'] = $layers['layers'];
    $layout['scene']['zaxis']['ticktext'] = $layers['layer_labels'];
  }
  $layout['scene']['zaxis']['title'] = $layers['title'];
  
  return array_values($traces);
}

/**
 * 
 */
function tripal_network_get_plotly_degree_distribution_plot_layout(){
  return [
    'title' => "Node Degree Distribution",
    'xaxis' => [
      'type' => "log",
      'title' => 'log(degree)'
    ],
    'yaxis' => [
      'type' => "log",
      'title' => 'log(count)'
    ]
  ];
}

function tripal_network_get_plotly_degree_distribution_plot_data($network_details){
  
  $plot_data = [
    'x' => [],
    'y' => [],
    'mode' => 'markers',
    'type' => 'scatter'
  ];
  $degree_count = [];
  foreach ($network_details['nodes'] as $node) {
    if (!array_key_exists($node['degree'], $degree_count)) {
      $degree_count[$node['degree']] = 1;
    }
    else {
      $degree_count[$node['degree']]++;
    }
  }
  foreach ($degree_count as $degree => $cnt) {
    $plot_data['x'][] = $degree;
    $plot_data['y'][] = $cnt;
  }
  return $plot_data;
}
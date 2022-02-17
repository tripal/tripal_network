<?php

class TripalNetworkViewerPlotly extends TripalNetworkViewer {
  
  private function getDefaultLayout() {
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
        'aspectratio' => ['x' => 2, 'y' => 2, 'z' => 1
        ],
        'camera' => [
          'up'=> ['x'=> 0, 'y'=> 0, 'z' => 1],
          'center'=> ['x'=> 0, 'y' => 0, 'z' => 0],
          'eye' => ['x' => 0, 'y' => 0, 'z' => 1],
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
  private function getEdgeTraceDefaults() {
    return [
      'mode' => 'lines',
      'type' => 'scatter3d',
      'line' => [
        'color' => [],
        'width' => 3,
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
  }
  
  /**
   * 
   */
  private function getNodeTraceDefaults() {
    return [
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
  }
  
  private function getEmptyNetwork () {
    $edge_trace_defaults = $this->getEdgeTraceDefaults();
    $node_trace_defaults = $this->getNodeTraceDefaults();
    return [$edge_trace_defaults, $node_trace_defaults];    
  }
  
  /**
   * 
   */
  public function getNetworkPlot() {
    
    // If there is no network object then return an empty network,
    if (!$this->network) {
      return $this->getEmptyNetwork();
    }
    
    // If this network has no edges then return an empty network.
    $edges = $this->network->getEdges();  
    $network_ids = $this->network->getNetworkIDs();
    if (count(array_keys($edges)) == 0) {
      return $this->getEmptyNetwork();
    }
    
    $edge_trace_defaults = $this->getEdgeTraceDefaults();    
    $node_trace_defaults = $this->getNodeTraceDefaults();
    
    $selection = $this->network->getSelection();    
    
    $network_ids = array_key_exists('network_ids', $selection) ? $selection['network_ids'] : NULL;
    
    module_load_include("inc", "tripal_network", "api/tripal_network.fa2.api");
     
    // If this is a single network then we may already have the x,y coordinates
    // loaded, in which case, let's use those. Otherwise, we need to
    // create a new graph layout.
    if (count(array_keys($network_ids)) == 1) {
      $nodes = $this->network->getNodes(); 
      
      // Load the x, y coordiantes and degree properties if they are already
      // present.
      $num_x_coords = $this->network->retreiveNodeProp('SIO:000400');
      $num_y_coords = $this->network->retrieveNodeProp('SIO:000401');
      $num_degree = $this->network->retreiveNodeProp('OBI:0200130');   
      
      // If we don't have a degree for every node then recalucate it.
      if ($num_degree != count(array_keys($nodes))) {
        $this->network->calculateDegree();
        $this->network->calculateLocalCC();
      }      
      
      // If the x and y coordiantes aren't avaialble for all nodes then
      // regenerate a new layout.
      if ($num_x_coords != count(array_keys($nodes)) or $num_y_coords != count(array_keys($nodes))) {        
        $layout2d = new TripalNetworkLayoutFA2($this->network);
        $layout2d->applyLayout();
      }       
    }
    else {
      $this->network->calculateDegree();
      $this->network->calculateLocalCC();
      $layout2d = new TripalNetworkLayoutFA2($this->network);
      $layout2d->applyLayout();
    }
       
    
    // Initialize the array to calculate the z coordianates for nodes.
    $nodes = $this->network->getNodes(); 
    $node_coords = [];
    foreach ($nodes as $node_id => $node) {
      $node_coords[$node_id] = [
        'x' => $node['SIO:000400'],
        'y' => $node['SIO:000401'],
        'z' => 0
      ];    
    }
    
    // Get the layer and colors for the selected properties.
    $layers = $this->layerEdges($this->layer_by);
    $colors = $this->colorEdges($this->color_by);
    
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
        if ($this->layer_by == 'NCIT:C48904' and $node_coords[$source_node_id]['z'] != 0) {
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
      $traces['nodes']['marker']['size'][] = max(round(log10($nodes[$node_id]['OBI:0200130'])*10), 5);
      $traces['nodes']['marker']['color'][] = in_array($node_id, $selection['feature_ids']) ? '#FFFF00AA': '#AAAAAAAA';
    }
    
    // Now set the tick labels
    $layout = $this->getDefaultLayout();
    if ($layers['data_class'] == 'categorical') {
      $layout['scene']['zaxis']['tickmode'] = "array";
      $layout['scene']['zaxis']['tickvals'] = $layers['layers'];
      $layout['scene']['zaxis']['ticktext'] = $layers['layer_labels'];
    }
    //$layout['scene']['zaxis']['title'] = $layers['title'];
    
    return [
      'layout' => $layout, 
      'data' => array_values($traces)      
    ];
  }
  
  /**
   * 
   */
  private function layerEdges($layer_by) {
    
    $filters = $this->network->getFilters();
    $this->network->retrieveEdgeProp($layer_by);    
    $edges = $this->network->getEdges();
    
    // We want to take the absolute value of some property types.
    $abs = FALSE;
    if ($layer_by == 'NCIT:C48834') {
      $abs = TRUE;
    }        
    
    $attributes = $this->network->getEdgeAttrs();
    
    
    if (!array_key_exists($layer_by, $attributes)) {
      return [];
    }   
    $attribute = $attributes[$layer_by];   
   
    $data_class = NULL;
    $data_type = NULL;
    $smallest = INF;
    $max = -INF;
    $min = INF;
    $categories = [];
    $num_categories = 0;
    $term = NULL;
    
    // The module stores propeties about each attribute in the
    // network_attrprop table which has a network_id field.  
    // But we're hardcoding if the properties are quantiative, 
    // ordinal or categorical so it's okay.
    foreach ($attribute as $network_id => $attr_details) {
      foreach ($attr_details['attr']->network_attrprop as $property) {
      
        $term = $property->type_id;
        $prop_term = $property->type_id->dbxref_id->db_id->name . ':' . $property->type_id->dbxref_id->accession;
        $value_name = $property->value_id ? $property->value_id->name : '';
        if ($prop_term == 'NCIT:C25161') {
          if ($value_name == 'Quantitative Variable') {
            $data_class = 'quantitative';
          }
          if ($value_name == 'Qualitative Variable') {
            $data_class = 'categorical';
          }
          if ($value_name == 'Ordinal Scale') {
            $data_class = 'ordinal';
          }
        }
        if ($prop_term == 'NCIT:C25570') {
          $min = $property->value_num;
        }
        if ($prop_term == 'NCIT:C25564') {
          $max = $property->value_num;
        }
        if ($prop_term == 'NCIT:C114464') {
          if (count(array_keys($filters)) > 0) {
            $categories = [];
            foreach ($filters as $filter) {
              if ($filter['limit_by'] == $layer_by) {
                $categories = array_merge($categories, array_keys($filter['categories']));
              }
            }
          }
          else {
            $categories[] = $property->value;
          }                  
          $num_categories = count($categories);
        }
        if ($prop_term == 'NCIT:C42645') {
          $data_type = $value_name;
        }
      }
    }
    
    // We only have 20 colors and we don't want to over run plotly, so
    // let's use 20 bins.
    $num_layers = 20;
    
    $item_layer = [];
    $layer_labels = [];
    $layers = [];
    $adj_values = [];
    $is_log10_transformed = FALSE;
    $is_abs = FALSE;
    
    // Reset the min/max if we need to take the absolute value.
    if ($abs == TRUE) {
      $max = -INF;
      $min = INF;
    }
    
    // Handle quantitiative data.
    if ($data_class == 'quantitative') {
      
      // Get the min and max value and take the abs value if requested.
      foreach ($edges as $network_edge_id => $edge) {
        $value = $edge[$layer_by];
        if (!$value) {
          continue;
        }
        if ($abs === TRUE) {
          $value = abs($value);
          $edges[$network_edge_id][$layer_by] = $value;
          $is_abs = TRUE;
        }
        if ($value > 0 and $value < 0.001 and $value < $smallest) {
          $smallest = $value;
        }
        if ($value > $max) {
          $max = $value;
        }
        if ($value < $min) {
          $min = $value;
        }
      }
      
      // If this data has extremely small numbers then transform.
      $needs_transform = FALSE;
      if ($smallest < 0.0001) {
        $needs_transform = TRUE;
        $max = -log10($smallest);
        $min = 0;
        $is_log10_transformed = TRUE;
      }
      
      // Now do any log transform if needed, otherwise round the value
      // to the nearest 2 decimal places.
      foreach ($edges as $network_edge_id => $edge) {
        $value = $edge[$layer_by];
        if (!$value) {
          $edges[$network_edge_id][$layer_by] = $min;
          continue;
        }
        if ($min >= 0 and $needs_transform) {
          $edges[$network_edge_id][$layer_by] = -log10($value);
        }
        else {
          $edges[$network_edge_id][$layer_by] = round($value, 2);
        }
      }
      
      // Assign the value to a bin.
      $bin_size = ($max - $min) / $num_layers;
      foreach ($edges as $network_edge_id => $edge) {
        $value = $edge[$layer_by];
        $adj_values[$network_edge_id] = $value;
        $item_layer[$network_edge_id] = intval(floor(($value - $min) / $bin_size));
      }
      
      // Set the bin labels.
      for ($i = 0; $i <= 20; $i++){
        if ($data_type == 'Float') {
          $layer_labels[] = strval(round($min + ($bin_size * $i), 2));
        }
        if ($data_type == 'Integer') {
          $layer_labels[] = strval(round($min + ($bin_size * $i), 0));
        }
        $layers[] = $i;
      }
    }
    
    // Handle categorical values.
    if ($data_class == 'categorical') {
      
      // Set the bin labels.
      if ($num_categories <= $num_layers) {
        
        // Use the index of the category to set it's bin.
        foreach ($edges as $network_edge_id => $edge) {
          $value = $edge[$layer_by];
          $item_layer[$network_edge_id] = array_search($value, $categories);
        }
        
        
        for ($i = 0; $i < $num_categories; $i++){
          $layer_labels[] = $categories[$i];
          $layers[] = $i;
        }
      }
      else {
        foreach ($edges as $network_edge_id => $edge) {
          $value = $edge[$layer_by];
          $item_layer[$network_edge_id] = 0;
        }
        
        for ($i = 0; $i < $num_layers; $i++){
          $layer_labels[] = 'Too many categories';
          $layers[] = 0;
        }
      }
    }
    
    
    $title = ucfirst($term->name);
    if ($is_log10_transformed) {
      $title = '-log10 ' . $title;
    }
    if ($is_abs) {
      $title = 'Absolute Value  ' . $title;
    }
    
    return [
      'data_class' => $data_class,
      'data_type' => $data_type,
      'is_log10_transformed' => $is_log10_transformed,
      'is_abs' => $is_abs,
      'adj_values' => $adj_values,
      'item_layer' => $item_layer,
      'layers' => $layers,
      'layer_labels' => $layer_labels,
      'min' => $min,
      'max' => $max,
      'title' => $title,
    ];
  }
  
  /**
   * 
   */
  private function colorEdges($color_by) {
    
    // Get a set of layers by color.
    $layers = $this->layerEdges($color_by);
    
    // The colros array that will be returned.
    $colors = [];
    
    
    // Hard-coded virdis colors
    $virdis_colors = [
      'rgb(68, 1, 84)',
      'rgb(71, 19, 101)',
      'rgb(72, 36, 117)',
      'rgb(70, 52, 128)',
      'rgb(65, 68, 135)',
      'rgb(59, 82, 139)',
      'rgb(53, 95, 141)',
      'rgb(47, 108, 142)',
      'rgb(42, 120, 142)',
      'rgb(37, 132, 142)',
      'rgb(33, 145, 140)',
      'rgb(30, 156, 137)',
      'rgb(34, 168, 132)',
      'rgb(47, 180, 124)',
      'rgb(68, 191, 112)',
      'rgb(94, 201, 98)',
      'rgb(122, 209, 81)',
      'rgb(155, 217, 60)',
      'rgb(189, 223, 38)',
      'rgb(223, 227, 24)',
      'rgb(253, 231, 37)',
    ];
    $num_colors = count($virdis_colors);
    $num_layers = count($layers['layers']);
    if ($num_layers == 0) {
      $num_layers = 1;
    }   
    
    // Convert the layer bin into a color
    $labels = [];
    foreach ($layers['item_layer'] as $network_edge_id => $layer) {
      // Sanity checks.
      if ($layer < 0) {
        $layer = 0;
      }
      if ($layer > 20) {
        $layer = 20;
      }

      // Calculate the index into the color array.
      $ci = floor($layer * ($num_colors / $num_layers));
      
      // Reverse the colors for the rank property.
      if ($this->color_by == 'NCIT:C48904'){
        $ci = ($num_colors - 1) - $ci;
      }
      $colors[$network_edge_id] = $virdis_colors[$ci];
      $labels[$virdis_colors[$ci]] = $layers['layer_labels'][$layer];
    }
    
    return [
      'item_color' => $colors,
      'labels' => $labels,
      'data_class' => $layers['data_class'],
      'data_type' => $layers['data_type'],
    ];
  }
  
  /**
   * 
   */
  public function getDegreeDistPlot() {
    $layout = [
      'title' => "",
      'xaxis' => [
        'type' => "log",
        'title' => 'log(degree)'
      ],
      'yaxis' => [
        'type' => "log",
        'title' => 'log(count)'
      ],
      'width' => 380,
      'height' => 380,
      'margin' => [
        'l' => 60,
        'r' => 50,
        't' => 70,
        'b' => 70,
      ],
    ];
          
    $plot_data = [
      'x' => [],
      'y' => [],
      'mode' => 'markers',
      'type' => 'scatter'
    ];
    $degree_count = [];
    $nodes = $this->network->getNodes();
    
    foreach ($nodes as $node) {
      if (array_key_exists('OBI:0200130', $node)) {
        if (!array_key_exists($node['OBI:0200130'], $degree_count)) {
          $degree_count[$node['OBI:0200130']] = 1;
        }
        else {
          $degree_count[$node['OBI:0200130']]++;
        }
      }
    }
    foreach ($degree_count as $degree => $cnt) {
      $plot_data['x'][] = $degree;
      $plot_data['y'][] = $cnt;
    }
    
    return [
      'layout' => $layout,
      'data' => $plot_data,
    ];
  }
}

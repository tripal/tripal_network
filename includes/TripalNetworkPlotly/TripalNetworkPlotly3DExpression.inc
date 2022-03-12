<?php 

class TripalNetworkPlotly3DExpression extends TripalNetworkPlot3DExpression {
  
  /**
   * 
   */
  protected function defaultLayout() {
    return [
      'autosize' => TRUE,
      'margin' => ['l' => 30, 'r' => 20, 't' => 50, 'b' => 20],
      'showlegend' => TRUE,
      'legend' => [
        'itemsizing' => 'constant',
        'font' => [
          'color' => "#000000",
        ],
        'orientation' =>  'h',
      ],
      'margin' => ['l' => 10, 'r' => 10, 't' => 10, 'b' => 10],
      'scene' => [
        'aspectmode' => "cube",
        'camera' => [
          'eye' => ['x' => 1, 'y' => 1, 'z' => 1],
          'projection' => [
            'type' => 'orthographic'
          ]
        ],
        'xaxis' => [
          'showbackground' => TRUE, 
          'showline' => TRUE, 
          'zeroline' => TRUE, 
          'showgrid' => TRUE,              
          'showticklabels' => TRUE, 
          'title' => '',              
          'showspikes' => TRUE
        ],
        'yaxis' => [
          'showbackground' => TRUE,
          'showline' => TRUE,
          'zeroline' => TRUE,
          'showgrid' => TRUE,
          'showticklabels' => TRUE,
          'title' => '',
          'showspikes' => TRUE
        ],
        'zaxis' => [
          'showbackground' => TRUE,
          'showline' => TRUE,
          'zeroline' => TRUE,
          'showgrid' => TRUE,
          'showticklabels' => TRUE,
          'title' => '',
          'showspikes' => TRUE,
          'tickmode' => 'array',
          'ticktext' => [],
          'tickvals' => [], 
          'nticks' => 0,
          'showspikes' => TRUE,
        ],
        'hovermode' => 'closest',
        'annotations' => [[
          'showarrow'=> FALSE,
          'text' => "",
          'xref' => 'paper', 
          'yref' => 'paper',
          'x' => 0, 'y' => 0.1, 
          'xanchor' => 'left', 
          'yanchor' => 'bottom', 
          'font' => [
            'size' => 14
          ]],
        ],
      ],
    ];
  }
  
  /**
   * 
   */
  protected function defaultTrace() {
    return [
      'x' => [],
      'y' => [],
      'z' => [],
      'type' => 'scatter3d',      
      'mode' => 'markers',
      'text' => [],
      'hoverinfo' => 'text',
      'name' => [],
      'marker' => [
        'symbol' => 'circle',
        'size' => 3,
        'color' => []
      ]
    ];
  }
  
  /**
   * {@inheritDoc}
   * @see TripalNetworkPlot3DExpression::__construct()
   */
  public function __construct(TripalNetwork $network, $edge_id, $z = NULL) {
    parent::__construct($network, $edge_id, $z);
    
    $this->engine = new TripalNetworkPlotlyEngine();
  }
  
  /**
   * 
   * {@inheritDoc}
   * @see TripalNetworkPlot3DExpression::data()
   */
  public function data() {
    
    if (!$this->network) {
      return;
    }
    
    // Putsome data in variables for easier access.
    $edge = $this->edge->details();
    $source = $edge['source_id'];
    $target = $edge['target_id'];
    $sample_names = $this->expression['samples']['names'];
    $sample_attribute_types  = $this->expression['samples']['attributes']['types'];
    $sample_attribute_values = $this->expression['samples']['attributes']['values'];
    $sample_attribute_labels = $this->expression['samples']['attributes']['labels'];
    $edge_attributes = $this->edge->attributes();
    $edge_attribute_values = $this->edge->attributeValues();
    
    $expression = $this->expression['expression'];
    $labels = $sample_attribute_labels[$this->z_axis];
    
    $cluster_samples = [];
    if (array_key_exists('local:KINC_sample_string', $edge_attributes)) {
      $sample_string = $edge_attribute_values['local:KINC_sample_string'];
    }
            
    // TODO: can we gauarantee all expression data is log transformed?
    
    // Set the layout.
    $layout = $this->defaultLayout(); 
    $layout['scene']['xaxis']['title'] = $edge['source_name'];
    $layout['scene']['yaxis']['title'] = $edge['target_name'];
    //$layout['scene']['zaxis']['title'] = $this->z_axis;
    
    // Get the colors for the selected z-axis values.
    $colors = $this->engine->categoryColors($labels);
    
    // Build the traces for plot.       
    $traces = [];
    if ($sample_attribute_types[$this->z_axis] == 'numeric') {
      $trace = $this->defaultTrace();
      $trace['marker']['color'] = $colors;
      
      // Iterate through the samples and add the values.
      foreach ($sample_names as $biomaterial_id => $bname) {
        $trace['x'][] = $expression[$source][$biomaterial_id];
        $trace['y'][] = $expression[$target][$biomaterial_id];
        $trace['text'][] = $bname;
        $trace['name'][] = $sample_attribute_labels[$this->z_axis];;
      }
      $traces[] = $trace;
      
    }
    else {
      $ctraces = [];           
      $layout['scene']['zaxis']['ticktext'] = $labels;
      $layout['scene']['zaxis']['tickvals'] = array_keys($labels);
      $layout['scene']['zaxis']['nticks'] = count($labels);
      foreach ($labels as $label) {
        $ctraces[$label] = $this->defaultTrace();        
      }
      
      foreach ($sample_names as $biomaterial_id => $bname) {
        $label = $sample_attribute_values[$biomaterial_id][$this->z_axis];
        $z = array_search($label, $labels);
        $ctraces[$label]['x'][] = $expression[$source][$biomaterial_id];
        $ctraces[$label]['y'][] = $expression[$target][$biomaterial_id];
        $ctraces[$label]['z'][] = $z;
        $ctraces[$label]['text'] = $label;
        $ctraces[$label]['name'] = $label;
        $ctraces[$label]['marker']['color'] = $colors[$z];
        $ctraces[$label]['hovertext'][] = $bname;
      }             
      $traces = array_values($ctraces);
    }       
    
    return [
      'layout' => $layout,
      'data' => $traces,
    ];
  }
}
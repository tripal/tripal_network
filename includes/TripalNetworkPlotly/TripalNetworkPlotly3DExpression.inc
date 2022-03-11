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
            
    // TODO: can we gauarantee all expression data is log transformed?
    
    // Set the layout.
    $layout = $this->defaultLayout(); 
    $layout['scene']['xaxis']['title'] = $this->expression['feature1']['name'];
    $layout['scene']['yaxis']['title'] = $this->expression['feature2']['name'];
    //$layout['scene']['zaxis']['title'] = $this->z_axis;
    
    // Build the traces for plot.
    $traces = [];
    if ($this->attribute_types[$this->z_axis] == 'numeric') {
      $trace = $this->defaultTrace();
      $trace['marker']['color'] = $this->engine->numericColors($this->sample_attributes[$this->z_axis]);
      
      // Make sure we pair the values by biomaterials.
      foreach ($this->expression['samples'] as $biomaterial_id => $bname) {
        $label = $this->expression['attributes'][$biomaterial_id][$this->z_axis];
        
        $trace['x'][] = $this->expression['feature1']['values'][$biomaterial_id];
        $trace['y'][] = $this->expression['feature2']['values'][$biomaterial_id];
        $trace['text'][] = $bname;
        $trace['name'][] = $label;
      }
      $traces[] = $trace;
      
    }
    else {
      $ctraces = [];
      $labels = array_unique($this->sample_attributes[$this->z_axis]);
      sort($labels, SORT_NUMERIC);
      
      $colors = $this->engine->categoryColors($labels);
      
      $layout['scene']['zaxis']['ticktext'] = $labels;
      $layout['scene']['zaxis']['tickvals'] = array_keys($labels);
      $layout['scene']['zaxis']['nticks'] = count($labels);
      foreach ($labels as $label) {
        $ctraces[$label] = $this->defaultTrace();        
      }
      
      foreach ($this->expression['samples'] as $biomaterial_id => $bname) {
        $label = $this->expression['attributes'][$biomaterial_id][$this->z_axis];
        $z = array_search($label, $labels);
        $ctraces[$label]['x'][] = $this->expression['feature1']['values'][$biomaterial_id];
        $ctraces[$label]['y'][] = $this->expression['feature2']['values'][$biomaterial_id];
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
<?php 

class TripalNetworkPlotlyDegreeDist extends TripalNetworkPlotDegreeDist {
  
  /**
   * 
   */
  protected function defaultLayout() {
    return [
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
  }
  
  /**
   * 
   */
  protected function defaultData() {
    return [
      'x' => [],
      'y' => [],
      'mode' => 'markers',
      'type' => 'scatter'
    ];
  }
  
  
  /**
   * Constructor
   *
   * @param TripalNetwork $network
   *   The TripalNetwork instance.
   */
  public function __construct(TripalNetwork $network) {
    parent::__construct($network);
    
    $this->engine = new TripalNetworkPlotlyEngine();
  }
  
  /**
   *
   */
  public function data() {
    
    $layout = $this->defaultLayout();
    $plot_data = $this->defaultData();
    
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
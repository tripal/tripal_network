<?php

abstract class TripalNetworkPlotNetwork extends TripalNetworkPlot {
  
  protected $layer_by = 'NCIT:C48834';
  protected $color_by = 'NCIT:C48834';
  
  protected $layout = 'random';
  
  
  /**
   * 
   * @param TripalNetwork $network
   */
  public function __construct(TripalNetwork $network, $layout = 'random') {   
    parent::__construct($network);
    
    $this->layout = $layout;    
    $this->network->calculateDegree();
    $this->network->calculateLocalCC();
    $this->applyLayout($layout);
    return;
        
    $network_ids = $this->network->getNetworkIDs();      
    
    // If this is a single network then we may already have the x,y coordinates
    // loaded, in which case, let's use those. Otherwise, we need to
    // create a new graph layout.
    if (count(array_keys($network_ids)) == 1) {
      $nodes = $this->network->getNodes();
      
      // Load "x cartesian coordinate" property.
      $num_x_coords = $this->network->retrieveNodeProp('SIO:000400');
      
      // Load "y cartesian coordinate" property.
      $num_y_coords = $this->network->retrieveNodeProp('SIO:000401');
      
      // Load "node degree calculation" proprety.
      $num_degree = $this->network->retrieveNodeProp('OBI:0200130');
      
      // If we don't have a degree for every node then recalucate it.
      if ($num_degree != count(array_keys($nodes))) {
        $this->network->calculateDegree();
        $this->network->calculateLocalCC();
      }
      
      // If the x and y coordiantes aren't avaialable for all nodes then
      // regenerate a new layout.
      if ($num_x_coords != count(array_keys($nodes)) or $num_y_coords != count(array_keys($nodes))) {
        $this->applyLayout($layout);
      }
    }
    else {
      $this->network->calculateDegree();
      $this->network->calculateLocalCC();
      $this->applyLayout($layout);
    }
  }
  
  /**
   * 
   */
  protected function applyLayout($layout) {
    
    $layout2d = NULL;
    if ($layout == 'fa2') {
      $layout2d = new TripalNetworkLayoutFA2($this->network);
    }
    else if ($layout == 'random') {
      $layout2d = new TripalNetworkLayoutRandom($this->network);
    }
    $layout2d->applyLayout();
  }
  
  /**
   *
   */
  public function layerBy($layer_by) {
    $this->layer_by = $layer_by;
  }
  
  /**
   *
   */
  public function colorBy($color_by) {
    $this->color_by = $color_by;
  }    
}
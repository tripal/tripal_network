<?php

class TripalNetworkViewer {
  
  protected $network = NULL;
  protected $layer_by = 'NCIT:C48834';
  protected $color_by = 'NCIT:C48834';
  
  public function __construct(TripalNetwork $network) {    
    $this->network = $network;
  }
  
  public function layerBy($layer_by) {
    $this->layer_by = $layer_by;
  }
  public function colorBy($color_by) {
    $this->color_by = $color_by;
  }
  
  /**
   * Returns the data needed for display 
   */
  public function getNetworkPlot() {    
    return [
      
    ];
  }
  
  /**
   * Returns the data needed for display
   */
  public function getDegreeDistPlot() {
    return [
      
    ];
  }
  

}
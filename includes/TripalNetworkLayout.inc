<?php 

class TripalNetworkLayout {
  
  protected $network = NULL;
  protected $options = [];
  
  protected $x_coord_term = 'SIO:000400';
  protected $y_coord_term = 'SIO:000401';
  
  public function __construct(TripalNetwork $network, $options = []) {    
    $this->network = $network;
    $this->options = $options;
  }
  
  public function applyLayout() {
    
  }    
}
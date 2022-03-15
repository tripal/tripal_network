<?php 

abstract class TripalNetworkAnalysis {
  
  protected $network = NULL;
  protected $options = [];
    
  public function __construct(TripalNetwork $network, $options = []) {
    $this->network = $network;
    $this->options = $options;
  }
  
  abstract public function performAnalysis(); 
  
}
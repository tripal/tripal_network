<?php 

class TripalNetworkAnalysisFUNCE extends TripalNetworkAnalysis {

  
  protected $background = [];
  protected $annotions = [];
  protected $query = [];
  
  public function __construct(TripalNetwork $network, $query, $options = []) {    
    parent::_construct($network, $options);
  }
  
  protected function getBackground() {
    
  }
  
  protected function getAnnotations() {
    
  }
  
  
  public function performAnalysis() {
    
    
  }
}

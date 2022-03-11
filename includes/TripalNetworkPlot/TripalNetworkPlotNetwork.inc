<?php

abstract class TripalNetworkPlotNetwork extends TripalNetworkPlot {
  
  protected $layer_by = 'NCIT:C48834';
  protected $color_by = 'NCIT:C48834';
    
  public function layerBy($layer_by) {
    $this->layer_by = $layer_by;
  }
  public function colorBy($color_by) {
    $this->color_by = $color_by;
  }    
}
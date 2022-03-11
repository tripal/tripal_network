<?php 

abstract class TripalNetworkPlot {
  
  /**
   * The TripalNetwork object
   */
  protected $network = NULL;
  
  /**
   * The plotting engine should be set by the child class, and
   * should be hard-coded for each plot.
   */
  protected $engine = NULL;
  
  /**
   * The constructor
   * 
   * @param TripalNetwork $network
   *   The network object.
   */
  public function __construct(TripalNetwork $network) {
    $this->network = $network;
  }  
  
  /**
   * Returns the data needed for display
   */  
  abstract public function data();
}
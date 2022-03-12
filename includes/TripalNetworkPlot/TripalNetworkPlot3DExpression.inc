<?php 

abstract class TripalNetworkPlot3DExpression extends TripalNetworkPlot {
   
  /**
   * An array containing details about the edge.
   */
  protected $edge = NULL;
  
  /**
   * The attribute name to use for the z-axis dimension.
   */
  protected $z_axis = NULL;
  
  /**
   * The expression array derived from the Tripalnetwork->getEdgeExpression()
   * function. 
   */
  protected $expression = NULL;
  
  
  protected $available_sample_attributes = [];
  

  /**
   * Generates data for a 3D visualization of pairwise gene expression.
   * 
   * @param TripalNetwork $network
   *   The TripalNetwork instance 
   * @param $edge_id
   *   The edge ID in the network object to plot.
   * @param $z
   *   The attribute name to use for the z-axis.
   */
  public function __construct(TripalNetwork $network, $edge_id, $z = NULL) {
    parent::__construct($network);
       
    
    $this->edge = new TripalNetworkEdge($edge_id);
    $this->z_axis = $z;       
    $this->expression = $this->edge->expression();
    $sample_names = $this->expression['samples']['names'];
    $sample_attributes = $this->expression['samples']['attributes'];
    
    // Don't use sample attributes that have one value per sample or
    // or only 1 unique value.
    foreach ($sample_attributes['names'] as $attr_name) {
      $num_labels = count($sample_attributes['labels'][$attr_name]); 
      if ($num_labels < count($sample_names)) {
        if ($num_labels > 1) {
          $this->available_sample_attributes[] = $attr_name;
        }
      }
    }
    sort($this->available_sample_attributes);
    
    // If the Z value isn't set then pick the first one alphabetically.
    if (!$this->z_axis) {
      $this->z_axis = $this->available_sample_attributes[0];
    }
  }
  
  public function sampleAttrNames() {
    return $this->available_sample_attributes;
  }
  
  /**
   * 
   */
  public function expression() {
    return $this->expression['expression'];
  }
  
  /**
   * 
   */
  public function samples() {
    return $this->expression['samples'];
  }
  
  /**
   * @return TripalnetworkEdge
   */
  public function edge() {
    return $this->edge;
  }
  
}
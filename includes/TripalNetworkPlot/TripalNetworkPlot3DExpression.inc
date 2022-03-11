<?php 

abstract class TripalNetworkPlot3DExpression extends TripalNetworkPlot {
  
  /**
   * The ID of the edge to plot.
   */
  protected $edge_id = NULL;
  
  /**
   * The attribute name to use for the z-axis dimension.
   */
  protected $z_axis = NULL;
  
  /**
   * The expression array derived from the Tripalnetwork->getEdgeExpression()
   * function. 
   */
  protected $expression = NULL;
  
  /**
   * A list of sample attribute values, keyed by attribute name
   */
  protected $sample_attributes = NULL;
  
  /**
   * A list of edge attributes.
   */
  protected $edge_attributes = NULL;
  
  /**
   * Indicates if attributes are categorical or numeric.
   */
  protected $sample_attribute_types = NULL;
  
  /**
   * The number of samples with expression data.
   */
  protected $num_samples = 0;

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
    
    $this->edge_id = $edge_id;
    $this->z_axis = $z;
    
    $this->expression = $this->network->getEdgeExpression($this->edge_id);
    
    // Get the list of values for each attribute.
    foreach ($this->expression['samples'] as $biomaterial_id => $bname) {
      $this->num_samples++;
      $attrs = $this->expression['attributes'][$biomaterial_id];
      foreach ($attrs as $attr_name => $value) {
        $this->sample_attributes[$attr_name][] = $value;
      }
    }
    array_multisort($this->sample_attributes);
    
    // Determine the type (categorical or numeric) for each attribute.
    foreach ($this->sample_attributes as $attr_name => $values) {
      $this->attribute_types[$attr_name] = 'numeric';
      foreach ($values as $value) {
        if (!is_numeric($value)) {
          $this->attribute_types[$attr_name] = 'categorical';
        }
      }
    }    
    
    // Remove attributes that are not numerical but which are a 1 to 1
    foreach ($this->attribute_types as $attr_name => $attr_type) {
      if ($attr_type == 'categorical') {
        if (count($this->sample_attributes[$attr_name]) == count($this->num_samples)) {
          unset($this->attribute_types[$attr_name]);
        }
      }
    }    
    
    // If the Z value isn't set then pick the first one alphabetically.
    if (!$this->z_axis) {
      $this->z_axis = array_keys($this->sample_attributes)[0];
    }
  }   
  
  /**
   * 
   */
  public function sampleAttributes() {
    return $this->sample_attributes;
  }
}
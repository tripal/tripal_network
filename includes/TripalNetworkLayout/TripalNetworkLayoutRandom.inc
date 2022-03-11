<?php 

class TripalNetworkLayoutRandom extends TripalNetworkLayout {
  
  public function applyLayout() {
    
    // Get the network nodes.
    $nodes = $this->network->getNodes();
    
    // Now update the network node properties with x/y coordinates.
    foreach ($nodes as $node_id => $node) {
      // Add a random x coordiante.
      $this->network->addNodeProp($node_id, $this->x_coord_term, mt_rand(0, 10));
      // Add a random y coordinate.
      $this->network->addNodeProp($node_id, $this->y_coord_term, mt_rand(0, 10));
    }
  }    
}
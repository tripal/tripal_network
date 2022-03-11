<?php 

/**
 *
 * The code in this file has been borrowed and adapted for PHP from the
 * https://github.com/bhargavchippada/forceatlas2 project as present on
 * Dec 2021.
 */


class TripalNetworkLayoutFA2 extends TripalNetworkLayout {
  
  protected $options = [
    'ticks' => 100,
    'repulsion_coefficient' => 2,
    'gravity_coefficient' => 0.5,
    'barnes_hut_optimize' => TRUE,
    'barnes_hut_theta' => 1.2,
    'edge_weight_influence' => 1.0,
    'distribute_attraction' => TRUE,
    'jitter_tolerance' => 1.0,
  ];
  
  /**
   * 
   * @param TripalNetwork $network
   * @param array $options
   */
  public function __construct(TripalNetwork $network, $options = []) {
    $this->network = $network;
    
    if (array_key_exists('ticks', $options)) {
      $this->options['ticks'] = $options['ticks'];
    }
    if (array_key_exists('repulsion_coefficient', $options)) {
      $this->options['repulsion_coefficient'] = $options['repulsion_coefficient'];
    }
    if (array_key_exists('gravity_coefficient', $options)) {
      $this->options['gravity_coefficient'] = $options['gravity_coefficient'];
    }
    if (array_key_exists('barnes_hut_optimize', $options)) {
      $this->options['barnes_hut_optimize'] = $options['barnes_hut_optimize'];
    }
    if (array_key_exists('barnes_hut_theta', $options)) {
      $this->options['barnes_hut_theta'] = $options['barnes_hut_theta'];
    }
    if (array_key_exists('edge_weight_influence', $options)) {
      $this->options['edge_weight_influence'] = $options['edge_weight_influence'];
    }
    if (array_key_exists('distribute_attraction', $options)) {
      $this->options['distribute_attraction'] = $options['distribute_attraction'];
    }
    if (array_key_exists('jitter_tolerance', $options)) {
      $this->options['jitter_tolerance'] = $options['jitter_tolerance'];
    }
  }
  
  /**
   * 
   */
  public function applyLayout() {
    
    $ticks = $this->options['ticks'];
    $kg = $this->options['gravity_coefficient'];
    $kr = $this->options['repulsion_coefficient'];
    $bho = $this->options['barnes_hut_optimize'];
    $bht = $this->options['barnes_hut_theta'];
    $ewi = $this->options['edge_weight_influence'];
    $da = $this->options['distribute_attraction'];
    $jt = $this->options['jitter_tolerance'];
    
    $nodes = $this->network->getNodes();
    $edges = $this->network->getEdges();
    
    # Speed and $speed_efficiency are a scaling factor of dx and dy
    # before x and y are adjusted.  These are modified as the
    # algorithm runs to ensure convergence.
    $speed = 1.0;
    $speed_efficiency = 1.0;
    
    // Initialize a starting position for each node and set
    // some other variables.
    // Outbound attraction comparison
    $oac = 1.0;
    foreach ($nodes as $node_id => $node) {
      mt_srand($node_id);
      $nodes[$node_id]['x'] =  floatval(mt_rand(-100, 100))/100;
      $nodes[$node_id]['y'] =  floatval(mt_rand(-100, 100))/100;
      $nodes[$node_id]['dx'] = 0.0;
      $nodes[$node_id]['dy'] = 0.0;
      $nodes[$node_id]['prev_dx'] = 0.0;
      $nodes[$node_id]['prev_dy'] = 0.0;
      $nodes[$node_id]['mass'] = 2.0;
      if (array_key_exists('degree', $nodes[$node_id])) {
        $nodes[$node_id]['mass'] = 1.0 + $nodes[$node_id]['degree'];
      }
      $oac += $nodes[$node_id]['mass'];
    }
    $oac = $oac / count(array_keys($nodes));
    
    // Second, iterate through the edges and adjust the positions
    for ($i = 0; $i < $ticks; $i++) {
      
      // On each tick, reset the change in X and Y coordiantes for each node.
      foreach ($nodes as $node_id => $node) {
        $nodes[$node_id]['prev_dx'] = $nodes[$node_id]['dx'];
        $nodes[$node_id]['prev_dy'] = $nodes[$node_id]['dy'];
        $nodes[$node_id]['dx'] = 0;
        $nodes[$node_id]['dy'] = 0;
      }
      
      
      // Calculate the repulsion forces (region by region or node by node).
      if ($bho) {
        // Use the Barnes Hut optimization.
        $this->calculateBHOrepulsion($nodes, $bht, $kr);
      }
      else {
        // Perform a edgewise replusion.
        $this->calculateEdgewiseRepulsion($nodes, $kr);
      }
      
      
      // Calculate the gravity forces by node.
      foreach ($nodes as $node_id => $node) {
        $this->calculateGravity($nodes, $node_id, $kg);
      }
      
      // Calculate the attraction.forces.
      $this->calculateAttraction($nodes, $edges, $da, $oac, $ewi);
      
      
      
      // Now that we've calculated all the forces, apply these to the nodes.
      $diff = $this->applyForcesOnNodes($nodes, $speed, $speed_efficiency, $jt);
      $speed = $diff['speed'];
      $speed_efficiency = $diff['speed_efficiency'];
      
    }
    
    // Now update the network node properties with x/y coordinates.
    foreach ($nodes as $node_id => $node) {
      
      // Add a random x coordiante.
      $this->network->addNodeProp($node_id, $this->x_coord_term, $node['x']);
      
      // Add a random y coordinate.
      $this->network->addNodeProp($node_id, $this->y_coord_term, $node['y']);
    }
    
  }
  
  /**
   *
   */
  private function buildRegions($nodes) {
    
    if (count(array_keys($nodes)) == 0) {
      return NULL;
    }
    
    $region = [];
    $region['mass'] = 0;
    $region['mass_center_x'] = 0;
    $region['mass_center_y'] = 0;
    $region['node_ids'] = [];
    $region['subregions'] = [];
    $region['size'] = 0.0;
    $region['node_ids'] = array_keys($nodes);
    
    if (count(array_keys($nodes)) == 1) {
      return $region;
    }
    
    // Calculate the center coordinate of all nodes and the sum of node
    // mass based on the nodes present.
    $mass_sum_x = 0;
    $mass_sum_y = 0;
    foreach ($nodes as $node_id => $n) {
      $region['mass'] += $n['mass'];
      $mass_sum_x += $n['x'] * $n['mass'];
      $mass_sum_y += $n['y'] * $n['mass'];
    }
    $region['mass_center_x'] = $mass_sum_x / $region['mass'];
    $region['mass_center_y'] = $mass_sum_y / $region['mass'];
    
    foreach ($nodes as $node_id => $n) {
      $distance = sqrt((($n['x'] - $region['mass_center_x']) * ($n['x'] - $region['mass_center_x'])) +
          (($n['y'] - $region['mass_center_y']) * ($n['y'] - $region['mass_center_y'])));
      $region['size'] = max($region['size'], 2 * $distance);
    }
    
    
    // Intialize the arrays into which the nodes will be
    // sorted based on their quadrants in the x-y plane with
    // the mass_center_x and mass_center_y being the
    // origin point.
    $top_left_nodes = [];
    $bottom_left_nodes = [];
    $top_right_nodes = [];
    $bottom_right_nodes = [];
    
    # Group the nodes based on their quadrants in the x-y plane.
    foreach ($nodes as $node_id => $node) {
      if ($nodes[$node_id]['x'] < $region['mass_center_x']) {
        if ($nodes[$node_id]['y'] < $region['mass_center_y']) {
          $bottom_left_nodes[$node_id] = $node;
        }
        else {
          $top_left_nodes[$node_id] = $node;
        }
      }
      else {
        if ($nodes[$node_id]['y'] < $region['mass_center_y']) {
          $bottom_right_nodes[$node_id] = $node;
        }
        else {
          $top_right_nodes[$node_id] = $node;
        }
      }
    }
    
    if (count(array_keys($top_left_nodes)) > 0) {
      if (count(array_keys($top_left_nodes)) < count(array_keys($nodes))) {
        $region['subregions'][] = $this->buildRegions($top_left_nodes);
      }
      else {
        foreach ($top_left_nodes as $n) {
          $region['subregions'][] = $this->buildRegions([$node_id => $n]);
        }
      }
    }
    if (count(array_keys($bottom_left_nodes)) > 0) {
      if (count(array_keys($bottom_left_nodes)) < count(array_keys($nodes))) {
        $region['subregions'][] = $this->buildRegions($bottom_left_nodes);
      }
      else {
        foreach($bottom_left_nodes as $n) {
          $region['subregions'][] = $this->buildRegions([$node_id => $n]);
        }
      }
    }
    if (count(array_keys($top_right_nodes)) > 0) {
      if (count(array_keys($top_right_nodes)) < count(array_keys($nodes))) {
        $region['subregions'][] = $this->buildRegions($top_right_nodes);
      }
      else {
        foreach($top_right_nodes as $n) {
          $region['subregions'][] = $this->buildRegions([$node_id => $n]);
        }
      }
    }
    
    if (count(array_keys($bottom_right_nodes)) > 0) {
      if (count(array_keys($bottom_right_nodes)) < count(array_keys($nodes))) {
        $region['subregions'][] = $this->buildRegions($bottom_right_nodes);
      }
      else {
        foreach ($bottom_right_nodes as $n) {
          $region['subregions'][] = $this->buildRegions([$node_id => $n]);
        }
      }
    }
    
    return $region;
  }
  
  /**
   *
   */
  private function calculateBHORepulsion(&$nodes, $bht=1.2, $kr=1) {
    
    $root_region = $this->buildRegions($nodes);
    foreach ($root_region['node_ids'] as $node_id) {
      $this->calculateRepulsionVSregions($nodes, $node_id, $root_region, $bht, $kr);
    }
  }
  /**
   *
   */
  private function calculateEdgewiseRepulsion(&$nodes, $kr=1) {
    
    $node_ids = array_keys($nodes);
    for ($i = 0; $i < count(array_keys($nodes)); $i++) {
      for ($j = 0; $j < $i; $j++) {
        $this->calculateRepulsionVSnode($nodes, $node_ids[$i], $node_ids[$j], $kr);
      }
    }
  }
  
  /**
   *
   */
  private function calculateRepulsionVSnode(&$nodes, $node1_id, $node2_id, $kr = 1) {
    
    $xdist = $nodes[$node1_id]['x'] - $nodes[$node2_id]['x'];
    $ydist = $nodes[$node1_id]['y'] - $nodes[$node2_id]['y'];
    $distance_squared = $xdist * $xdist + $ydist * $ydist;
    
    if ($distance_squared > 0) {
      // Nodes will move away from each other proporaltional to their combined
      // mass.  A greater combined mass moves a greater distance.
      $Fr = $kr * $nodes[$node1_id]['mass'] * $nodes[$node2_id]['mass'] / $distance_squared;
      $nodes[$node1_id]['dx'] += $xdist * $Fr;
      $nodes[$node1_id]['dy'] += $ydist * $Fr;
      $nodes[$node2_id]['dx'] -= $xdist * $Fr;
      $nodes[$node2_id]['dy'] -= $ydist * $Fr;
    }
  }
  
  /**
   *
   */
  private function calculateRepulsionVSregion(&$nodes, $node_id, $region, $kr = 1) {
    $xdist = $nodes[$node_id]['x'] - $region['mass_center_x'];
    $ydist = $nodes[$node_id]['y'] - $region['mass_center_y'];
    $distance_squared = $xdist * $xdist + $ydist * $ydist;
    
    if ($distance_squared > 0) {
      // Nodes will move away from the region center proporaltional to their
      // mass.  A greater combined mass moves a greater distance.
      $Fr = $kr * $nodes[$node_id]['mass'] * $region['mass'] / $distance_squared;
      $nodes[$node_id]['dx'] += $xdist * $Fr;
      $nodes[$node_id]['dy'] += $ydist * $Fr;
    }
  }
  
  /**
   *
   */
  private function calculateEdgeAttraction(&$nodes, $node1_id, $node2_id, $ka=1.0,
      $da=TRUE, $ew=1.0) {
        
        $xdist = $nodes[$node1_id]['x'] - $nodes[$node2_id]['x'];
        $ydist = $nodes[$node1_id]['y'] - $nodes[$node2_id]['y'];
        $distance_squared = $xdist * $xdist + $ydist * $ydist;
        $distance = sqrt($distance_squared);
        
        if (!$da) {
          $Fa = $ka * $ew;
        }
        else {
          $Fa = $ka * $ew / $nodes[$node1_id]['mass'];
        }
        
        //$Fa = log(1 + $distance);
        $Fa = 1;
        
        // Nodes will move closer to each other proportional equal to
        // their distance times a factor.
        $nodes[$node1_id]['dx'] -= $xdist * $Fa;
        $nodes[$node1_id]['dy'] -= $ydist * $Fa;
        $nodes[$node2_id]['dx'] += $xdist * $Fa;
        $nodes[$node2_id]['dy'] += $ydist * $Fa;
  }
  
  /**
   *
   */
  private function calculateAttraction(&$nodes, $edges, $da=FALSE, $oac=1.0, $ewi=0.0) {
    
    // Apply attraction forces by edges.
    foreach ($edges as $edge) {
      // Set the attraction coefficient to be 1 or the outbound
      // attraction comparison average.
      $ka = $da ? $oac : 1.0;
      
      if ($ewi == 0) {
        // If there is no influecne of the edge weight then set it to 1.
        $ew = 1;
      }
      elseif ($ewi == 1) {
        // Edge weights aren't supported so just set to 1 for now.
        $ew = 1;
      }
      else {
        $ew = pow($ew, $ewi);
      }
      $this->calculateEdgeAttraction($nodes, $edge['source_node_id'],
          $edge['target_node_id'], $ka, $da, $ew);
    }
  }
  
  /**
   *
   */
  private function calculateGravity(&$nodes, $node_id, $kg=1.0) {
    $xdist = $nodes[$node_id]['x'];
    $ydist = $nodes[$node_id]['y'];
    $distance = sqrt($xdist * $xdist + $ydist * $ydist);
    
    if ($distance > 0) {
      // Nodes with a degree equal to the distance move towards the
      // center at a rate equal to the distance.
      $Fg = $kg * $nodes[$node_id]['mass'] / $distance;
      $nodes[$node_id]['dx'] -= $xdist * $Fg;
      $nodes[$node_id]['dy'] -= $ydist * $Fg;
    }
  }
  /**
   *
   */
  private function calculateRepulsionVSregions(&$nodes, $node_id, $region, $theta = 1.2, $kr=1) {
    
    if (count($region['node_ids']) < 2) {
      _tripal_network_fa2_calculate_repulsion_vs_node($nodes, $node_id, $region['node_ids'][0], $kr);
    }
    else {
      $n = $nodes[$node_id];
      $distance = sqrt((($n['x'] - $region['mass_center_x']) * ($n['x'] - $region['mass_center_x'])) +
          (($n['y'] - $region['mass_center_y']) * ($n['y'] - $region['mass_center_y'])));
      if ($distance * $theta > $region['size']) {
        _tripal_network_fa2_calculate_repulsion_vs_region($nodes, $node_id, $region, $kr);
      }
      else {
        // Now recurse though each subregion.
        foreach ($region['subregions'] as $subregion) {
          _tripal_network_fa2_calculate_repulsion_vs_regions($nodes, $node_id, $subregion, $theta, $kr);
        }
      }
    }
  }
  
  /**
   *
   */
  private function applyForcesOnNodes(&$nodes, $speed=1.0, $speed_efficiency = 1.0,
      $jitter_tolerance = 1.0) {
        
        // Calculate some global swing stats.
        $total_swinging = 0.0;
        $total_effective_traction = 0.0;
        foreach ($nodes as $node_id => $node) {
          $dx = $nodes[$node_id]['dx'];
          $dy = $nodes[$node_id]['dy'];
          $prev_dx = $nodes[$node_id]['prev_dx'];
          $prev_dy = $nodes[$node_id]['prev_dy'];
          $mass = $nodes[$node_id]['mass'];
          
          $swinging = sqrt(($prev_dx - $dx) * ($prev_dx - $dx) + ($prev_dy - $dy) * ($prev_dy - $dy));
          $total_swinging += $mass * $swinging;
          $total_effective_traction +=  0.5 * $mass *
          sqrt(($prev_dx + $dx) * ($prev_dx + $dx) + ($prev_dy + $dy) * ($prev_dy + $dy));
        }
        
        # Optimize jitter tolerance.  The 'right' jitter tolerance for
        # this network. Bigger networks need more tolerance. Denser
        # networks need less tolerance. Totally empiric.
        $estimated_optimal_jitter_tolerance = 0.05 * sqrt(count(array_keys($nodes)));
        $min_JT = sqrt($estimated_optimal_jitter_tolerance);
        $max_JT = 10;
        $jt = $jitter_tolerance * max($min_JT,
            min($max_JT, $estimated_optimal_jitter_tolerance * $total_effective_traction / (
                count(array_keys($nodes)) * count(array_keys($nodes)))));
        
        $min_speed_efficiency = 0.05;
        
        # Protective against erratic behavior
        if ($total_effective_traction and ($total_swinging / $total_effective_traction) > 2.0) {
          if ($speed_efficiency > $min_speed_efficiency) {
            $speed_efficiency *= 0.5;
            $jt = max($jt, $jitter_tolerance);
          }
        }
        
        $target_speed = INF;
        if ($total_swinging > 0) {
          $target_speed = $jt * $speed_efficiency * $total_effective_traction / $total_swinging;
        }
        
        if ($total_swinging > $jt * $total_effective_traction) {
          if ($speed_efficiency > $min_speed_efficiency) {
            $speed_efficiency *= 0.7;
          }
          elseif ($speed < 1000) {
            $speed_efficiency *= 1.3;
          }
        }
        
        # But the speed shoudn't rise too much too quickly, since it would
        # make the convergence drop dramatically.
        $max_rise = 0.5;
        $speed = $speed + min($target_speed - $speed, $max_rise * $speed);
        
        # Apply forces.
        foreach ($nodes as $node_id => $node) {
          $dx = $nodes[$node_id]['dx'];
          $dy = $nodes[$node_id]['dy'];
          $prev_dx = $nodes[$node_id]['prev_dx'];
          $prev_dy = $nodes[$node_id]['prev_dy'];
          $mass = $nodes[$node_id]['mass'];
          
          $swinging = $mass * sqrt(($prev_dx - $dx) * ($prev_dx - $dx) + ($prev_dy - $dy) * ($prev_dy - $dy));
          $factor = $speed / (1.0 + sqrt($speed * $swinging));
          $nodes[$node_id]['x'] += $dx * $factor;
          $nodes[$node_id]['y'] += $dy * $factor;
        }
        
        return ['speed' => $speed, 'speed_efficiency' => $speed_efficiency];
  }
   
}
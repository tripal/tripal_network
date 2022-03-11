<?php 

abstract class TripalNetworkPlotEngine {
  
  protected $virdis_colors = [
    'rgb(68, 1, 84)',
    'rgb(71, 19, 101)',
    'rgb(72, 36, 117)',
    'rgb(70, 52, 128)',
    'rgb(65, 68, 135)',
    'rgb(59, 82, 139)',
    'rgb(53, 95, 141)',
    'rgb(47, 108, 142)',
    'rgb(42, 120, 142)',
    'rgb(37, 132, 142)',
    'rgb(33, 145, 140)',
    'rgb(30, 156, 137)',
    'rgb(34, 168, 132)',
    'rgb(47, 180, 124)',
    'rgb(68, 191, 112)',
    'rgb(94, 201, 98)',
    'rgb(122, 209, 81)',
    'rgb(155, 217, 60)',
    'rgb(189, 223, 38)',
    'rgb(223, 227, 24)',
    'rgb(253, 231, 37)',
  ];
  
  public function categoryColors($values) {
    
    $colors = [];
    $num_colors = count($this->virdis_colors);
    $unique_vals = array_unique($values);
    $num_values = count($unique_vals);
    $unique_vals_index = [];
    foreach ($unique_vals as $index => $value) {
      $unique_vals_index[$value] = $index;
    }
    
    foreach ($values as $value) {
      $i = $unique_vals_index[$value];
      $ci = floor($i * ($num_colors / $num_values));
      $colors[] = $this->virdis_colors[$ci];
    }
    
    return $colors;
  }
  
  public function numericColors($values) {
    $colors = [];
    $num_colors = count($this->virdis_colors);
    $min_value = INF;
    $max_value = -INF;
    
    $min_value = min($values);
    $max_value = max($values);
    foreach ($values as $value) {
      $ci = (($value - $min_value) / $max_value) * $num_colors;
      $colors[] = $this->virdis_colors[$ci];
    }
    return $colors;
  }
}
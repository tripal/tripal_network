<?php

/**
 *
 */
function tripal_network_viewer_page() {
  return theme('tripal_network_viewer', array());
}

/**
 *
 */
function tripal_network_viewer_ajax_retrieve() {


  // Get the fitering variables requested by the user.
  $network_id = $_GET['network_id'];
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : '';

  // Make sure we have required filters.
  if (!$network_id) {
    drupal_json_output($network_data);
    $network_id = 26;
  }

  // Get the list of edges and rows that match the query.
  $nodes = [];
  $edges = [];
  $params = [':network_id' => $network_id];

  // Get the edges from the network
  $select = "
    SELECT
      FS.feature_id as node1_id, FS.name as node1,
      FT.feature_id as node2_id, FT.name as node2,
      NE.network_edge_id
   ";
  $from = "
    FROM {network_edge} NE
      INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
      INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
      INNER JOIN {feature} FS on FS.feature_id = NNS.feature_id
      INNER JOIN {feature} FT on FT.feature_id = NNT.feature_id
  ";
  $where = "WHERE NE.network_id = :network_id";
  $order = "";
  $limit = "LIMIT 1000 OFFSET 0";


  if ($feature_id) {
    $where .= " AND (FS.feature_id = :feature_id OR FT.feature_id = :feature_id) ";
    $params[':feature_id'] = $feature_id;
  }

  $sql = $select . $from . $where . $order . $limit;

  // Execute the query and return the parsed results.
  $results = chado_query($sql, $params);
  $nodes = [];
  $edges = [];
  while ($record = $results->fetchObject()) {
    $nodes[$record->node1_id] = $record->node1;
    $nodes[$record->node2_id] = $record->node2;
    $edges[] = $record;
  }

  // Create the node trace
  $nodes_trace = [
    'mode'  => 'markers',
    'type' => 'scatter3d',
    'marker' => [
      'symbol' => 'circle',
      'color' => 'rgb(255, 255, 255)',
      'size' => 5,
      'opacity' => 0.9,
      'line' => [
        'width' => 2,
        'color' => 'rgb(100, 100, 100)',
      ],
    ],
    'hovertemplate'=> "%{text}",
    'name' => 'Node',
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];
  $node_coords = [];
  foreach ($nodes as $node_id => $node_name) {
    $x = mt_rand(0, 10);
    $y = mt_rand(0, 10);
    $z = mt_rand(0, 10);
    $node_coords[$node_id] = ['x' => $x, 'y' => $y, 'z' => $z];
    $nodes_trace['x'][] = $x;
    $nodes_trace['y'][] = $y;
    $nodes_trace['z'][] = $z;
    $nodes_trace['text'][] = $node_name;
  }

  $edges_trace = [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => 'rgb(175, 175, 175)',
      'width' => 1,
      'opacity' => 0.1,
    ],
    'name' => 'Edge',
    'hovertemplate'=> "%{text}",
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];
  foreach ($edges as $edge) {
    $node1 = $node_coords[$edge->node1_id];
    $node2 = $node_coords[$edge->node2_id];
    $edges_trace['x'][] = $node1['x'];
    $edges_trace['y'][] = $node1['y'];
    $edges_trace['z'][] = $node1['z'];

    $edges_trace['x'][] = $node2['x'];
    $edges_trace['y'][] = $node2['y'];
    $edges_trace['z'][] = $node2['z'];
    $edges_trace['text'][] = $edge->node1 . ' (-) ' . $edge->node2;
  }

  drupal_json_output([$nodes_trace, $edges_trace]);
}


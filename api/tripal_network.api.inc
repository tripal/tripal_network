<?php

/**
 * Retrieves the edges of a network.
 *
 * @param $network_id
 *   The network ID.
 * @param number $limit
 *   A number indicating the max number of edges to retrieve.
 * @param $offset
 *   A number indicating the offset in the edge list to retreive.
 *
 * @return
 *   An associative array of edges with the keys 'edges' and 'nodes'.
 */
function tripal_network_get_network_elements($network_id, $feature_id = NULL, $limit = 0, $offset = 0) {

  if ($offset and !$limit) {
    throw new Exception('tripal_network_get_edges: if an offset is provided a limit must also be provided.');
  }

  $params = [':network_id' => $network_id];

  // Get the edges from the network
  $sql = "
    SELECT
      FS.feature_id as source_feature_id, FS.name as source_name,
      FT.feature_id as target_feature_id, FT.name as target_name,
      NNS.network_node_id as source_node_id,
      NNT.network_node_id as target_node_id,
      NE.network_edge_id
    FROM {network_edge} NE
      INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
      INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
      INNER JOIN {feature} FS on FS.feature_id = NNS.feature_id
      INNER JOIN {feature} FT on FT.feature_id = NNT.feature_id
    WHERE NE.network_id = :network_id
  ";
  if ($feature_id) {
    $sql .= " AND (NNS.feature_id = :feature_id OR NNT.feature_id = :feature_id)";
    $params[':feature_id'] = $feature_id;
  }
  if ($limit) {
    $sql .= " LIMIT :limit OFFSET :offset";
    $params[':limit'] = $limit;
    $params[':offset'] = $offset;
  }

  // Execute the query and return the parsed results.
  $results = chado_query($sql, $params);
  $edges = [];
  $nodes = [];
  while ($record = $results->fetchAssoc()) {
    $edges[$record['network_edge_id']] = $record;
    $nodes[$record['source_feature_id']] = [
      'network_node_id' => $record['source_node_id'],
      'node_name' => $record['source_name'],
    ];
    $nodes[$record['target_feature_id']] = [
      'network_node_id' => $record['target_node_id'],
      'node_name' => $record['target_name'],
    ];
  }

  return [
    'network_id' => $network_id,
    'feature_id' => $feature_id,
    'limit' => $limit,
    'offset' => $offset,
    'edges' => $edges,
    'nodes' => $nodes
  ];
}

/**
 * A helper function that bins a set of properties.
 * @param array $props
 *   An associative array of edge properties as returned by the
 *   tripal_network_get_edgeprops function.
 * @param int $num_bins
 *   The total number of bins
 */
function _tripal_network_bin_by_props($props, $num_bins = INF, $abs = FALSE) {

  $bins = [];

  // Let's determine if the property is categorical, ordinal or quantitative.
  $is_ordinal = FALSE;
  $is_categorical = FALSE;
  $is_quantitative = FALSE;
  $num_numeric = 0;
  $num_floats = 0;
  $num_ints = 0;
  $num_strings = 0;
  $prop_values = [];

  // Count the number of types of values.
  foreach ($props as $network_edge_id => $value) {
    $value = trim($value);

    if (is_numeric($value)) {
      $num_numeric++;
      if (is_int($value)) {
        $num_ints++;
      }
      else {
        $num_floats++;
      }
    }
    else {
      // Exclude missing values.
      if (preg_match('/^(NA|NaN|null|None)$/i', $value)) {
        $props[$network_edge_id] = '';
        continue;
      }
      $num_strings++;
    }
    $prop_values[$value] = 1;
  }


  // Determine if the property is quantitative, ordinal or categorical.
  if ($num_strings > 0) {
    $is_categorical = TRUE;
  }
  else {
    if ($num_ints > 0 and $num_floats == 0 and $num_ints <= $num_bins) {
      $is_ordinal = TRUE;
    }
    else {
      $is_quantitative = TRUE;
    }
  }

  // For quantitative properties get min/max.
  $smallest = INF;
  $max = -INF;
  $min = INF;
  if ($is_quantitative) {
    foreach ($props as $network_edge_id => $value) {
      if (!$value) {
        continue;
      }
      if ($abs === TRUE) {
        $value = abs($value);
        $props[$network_edge_id] = $value;
      }
      if ($value > 0 and $value < 0.001 and $value < $smallest) {
        $smallest = $value;
      }
      if ($value > $max) {
        $max = $value;
      }
      if ($value < $min) {
        $min = $value;
      }
    }

    // If this data has extremely small numbers then transform.
    $needs_transform = FALSE;
    if ($smallest < 0.0001) {
      $needs_transform = TRUE;
      $max = -log10($smallest);
      $min = 0;
    }

    // Now do any log transform if needed, otherwise round the value
    // to the nearest 2 decimal places.
    foreach ($props as $network_edge_id => $value) {
      if (!$value) {
        $props[$network_edge_id] = $min;
        continue;
      }
      if ($min >= 0 and $needs_transform) {
        $props[$network_edge_id] = -log10($value);
      }
      else {
        $props[$network_edge_id] = round($value, 2);
      }
    }

    // Assign the value to a bin.
    if ($num_bins == INF) {
      foreach ($props as $network_edge_id => $value) {
        $bins[$network_edge_id] = $value;
      }
    }
    else {
      $bin_size = ($max - $min) / $num_bins;
      foreach ($props as $network_edge_id => $value) {
        $bins[$network_edge_id] = ($value - $min) / $bin_size;
      }
    }
  }

  $categories = [];
  if ($is_categorical or $is_ordinal) {
    if ($is_categorical) {
      $categories = array_keys($prop_values);
      asort($categories);
    }
    foreach ($props as $network_edge_id => $value) {
      $bins[$network_edge_id] = $value;
    }
  }

  return [
    'is_categorical' => $is_categorical,
    'is_ordinal' => $is_ordinal,
    'is_quantitative' => $is_quantitative,
    'bins' => $bins,
    'min' => $min,
    'max' => $max,
    'categories' => $categories
  ];
}

/**
 * Sorts nodes into bins using a node property
 *
 * This function accepts either feature IDs or network node IDs.  If
 * feature IDs are provided then the properties are returned indexed by
 * the feature ID. If a network node ID is provided then the properties are
 * returned indexed by the network node IDs.
 *
 * @param array $ids
 *   An array of feature IDs or network node IDs.
 * @param string $bin_term
 *   The name of the term that will used to layer the nodes into bins. It
 *   is in the format: db_name:accession. For example SWO:0000425.
 * @param string $color_term
 *   The name of the term that will be used to color the nodes. It is in
 *   the format: db_name:accession. For example OBI:0200130.
 */
function tripal_network_layer_network_edges($edge_ids, $bin_term = NULL, $color_term = NULL) {

  $virdis_colors = [
    'rgb(253, 231, 37)',
    'rgb(223, 227, 24)',
    'rgb(189, 223, 38)',
    'rgb(155, 217, 60)',
    'rgb(122, 209, 81)',
    'rgb(94, 201, 98)',
    'rgb(68, 191, 112)',
    'rgb(47, 180, 124)',
    'rgb(34, 168, 132)',
    'rgb(30, 156, 137)',
    'rgb(33, 145, 140)',
    'rgb(37, 132, 142)',
    'rgb(42, 120, 142)',
    'rgb(47, 108, 142)',
    'rgb(53, 95, 141)',
    'rgb(59, 82, 139)',
    'rgb(65, 68, 135)',
    'rgb(70, 52, 128)',
    'rgb(72, 36, 117)',
    'rgb(71, 19, 101)',
    'rgb(68, 1, 84)',
  ];
  $num_colors = count($virdis_colors);

  $colors = [];
  $z_props = tripal_network_get_edgeprops($edge_ids, $bin_term);
  $color_props = tripal_network_get_edgeprops($edge_ids, $color_term);

  //  For the similarity score, take the absolute value.
  $z_abs = FALSE;
  if ($bin_term == 'SWO:0000425') {
    $z_abs = TRUE;
  }
  $color_abs = FALSE;
  if ($bin_term == 'SWO:0000425') {
    $color_abs = TRUE;
  }

  $z = _tripal_network_bin_by_props($z_props, INF, $z_abs);
  $colors = _tripal_network_bin_by_props($color_props, 20, $color_abs);
  if ($colors['is_categorical'] === TRUE) {
    $csize = count($virdis_colors) / count($colors['categories']);
    foreach ($colors['bins'] as $network_edge_id => $bin) {
      $ci = array_search($bin, $colors['categories']);
      $ci = floor($ci * $csize);
      $colors['bins'][$network_edge_id] = $virdis_colors[$ci];
    }
  }
  else {
    foreach ($colors['bins'] as $network_edge_id => $bin) {
      // For the term 'rank' reverse order the colors.
      if ($color_term == 'NCIT:C48904') {
        $colors['bins'][$network_edge_id] = $virdis_colors[round($bin)];
      }
      else {
        $colors['bins'][$network_edge_id] = $virdis_colors[$num_colors - round($bin) - 1];
      }
    }
  }

  return [
    'z' => $z,
    'colors' => $colors
  ];
}

/**
 * Retreives properties of one or more networks.
 *
 * @param $ids
 *   An array of network IDs
 * @param  $term
 *   The name of the term in the format: db_name:accession. For example
 *   OBI:0200130.
 * @return array
 *   The list of properties for the given ids, indexed with the id.
 */
function tripal_network_get_networkprops($ids, $term) {
  $props = [];

  $term_split = preg_split('/:/', $term);
  if (!$term_split) {
    return $props;
  }


  $sql = "
    SELECT NP.network_id, NP.value
    FROM {networkprop} NP
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = NP.type_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE
      NP.network_id IN (:ids) AND
      DB.name = :db_name AND
      DBX.accession = :accession;
  ";
  $args = [
    ':ids' => $ids,
    ':db_name' => $term_split[0],
    ':accession' => $term_split[1],
  ];
  $results = chado_query($sql, $args);
  while ($property = $results->fetchObject()) {
    $props[$property->network_id] = $property->value;
  }
  return $props;
}

/**
 * Retreives properties of one or more nodes.
 *
 * This function accepts either feature IDs or network node IDs.  If
 * feature IDs are provided then the properties are returned indexed by
 * the feature ID. If a network node ID is provided then the properties are
 * returned indexed by the network node IDs.
 *
 * @param $ids
 *   An array of feature IDs or network node IDs.
 * @param $id_type
 *   The type of IDs either 'feature_id' or 'network_node_id',
 * @param $network_id
 *   The ID of the network.
 * @param  $term
 *   The name of the term in the format: db_name:accession. For example
 *   OBI:0200130.
 * @return array
 *   The list of properties for the given ids, indexed with the id.
 */
function tripal_network_get_nodeprops($ids, $id_type = 'feature_id', $network_id, $term) {
  $column = NULL;
  $props = [];

  // TODO: How to handle errors besides returning an empty array?
  if ($id_type != 'feature_id' and $id_type != 'network_node_id') {
    return $props;
  }
  $term_split = preg_split('/:/', $term);
  if (!$term_split) {
    return $props;
  }

  if ($id_type == 'feature_id') {
    $column = 'F.feature_id';
  }
  else {
    $column = 'NN.network_node_id';
  }


  $sql = "
    SELECT NN.network_node_id, F.feature_id, NNP.value
    FROM {network_nodeprop} NNP
      INNER JOIN {network_node} NN on NN.network_node_id = NNP.network_node_id
      INNER JOIN {feature} F on F.feature_id = NN.feature_id
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = NNP.type_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE
      NN.network_id = :network_id AND
      $column in (:ids) AND
      DB.name = :db_name AND
      DBX.accession = :accession;
  ";
  $args = [
    ':network_id' => $network_id,
    ':ids' => $ids,
    ':db_name' => $term_split[0],
    ':accession' => $term_split[1],
  ];
  $results = chado_query($sql, $args);
  while ($property = $results->fetchObject()) {
    $props[$property->$id_type] = $property->value;
  }
  return $props;
}

/**
 * Retreives properties of one or more edges.
 *
 * @param $ids
 *   An array of edge IDs
 * @param $id_type
 *   The type of IDs either 'feature_id' or 'network_node_id',
 * @param $network_id
 *   The ID of the network.
 * @param  $term
 *   The name of the term in the format: db_name:accession. For example
 *   OBI:0200130.
 * @return array
 *   The list of properties for the given ids, indexed with the id.
 */
function tripal_network_get_edgeprops($ids, $term) {
  $props = [];

  // TODO: How to handle errors besides returning an empty array?
  $term_split = preg_split('/:/', $term);
  if (!$term_split) {
    return $props;
  }

  $sql = "
    SELECT NEP.network_edge_id, NEP.value
    FROM {network_edgeprop} NEP
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = NEP.type_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE
      NEP.network_edge_id in (:ids) AND
      DB.name = :db_name AND
      DBX.accession = :accession;
  ";
  $args = [
    ':ids' => $ids,
    ':db_name' => $term_split[0],
    ':accession' => $term_split[1],
  ];
  $results = chado_query($sql, $args);
  while ($property = $results->fetchObject()) {
    $props[$property->network_edge_id] = $property->value;
  }
  return $props;
}


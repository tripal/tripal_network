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
function tripal_network_layer_edges($edge_ids, $layer_term) {

  // We want to take the absolute value of some property types.
  $abs = FALSE;
  if ($layer_term == 'SWO:0000425') {
    $abs = TRUE;
  }

  $term = chado_get_cvterm(['id' => $layer_term]);

  // Get the property used for layering for each edge.
  $props = tripal_network_get_edgeprops($edge_ids, $layer_term);

  // We only have 20 colors and we don't want to over run plotly, so
  // let's use 20 bins.
  $num_layers = 20;

  $item_layer = [];
  $layer_labels = [];
  $layers = [];
  $adj_values = [];

  // Let's determine if the property is categorical, ordinal or quantitative.
  $is_ordinal = FALSE;
  $is_categorical = FALSE;
  $is_quantitative = FALSE;
  $is_log10_transformed = FALSE;
  $is_abs = FALSE;
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
      if (preg_match('/^[01234567889]+$/', $value)) {
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
    if ($num_ints > 0 and $num_floats == 0 and $num_ints <= $num_layers) {
      $is_ordinal = TRUE;
    }
    else {
      $is_quantitative = TRUE;
    }
  }

  // Handle quantitiative data.
  $smallest = INF;
  $max = -INF;
  $min = INF;
  if ($is_quantitative) {

    // Get the min and max value and take the abs value if requested.
    foreach ($props as $network_edge_id => $value) {
      if (!$value) {
        continue;
      }
      if ($abs === TRUE) {
        $value = abs($value);
        $props[$network_edge_id] = $value;
        $is_abs = TRUE;
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
      $is_log10_transformed = TRUE;
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
    $bin_size = ($max - $min) / $num_layers;
    foreach ($props as $network_edge_id => $value) {
      $adj_values[$network_edge_id] = $value;
      $item_layer[$network_edge_id] = intval(floor(($value - $min) / $bin_size));
    }

    // Set the bin labels.
    for ($i = 0; $i <= 20; $i++){
      if ($num_floats > 0) {
        $layer_labels[] = strval(round($min + ($bin_size * $i), 2));
      }
      else {
        $layer_labels[] = strval(round($min + ($bin_size * $i), 0));
      }
      $layers[] = $i;
    }
  }

  // Handle categorical values.
  $categories = [];
  if ($is_categorical) {

    // Get the list of categories.
    $categories = array_keys($prop_values);
    $num_categories = count($categories);
    asort($categories);

    // Set the bin labels.
    if ($num_categories <= $num_layers) {

      // Use the index of the category to set it's bin.
      foreach ($props as $network_edge_id => $value) {
        $item_layer[$network_edge_id] = array_search($value, $categories);
      }


      for ($i = 0; $i < $num_categories; $i++){
        $layer_labels[] = $categories[$i];
        $layers[] = $i;
      }
    }
    else {
      foreach ($props as $network_edge_id => $value) {
        $item_layer[$network_edge_id] = 0;
      }

      for ($i = 0; $i < $num_layers; $i++){
        $layer_labels[] = 'Too many categories';
        $layers[] = 0;
      }
    }
  }

  // Handle ordinal data.
  if ($is_ordinal) {
    // Get the list of ordinal values.
    $ordinals = array_keys($prop_values);
    asort($ordinals);

    // Use the index of the ordinal to set it's bin.
    foreach ($props as $network_edge_id => $value) {
      $item_layer[$network_edge_id] = array_search($value, $ordinals);
    }

    // Set the bin labels.
    for ($i = 0; $i < count($ordinals); $i++){
      $layer_labels[] = strval($ordinals[$i]);
      $layers[] = $i;
    }
  }

  $title = ucfirst($term->name);
  if ($is_log10_transformed) {
    $title = '-log10 ' . $title;
  }
  if ($is_abs) {
    $title = 'AV ' . $title;
  }

  return [
    'num_floats' => $num_floats,
    'num_ints' => $num_ints,
    'num_numeric' => $num_numeric,
    'num_strings' => $num_strings,
    'is_categorical' => $is_categorical,
    'is_ordinal' => $is_ordinal,
    'is_quantitative' => $is_quantitative,
    'is_log10_transformed' => $is_log10_transformed,
    'is_abs' => $is_abs,
    'adj_values' => $adj_values,
    'item_layer' => $item_layer,
    'layers' => $layers,
    'layer_labels' => $layer_labels,
    'min' => $min,
    'max' => $max,
    'title' => $title,
  ];
}

/**
 *
 * @param unknown $edge_ids
 * @param unknown $color_term
 * @return string[]
 */
function tripal_network_color_edges($edge_ids, $color_term) {

  // The colros array that will be returned.
  $colors = [];


  // Hard-coded virdis colors
  $virdis_colors = [

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
  $num_colors = count($virdis_colors);

  // Get the layers for the color term as this wilh lelp us determine the
  // colors (one for each layer).
  $layers = tripal_network_layer_edges($edge_ids, $color_term);
  $num_layers = count($layers['layers']);

  // Convert the layer bin into a color
  $labels = [];
  foreach ($layers['item_layer'] as $network_edge_id => $layer) {
    $ci = floor($layer * ($num_colors / $num_layers));

    // Reverse the colors for the rank property.
    if ($color_term == 'NCIT:C48904'){
      $ci = ($num_colors - 1) - $ci;
    }
    $colors[$network_edge_id] = $virdis_colors[$ci];
    $labels[$virdis_colors[$ci]] = $layers['layer_labels'][$layer];
  }

  return [
    'item_color' => $colors,
    'labels' => $labels,
    'is_categorical' => $layers['is_categorical'],
    'is_quantitative' => $layers['is_quantitative'],
    'is_ordinal' => $layers['is_ordinal'],
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


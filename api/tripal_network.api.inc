<?php

function tripal_network_get_networks() {
  $params = [];
  $sql = "SELECT N.network_id FROM {network} N";
  if ($organism_id) {
    $sql .= " WHERE N.organism_id = :organism_id";
    $params[':organism_id'] = $organism_id;
  }
  $sql .= " ORDER BY N.name";
  
  $networks = [];
  $results = chado_query($sql, $params);
  while ($network = $results->fetchObject()) {
    $networks[] = chado_generate_var('network', ['network_id' => $network->network_id]);
  }
  return $networks;
}
/**
 * Retrieves the edges of a network.
 *
 * @return
 *   An associative array of edges with the keys 'edges' and 'nodes'.
 */
function tripal_network_get_network_elements($selection = []) {
  
  $organism_id = array_key_exists('organism_id', $selection) ? $selection['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $selection) ? $selection['network_id'] : NULL;
  $feature_ids = array_key_exists('feature_ids', $selection) ? $selection['feature_ids'] : [];
  $term_ids = array_key_exists('term_ids', $selection) ? $selection['term_ids'] : [];
  $filters = array_key_exists('filters', $selection) ? $selection['filters'] : [];

  $params = [];
  $params[':organism_id'] = $organism_id;

  // Get the edges from the network
  $sql = "
    SELECT
      FS.feature_id as source_feature_id, FS.name as source_name,
      FT.feature_id as target_feature_id, FT.name as target_name,
      NNS.network_node_id as source_node_id,
      NNT.network_node_id as target_node_id,
      NE.network_edge_id
    FROM {network_edge} NE
      INNER JOIN {network} N on N.network_id = NE.network_id
      INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
      INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
      INNER JOIN {network_feature} NFS on NFS.network_node_id = NNS.network_node_id
      INNER JOIN {network_feature} NFT on NFT.network_node_id = NNT.network_node_id
      INNER JOIN {feature} FS on FS.feature_id = NFS.feature_id
      INNER JOIN {feature} FT on FT.feature_id = NFT.feature_id
  ";
  $where = " WHERE N.organism_id = :organism_id ";

  if ($network_id) {
    $where .= " AND NE.network_id = :network_id";
    $params[':network_id'] = $network_id;
  }
  if (is_array($feature_ids) and count($feature_ids) > 0) {
    $where .= " AND (FS.feature_id IN (:feature_ids) OR FT.feature_id IN (:feature_ids)) ";
    $params[':feature_ids'] = $feature_ids;
  }
  if (is_array($term_ids) and count($term_ids) > 0) {
    $sql .= "  LEFT JOIN {feature_cvterm} FCVTS on FCVTS.feature_id = FS.feature_id ";
    $sql .= "  LEFT JOIN {feature_cvterm} FCVTT on FCVTT.feature_id = FT.feature_id ";
    $where .= " AND (FCVTS.cvterm_id IN (:term_ids) OR FCVTT.cvterm_id IN (:term_ids)) ";
    $params[':term_ids'] = $term_ids;
  }
  
  $sql = $sql . $where;
  
  // Now apply the filters
  foreach ($filters as $fi => $filter) {    
    if (array_key_exists('limit_by', $filter) and $filter['limit_by']) {
      $limit_term = chado_get_cvterm(['id' => $filter['limit_by']]); 
      $sql = "
        SELECT TBL${fi}.* 
        FROM ($sql) as TBL${fi}
          INNER JOIN {network_edgeprop} NEP${fi} on NEP${fi}.network_edge_id = TBL${fi}.network_edge_id              
        WHERE NEP${fi}.type_id = :fi_${fi}_cvterm_id  
      ";
      
      # Add min and max limits.
      if (array_key_exists('max_value', $filter) and is_numeric($filter['max_value'])) {
        $sql .= " AND NEP${fi}.value_num <= :${fi}_max_value";
        $params[":${fi}_max_value"] = $filter['max_value'];
      }
      if (array_key_exists('min_value', $filter) and is_numeric($filter['min_value'])) {
        $sql .= " AND NEP${fi}.value_num >= :${fi}_min_value";
        $params[":${fi}_min_value"] = $filter['min_value'];
      }
      
      if (array_key_exists('categories', $filter) and !empty($filter['categories'])) {
        $sql .= " AND NEP${fi}.value IN (:${fi}_categories)";
        $params[":${fi}_categories"] = $filter['categories'];
      }
      
      $order_by = 'ASC';
      if (array_key_exists('order_by', $filter) and $filter['order_by'] == 'DESC') {
        $order_by = 'DESC';
      }
      
      if (array_key_exists('use_abs', $filter) and $filter['use_abs'] == 1) {
        $sql .= " ORDER BY ABS(NEP${fi}.value_num) $order_by";
      }
      else {
        $sql .= " ORDER BY NEP${fi}.value_num $order_by";
      }
      $params[":fi_${fi}_cvterm_id"] = $limit_term->cvterm_id;
      
    } 
    if (array_key_exists('limit', $filter) and $filter['limit'] > 0) {
      $sql .= " LIMIT :${fi}_limit OFFSET 0";
      $params[":${fi}_limit"] = $filter['limit'];
    }
  }

  // Execute the query and return the parsed results.
  $results = chado_query($sql, $params);
  $edges = [];
  $nodes = [];
  while ($record = $results->fetchAssoc()) {
    $edges[$record['network_edge_id']] = $record;
    $nodes[$record['source_node_id']] = [
      'feature_id' => $record['source_feature_id'],
      'node_name' => $record['source_name'],
    ];
    $nodes[$record['target_node_id']] = [
      'feature_id' => $record['target_feature_id'],
      'node_name' => $record['target_name'],
    ];
  }

  return [
    'organism_id' => $organism_id,
    'network_id' => $network_id,
    'feature_ids' => $feature_ids,
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
function tripal_network_layer_edges($organism_id, $network_id, $edge_ids, $layer_term) {
  
  if (empty($edge_ids)) {
    return NULL;
  }

  // We want to take the absolute value of some property types.
  $abs = FALSE;
  if ($layer_term == 'SWO:0000425') {
    $abs = TRUE;
  }

  $term = chado_get_cvterm(['id' => $layer_term]);

  // Get the property used for layering for each edge.
  $props = tripal_network_get_edgeprops($edge_ids, $layer_term);

  // Get the
  $attribute = tripal_network_get_attributes($organism_id, $network_id, $layer_term);

  $data_class = NULL;
  $data_type = NULL;
  $smallest = INF;
  $max = -INF;
  $min = INF;
  $categories = [];
  $num_categories = 0;
  foreach (array_values($attribute)[0]['properties'] as $property) {
    if ($property['term'] == 'NCIT:C25161') {
      if ($property['value_name'] == 'Quantitative Variable') {
        $data_class = 'quantitative';
      }
      if ($property['value_name'] == 'Qualitative Variable') {
        $data_class = 'categorical';
      }
      if ($property['value_name'] == 'Ordinal Scale') {
        $data_class = 'ordinal';
      }
    }
    if ($property['term'] == 'NCIT:C25570') {
      $min = $property['value_num'];
    }
    if ($property['term'] == 'NCIT:C25564') {
      $max = $property['value_num'];
    }
    if ($property['term'] == 'NCIT:C114464') {
      $categories[] = $property['value'];
      $num_categories = count($categories);
    }
    if ($property['term'] == 'NCIT:C42645') {
      $data_type = $property['value_name'];
    }
  }

  // We only have 20 colors and we don't want to over run plotly, so
  // let's use 20 bins.
  $num_layers = 20;

  $item_layer = [];
  $layer_labels = [];
  $layers = [];
  $adj_values = [];
  $is_log10_transformed = FALSE;
  $is_abs = FALSE;

  // Reset the min/max if we need to take the absolute value.
  if ($abs == TRUE) {
    $max = -INF;
    $min = INF;
  }

  // Handle quantitiative data.
  if ($data_class == 'quantitative') {

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
      if ($data_type == 'Float') {
        $layer_labels[] = strval(round($min + ($bin_size * $i), 2));
      }
      if ($data_type == 'Integer') {
        $layer_labels[] = strval(round($min + ($bin_size * $i), 0));
      }
      $layers[] = $i;
    }
  }

  // Handle categorical values.
  if ($data_class == 'categorical') {

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


  $title = ucfirst($term->name);
  if ($is_log10_transformed) {
    $title = '-log10 ' . $title;
  }
  if ($is_abs) {
    $title = 'Absolute Value  ' . $title;
  }

  return [
    'data_class' => $data_class,
    'data_type' => $data_type,
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
function tripal_network_color_edges($organism_id, $network_id, $edge_ids, $color_term) {
  
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
  $layers = tripal_network_layer_edges($organism_id, $network_id, $edge_ids, $color_term);
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
    'data_class' => $layers['data_class'],
    'data_type' => $layers['data_type'],
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
    if ($property->value and !$property->value_num) {
      $props[$property->network_id] = $property->value;
    }
    elseif (!$property->value and $property->value_num) {
      $props[$property->network_id] = $property->value_num;
    }
  }
  return $props;
}

/**
 * Retreives one or more nodes.
 *
 * This function accepts either feature IDs or network node IDs.
 *
 * @param $ids
 *   An array of feature IDs or network node IDs.
 * @param $id_type
 *   The type of IDs either 'feature_id' or 'network_node_id',
 */
function tripal_network_get_nodes($ids, $id_type = 'network_node_id') {  
  $nodes = [];
    
  if ($id_type == 'feature_id') {
    $column = 'F.feature_id';
  }
  else {
    $column = 'NN.network_node_id';
  }
   
  $sql = "
    SELECT NN.network_node_id, F.*
    FROM {network_node} NN
      INNER JOIN {network_feature} NF on NF.network_node_id = NN.network_node_id
      INNER JOIN {feature} F on F.feature_id = NF.feature_id
    WHERE
      $column in (:ids) 
  ";
  $args = [':ids' => $ids];
  $results = chado_query($sql, $args);
  while ($node = $results->fetchObject()) {
    $nodes[$node->$id_type] = $node;
  }
  return $nodes;
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
function tripal_network_get_nodeprops($ids, $id_type = 'network_node_id', $term, $organism_id = NULL, $network_id = NULL) {
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

  $args = [
    ':ids' => $ids,
    ':db_name' => $term_split[0],
    ':accession' => $term_split[1],
  ];
  $sql = "
    SELECT NN.network_node_id, F.feature_id, NNP.value
    FROM {network_nodeprop} NNP
      INNER JOIN {network_node} NN on NN.network_node_id = NNP.network_node_id
      INNER JOIN {network_feature} NF on NF.network_node_id = NN.network_node_id
      INNER JOIN {network} N on N.network_id = NN.network_id
      INNER JOIN {feature} F on F.feature_id = NF.feature_id
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = NNP.type_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE
      $column in (:ids) AND
      DB.name = :db_name AND
      DBX.accession = :accession
  ";
  if ($network_id) {
    $sql .= " AND NN.network_id = :network_id ";
    $args['network_id'] = $network_id;
  }
  if ($organism_id) {
    $sql .= " AND N.organism_id = :organism_id ";
    $args['organism_id'] = $organism_id;
  }

  $results = chado_query($sql, $args);
  while ($property = $results->fetchObject()) {
    if ($property->value and !$property->value_num) {
      $props[$property->$id_type] = $property->value;
    }
    elseif (!$property->value and $property->value_num) {
      $props[$property->$id_type] = $property->value_num;
    }
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
    SELECT NEP.network_edge_id, NEP.value, NEP.value_num
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
    if ($property->value and !$property->value_num) {
      $props[$property->network_edge_id] = $property->value;
    }
    elseif (!$property->value and $property->value_num) {
      $props[$property->network_edge_id] = $property->value_num;
    }
  }
  return $props;
}


/**
 * Retrieves all infromation about a network attribute.
 *
 * @param integer $network_id
 *   The network ID for which the attributes should be retreived.
 * @param string $term_id
 *   The ID of a controlled voabulary term in the form DB:ACCESSION. If not
 *   provided then all attributes are returend. If included only the specified
 *   atttribute is returned.
 */
function tripal_network_get_attributes($organism_id = NULL, $network_id = NULL, $attr_id = NULL) {

  $attr_term = chado_get_cvterm(['id' => $attr_id]);
  $args = [];

  // Get all the information about this attribute.
  $sql = "
      SELECT
        NA.network_attr_id, CVTD.name as domain,
        CVTA.name as attr_name, DBA.name as attr_db_name, DBXA.accession as attr_accession,
        CVTP.name as prop_name, CVTA.definition as attr_desc,
        DBP.name as prop_db_name, DBXP.accession as prop_accession,
        NAP.value, NAP.value_num, CVTV.name as value_name, NAP.rank
      FROM {network_attr} NA
        INNER JOIN {network} N on N.network_id = NA.network_id
        INNER JOIN {cvterm} CVTD on CVTD.cvterm_id = NA.domain_id
        INNER JOIN {cvterm} CVTA on CVTA.cvterm_id = NA.attr_id
        INNER JOIN {network_attrprop} NAP on NAP.network_attr_id = NA.network_attr_id
        INNER JOIN {cvterm} CVTP on CVTP.cvterm_id = NAP.type_id
        INNER JOIN {dbxref} DBXP on DBXP.dbxref_id = CVTP.dbxref_id
        INNER JOIN {dbxref} DBXA on DBXA.dbxref_id = CVTA.dbxref_id
        INNER JOIN {db} DBP on DBP.db_id = DBXP.db_id
        INNER JOIN {db} DBA on DBA.db_id = DBXA.db_id
        LEFT JOIN {cvterm} CVTV on CVTV.cvterm_id = NAP.value_id
      WHERE 1=1
    ";
  if ($network_id) {
    $sql .= " AND NA.network_id = :network_id";
    $args[':network_id'] = $network_id;
  }
  if ($organism_id) {
    $sql .= " AND N.organism_id = :organism_id";
    $args[':organism_id'] = $organism_id;
  }
  if ($attr_id) {
    $sql .= " AND NA.attr_id = :attr_id";
    $args[':attr_id'] = $attr_term->cvterm_id;
  }

  // Store the attributes in a handy array keyed by attribute.
  $attributes = chado_query($sql, $args);
  $response = [];
  while ($attribute = $attributes->fetchObject()) {
    if (!array_key_exists($attribute->network_attr_id, $response)) {
      $response[$attribute->network_attr_id] = [
        'name' => $attribute->attr_name,
        'desc' => $attribute->attr_desc,
        'domain' => $attribute->domain,
        'term' => $attribute->attr_db_name . ':' . $attribute->attr_accession,
        'properties' => [],
      ];
    }
    $response[$attribute->network_attr_id]['properties'][] = [
      'prop_name'=> $attribute->prop_name,
      'value' => $attribute->value,
      'value_num' => $attribute->value_num,
      'value_name' => $attribute->value_name,
      'term' => $attribute->prop_db_name . ':' . $attribute->prop_accession,
    ];
  }
  return $response;
}
/**
 * Returns an options array of attributes for a given network.
 *
 * @param integer $network_id
 *   The network ID for which attributes should be retreived.
 * @param boolean $quantitative
 *   Set to TRUE to retrieve quantitative attributes.
 * @param boolean $categorical
 *   set to TRUE to retrieve categorical attributes.
 * @param boolean $ordinal
 *   Set to TRUE to retrieve ordinal attributes.
 * @param boolean $annotation
 *   Set to TRUE to retrieve annotation attributes.
 *
 * @return
 *   An array of attributes indexed by the term ID and the value the name.
 */
function tripal_network_get_attribute_options($organism_id=NULL, $network_id=NULL, $quantitative = TRUE,
  $categorical = TRUE, $ordinal = TRUE, $annotation = TRUE) {

  $edge_term = chado_get_cvterm(['id' => 'NCIT:C75923']);
  $data_class_term = chado_get_cvterm(['id' => 'NCIT:C25161']);
  $quantitative_term = chado_get_cvterm(['id' => 'NCIT:C142658']);
  $categorical_term = chado_get_cvterm(['id' => 'NCIT:C142657']);
  $ordinal_term = chado_get_cvterm(['id' => 'NCIT:C47797']);
  $annotation_term = chado_get_cvterm(['id' => 'NCIT:C44272']);

  $class_ids = [];
  if ($quantitative) {
    $class_ids[] = $quantitative_term->cvterm_id;
  }
  if ($categorical) {
    $class_ids[] = $categorical_term->cvterm_id;
  }
  if ($ordinal) {
    $class_ids[] = $ordinal_term->cvterm_id;
  }
  if ($annotation) {
    $class_ids[] = $annotation_term->cvterm_id;
  }

  $sql = "
    SELECT CVT.cvterm_id, CVT.name, DBX.accession, DB.name as db_name
    FROM {network_attr} NA
      INNER JOIN {network} N on N.network_id = NA.network_id
      INNER JOIN {network_attrprop} NAP on NAP.network_attr_id = NA.network_attr_id
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = NA.attr_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
    WHERE
      NA.domain_id = :domain_id AND
      NAP.type_id = :class_type AND
      NAP.value_id IN (:class_ids)
  ";
  $args = [
    ':domain_id' => $edge_term->cvterm_id,
    ':class_type' => $data_class_term->cvterm_id,
    ':class_ids' => $class_ids
  ];
  if ($organism_id) {
    $sql .= " AND N.organism_id = :organism_id";
    $args[':organism_id'] = $organism_id;
  }
  if ($network_id) {
    $sql .= " AND NA.network_id = :network_id";
    $args[':network_id'] = $network_id;
  }
  $results = chado_query($sql, $args);
  $props[] = '--Select--';
  while ($record = $results->fetchObject()) {
    $term = $record->db_name . ':' . $record->accession;
    $props[$term] = ucfirst($record->name);
  }
  return $props;
}

/**
 * Calcualtes the degree of each node in a network.
 * 
 * Adds a 'degree' attrbitue to the 'nodes' element of the $network_items.
 * 
 * @param $network_items
 *   An array of elements selected using the tripal_network_get_network_elements
 *   function.
 */
function tripal_network_calculate_degree(&$network_items) {
  
  $edges = $network_items['edges'];
  $nodes = $network_items['nodes'];
  
  // Add the degree attrbute to the node.
  foreach ($nodes as $node_id => $node) {
    $network_items['nodes'][$node_id]['degree'] = 1;
  }
  
  foreach ($edges as $edge) {
    $node1_id = $edge['source_node_id'];
    $node2_id = $edge['target_node_id'];
    
    $network_items['nodes'][$node1_id]['degree']++;
    $network_items['nodes'][$node2_id]['degree']++;
  }
} 
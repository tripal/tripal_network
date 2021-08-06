<?php

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
 *   An array of edge IDs or network node IDs.
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


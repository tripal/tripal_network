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
    SELECT 
      NN.network_node_id, F.*, 
      DB.name as type_db_name, DBX.accession as type_accession
    FROM {network_node} NN
      INNER JOIN {network_feature} NF on NF.network_node_id = NN.network_node_id
      INNER JOIN {feature} F on F.feature_id = NF.feature_id
      INNER JOIN {cvterm} CVT on F.type_id = CVT.cvterm_id
      INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
      INNER JOIN {db} DB on DB.db_id = DBX.db_id
      INNER JOIN {cv} CV on CV.cv_id = CVT.cv_id
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
<?php

class TripalNetwork {
  
  private $filters = [
    0 => [
      'limit' => 500,
      'limit_by' => 'NCIT:C48834',
      'order_by' => 'DESC',
      'use_abs' => 1,
    ],  
  ];
  
  private $selection = [
    'organism_id' => NULL,    
    'network_ids' => [],
    'feature_ids' => [],
    'term_ids' => [],
  ];
   
  private $node_attributes = [];
  private $edge_attributes = [];
  
  private $network_ids = [];
  private $nodes = [];  
  private $edges = [];
  
  /**
   *  
   */
  public function __construct($selection) {  
    
    $this->selection['organism_id'] = array_key_exists('organism_id', $selection) ? $selection['organism_id'] : NULL;    
    $this->selection['network_ids'] = (array_key_exists('network_ids', $selection) and is_array($selection['network_ids'])) ? $selection['network_ids'] : [];
    $this->selection['feature_ids'] = (array_key_exists('feature_ids', $selection)  and is_array($selection['feature_ids'])) ? $selection['feature_ids'] : [];
    $this->selection['term_ids'] = (array_key_exists('term_ids', $selection)  and is_array($selection['term_ids'])) ? $selection['term_ids'] : [];    
  }
  
  public function getSelection() {
    return $this->selection;
  }
    
  /**
   *
   */
  public function getFilters() {
    return $this->filters;
  }
  
  public function getEdges() {
    return $this->edges;
  }
  public function getNodes() {
    return $this->nodes;
  }
  public function getEdgeAttrs() {
    return $this->edge_attributes;    
  }
  public function getNodeAttrs() {
    return $this->node_attributes;
  }
  public function getNetworkIDs() {
    return $this->network_ids;
  }
  
  /**
   * 
   */
  public function addFilter($filter) {
    
    // TODO: Make sure the filter matches the expected values.
    
    $this->filters[] = $filter;
    
//     'limit' => $limit,
//     'limit_by' => $limit_by,
//     'min_value' => $min_value,
//     'max_value' => $max_value,
//     'order_by' => $order_by,
//     'use_abs' => $use_abs,
//     'categories' => $categories,
  }
  public function resetFilters() {
    $this->filters = [];
  }
  
  /**
   * 
   */
  public function deleteFilter($index) {
    unset($this->filters[$index]);
  }
 
  /**
   * 
   */
  public function loadNetwork() {
    
    $organism_id = $this->selection['organism_id'];
    $network_ids = $this->selection['network_ids'];
    $feature_ids = $this->selection['feature_ids'];
    $term_ids = $this->selection['term_ids'];
    $filters = $this->filters;
    
    $params = [];
    $params[':organism_id'] = $organism_id;
    
    // Get the edges from the network
    $sql = "
    SELECT
      FS.feature_id as source_feature_id, FS.name as source_name,
      FT.feature_id as target_feature_id, FT.name as target_name,
      NNS.network_node_id as source_node_id,
      NNT.network_node_id as target_node_id,
      NE.network_edge_id,
      N.network_id
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
    
    if (count($network_ids) > 0 ) {
      $where .= " AND NE.network_id IN (:network_ids)";         
      $params[':network_ids'] = $network_ids;
    }
    if (count($feature_ids) > 0) {
      $where .= " AND (FS.feature_id IN (:feature_ids) OR FT.feature_id IN (:feature_ids)) ";
      $params[':feature_ids'] = $feature_ids;
    }
    if (count($term_ids) > 0) {
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
    $network_ids = [];
    $edges = [];
    $nodes = [];
    while ($record = $results->fetchAssoc()) {
      $network_ids[$record['network_id']] = $record['network_id'];
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
    
    $this->network_ids = $network_ids;
    $this->edges = $edges;
    $this->nodes = $nodes;
    
    $this->getEdgeAttributes();
    //$this->getNodeAttributes();
  } 
  
  /**
   * 
   */
  private function getEdgeAttributes() {
    
    if (count($this->edges) == 0) {
      return;
    }
    
    $quantitative_term = chado_get_cvterm(['id' => 'NCIT:C142658']);
    $categorical_term = chado_get_cvterm(['id' => 'NCIT:C142657']);
    $ordinal_term = chado_get_cvterm(['id' => 'NCIT:C47797']);
    
    $sql = "
      SELECT DISTINCT NE.network_id, NEP.type_id
      FROM {network_edgeprop} NEP 
        INNER JOIN {network_edge} NE on NE.network_edge_id = NEP.network_edge_id
        INNER JOIN {network_attr} NA on NA.attr_id = NEP.type_id and NA.network_id = NE.network_id
      WHERE NE.network_edge_id IN (:edge_ids)
    ";
    $params[':edge_ids'] = array_keys($this->edges);
    $results = chado_query($sql, $params);
    while ($attr_type = $results->fetchObject()) {
      $options = ['include_fk' => ['attr_id' => ['cv_id' => TRUE, 'dbxref_id' => ['db_id' => TRUE]]]];
      $select = ['attr_id' => $attr_type->type_id, 'network_id' => $attr_type->network_id];
      $attr = chado_generate_var('network_attr', $select, $options);
      $options = [
        'include_fk' => ['type_id' => ['cv_id' => TRUE, 'dbxref_id' => ['db_id' => TRUE]], 'value_id' => TRUE],
        'return_array' => TRUE,
      ];
      $attr = chado_expand_var($attr, 'table', 'network_attrprop', $options);      
      $term_id = $attr->attr_id->dbxref_id->db_id->name . ":" . $attr->attr_id->dbxref_id->accession;
      $data_type = $categorical_term;
      foreach ($attr->network_attrprop as $attrprop) {
        if ($attrprop->value_id == $quantitative_term->cvterm_id) {
          $data_type = $quantitative_term;
        }
        if ($attrprop->value_id == $ordinal_term->cvterm_id) {
          $data_type = $ordinal_term;
        }
      }
      $this->edge_attributes[$term_id][$attr_type->network_id] = [
        'attr' => $attr,
        'type' => $data_type,
      ];
    }    
  }
  
  /**
   *
   */
  private function getNodeAttributes() {
    
    if (count($this->nodes) == 0) {
      return;
    }
    
    $quantitative_term = chado_get_cvterm(['id' => 'NCIT:C142658']);
    $categorical_term = chado_get_cvterm(['id' => 'NCIT:C142657']);
    $ordinal_term = chado_get_cvterm(['id' => 'NCIT:C47797']);
    
    $sql = "
      SELECT DISTINCT NE.network_id, NNP.type_id
      FROM {network_nodeprop} NNP
        INNER JOIN {network_node} NN on NN.network_node_id = NNP.network_node_id
        INNER JOIN {network_attr} NA on NA.attr_id = NNP.type_id and NA.network_id = NN.network_id
      WHERE NN.network_node_id IN (:node_ids)
    ";
    $params[':node_ids'] = array_keys($this->nodes);
    $results = chado_query($sql, $params);
    while ($attr_type = $results->fetchObject()) {
      $options = ['include_fk' => ['attr_id' => ['cv_id' => TRUE, 'dbxref_id' => ['db_id' => TRUE]]]];
      $select = ['attr_id' => $attr_type->type_id, 'network_id' => $attr_type->network_id];
      $attr = chado_generate_var('network_attr', $select, $options);
      $options = [
        'include_fk' => ['type_id' => ['cv_id' => TRUE, 'dbxref_id' => ['db_id' => TRUE]], 'value_id' => TRUE],
        'return_array' => TRUE,
      ];
      $attr = chado_expand_var($attr, 'table', 'network_attrprop', $options);
      $data_type = $categorical_term;
      foreach ($attr->network_attrprop as $attrprop) {
        if ($attrprop->value_id == $quantitative_term->cvterm_id) {
          $data_type = $quantitative_term;
        }
        if ($attrprop->value_id == $ordinal_term->cvterm_id) {
          $data_type = $ordinal_term;
        }
      }
      $term_id = $attr->type_id->dbxref_id->db_id->name . ":" . $attr->type_id->dbxref_id->accession;
      $this->node_attributes[$term_id][$attr_type->network_id] = [
        'attr' => $attr,
        'type' => $data_type,
      ];
    }  
  }
  
  /**
   * 
   */
  public function retrieveNodeProp($term_id) {    
    $props = [];
    
    $term_split = preg_split('/:/', $term_id);
    if (!$term_split) {
      return $props;
    }
        
    $sql = "
      SELECT NNP.network_node_id, NNP.value, NNP.value_num
      FROM {network_nodeprop} NNP
        INNER JOIN {cvterm} CVT on CVT.cvterm_id = NNP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE
        NNP.network_node_id in (:ids) AND
        DB.name = :db_name AND
        DBX.accession = :accession
    ";  
    $args = [
      ':ids' => array_keys($this->nodes),
      ':db_name' => $term_split[0],
      ':accession' => $term_split[1],
    ];
    $results = chado_query($sql, $args);
    $num_found = 0;
    while ($property = $results->fetchObject()) {
      $num_found++;
      if ($property->value and !$property->value_num) {
        $this->nodes[$property->network_node_id][$term_id] = $property->value;
      }
      elseif (!$property->value and $property->value_num) {
        $this->nodes[$property->network_node_id][$term_id] = $property->value_num;
      }
    }   
    return $num_found;
  }
  
  /**
   * 
   */
  public function retrieveEdgeProp($term_id) {
    $props = [];
    
    $term_split = preg_split('/:/', $term_id);
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
      ':ids' => array_keys($this->edges),
      ':db_name' => $term_split[0],
      ':accession' => $term_split[1],
    ];
    $results = chado_query($sql, $args);
    $num_found = 0;
    while ($property = $results->fetchObject()) {
      $num_found++;
      if ($property->value and !$property->value_num) {
        $this->edges[$property->network_edge_id][$term_id] = $property->value;
      }
      elseif (!$property->value and $property->value_num) {
        $this->edges[$property->network_edge_id][$term_id] = $property->value_num;
      }
    }
    return $num_found;
  }
  
  /**
   *
   */
  public function addNodeProp($node_id, $term_id, $value) {
    $this->nodes[$node_id][$term_id] = $value;
  }
  
  /**
   *
   */
  public function addEdgeProp($edge_id, $term_id, $value) {
    $this->edges[$edge_id][$term_id] = $value;
  }
  
  /**
   * 
   */
  public function calculateDegree() {
    
    // Add the degree attrbute to the node.
    foreach ($this->nodes as $node_id => $node) {
      $this->nodes[$node_id]['OBI:0200130'] = 1;
    }
    
    foreach ($this->edges as $edge) {
      $node1_id = $edge['source_node_id'];
      $node2_id = $edge['target_node_id'];      
      $this->nodes[$node1_id]['OBI:0200130']++;
      $this->nodes[$node2_id]['OBI:0200130']++;
    }    
    
    // Add the degree attribute as if it were in the databas.
    $domain_term = chado_get_cvterm(['id' => 'NCIT:C75907']);
    $term = chado_get_cvterm(['id' => 'OBI:0200130']);
    $this->node_attributes['OBI:0200130'][0] = [
      'attr' => (object)[
        'network_id' => 0,
        'attr_id' => $term,
        'domain_id' => $domain_term
      ]      
    ];   
  }
  
  public function calculateLocalCC() {
     $neighbors = [];
     foreach ($this->edges as $edge_id => $edge_details) {
       $source_node_id = $edge_details['source_node_id'];
       $target_node_id = $edge_details['target_node_id'];
       $neighbors[$source_node_id][] = $edge_details[$target_node_id];
       $neighbors[$target_node_id][] = $edge_details[$source_node_id];
     }
     
     foreach ($neighbors as $node_id => $firstn) {
       $k = count($firstn);
       $this->nodes[$node_id]['local:local_clustering_coefficient'] = $k / ($k*($k-1));
     }
     
     // Add the degree attribute as if it were in the databas.
     $domain_term = chado_get_cvterm(['id' => 'NCIT:C75907']);
     $term = chado_get_cvterm(['id' => 'local:local_clustering_coefficient']);
     $this->node_attributes['local:local_clustering_coefficient'][0] = [
       'attr' => (object)[
         'network_id' => 0,
         'attr_id' => $term,
         'domain_id' => $domain_term         
       ]      
     ];  
  }
}
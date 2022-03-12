<?php 

class TripalNetworkEdge {
  
  protected $id = NULL;
  
  protected $attributes = NULL;
  
  protected $attribute_values = [];
  
  protected $edge_name;
  
  protected $net_name;
  
  protected $details;
  
  protected $quantitative_term = NULL;
  
  protected $categorical_term = NULL;
  
  protected $ordinal_term = NULL;
  
  /**
   *
   */
  public function __construct($edge_id) {
    
    $this->quantitative_term = chado_get_cvterm(['id' => 'NCIT:C142658']);
    $this->categorical_term = chado_get_cvterm(['id' => 'NCIT:C142657']);
    $this->ordinal_term = chado_get_cvterm(['id' => 'NCIT:C47797']);
    
    $sql = "
      SELECT 
        N.network_id,
        N.name as net_name,
        FS.feature_id as source_id, 
        FS.name as source_name, 
        FT.feature_id as target_id,
        FT.name as target_name
      FROM {network_edge} NE
        INNER JOIN {network} N on NE.network_id = N.network_id
        INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
        INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
        INNER JOIN {network_feature} NFS on NNS.network_node_id = NFS.network_node_id
        INNER JOIN {network_feature} NFT on NNT.network_node_id = NFT.network_node_id
        INNER JOIN {feature} FS on FS.feature_id = NFS.feature_id
        INNER JOIN {feature} FT on FT.feature_id = NFT.feature_id
     WHERE NE.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);
    
    $this->id = $edge_id;
    $this->details = $results->fetchAssoc();
    $this->edge_name = $this->details['source_name'] . ' (-) ' . $this->details['target_name'];
    $this->net_name = $this->details['net_name'];
  }
  
  /**
   *
   */
  public function id() {
    return $this->id;
  }
  
  /**
   *
   */
  public function details() {
    return $this->details;
  }
  
  /**
   *
   */
  public function edgeName() {
    return $this->edge_name;
  }
  
  /**
   *
   */
  public function netName() {
    return $this->net_name;
  }
  
  /**
   *
   */
  public function attributes() {
    if (empty($this->attributes)) {
      $sql = "
        SELECT DISTINCT NE.network_id, NEP.type_id
        FROM {network_edgeprop} NEP
          INNER JOIN {network_edge} NE on NE.network_edge_id = NEP.network_edge_id
          INNER JOIN {network_attr} NA on NA.attr_id = NEP.type_id and NA.network_id = NE.network_id
        WHERE NE.network_edge_id = :edge_id
      ";
      $params[':edge_id'] = $this->id;
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
        $attr = chado_expand_var($attr, 'field', 'network_attrprop.value');
        $term_id = $attr->attr_id->dbxref_id->db_id->name . ":" . $attr->attr_id->dbxref_id->accession;
        $data_type = $this->categorical_term;
        foreach ($attr->network_attrprop as $attrprop) {
          if ($attrprop->value_id == $this->quantitative_term->cvterm_id) {
            $data_type = $this->quantitative_term;
          }
          if ($attrprop->value_id == $this->ordinal_term->cvterm_id) {
            $data_type = $this->ordinal_term;
          }
        }
        $this->attributes[$term_id] = [
          'attr' => $attr,
          'type' => $data_type,
        ];
      }
    }
    return $this->attributes;
  }
  
  /**
   * 
   */
  public function attributeValues() {
    if (empty($this->attribute_values)) {
      $sql = "
        SELECT CVT.name, NEP.value, NEP.value_num, DBX.accession, DB.name as db_name
        FROM {network_edgeprop} NEP
          INNER JOIN {cvterm} CVT ON CVT.cvterm_id = NEP.type_id
          INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
          INNER JOIN {db} DB on DB.db_id = DBX.db_id
        WHERE NEP.network_edge_id = :network_edge_id
      ";
      $args[':network_edge_id'] = $this->id;
      $results = chado_query($sql, $args);
      
      while ($prop = $results->fetchObject()) {
        $term = $prop->db_name . ':' . $prop->accession;
        $this->attribute_values[$term] = $prop->value ? $prop->value : $prop->value_num;
      }
    }
    return $this->attribute_values;
  }
  
  /**
   * 
   */
  public function expression() {

    $network_id = $this->details['network_id'];
    $source = $this->details['source_id'];
    $target = $this->details['target_id'];
    $sample_names = [];
       
    // The SQL for getting the expression.
    $sql = "
       SELECT F.feature_id, B.biomaterial_id, B.name as bname, ER.signal
       FROM {elementresult} ER
         INNER JOIN {element} E ON E.element_id = ER.element_id
         INNER JOIN {feature} F ON F.feature_id = E.feature_id
         INNER JOIN {quantification} Q ON ER.quantification_id = Q.quantification_id
         INNER JOIN {acquisition} ACQ ON ACQ.acquisition_id = Q.acquisition_id
         INNER JOIN {assay} A ON A.assay_id = ACQ.assay_id
         INNER JOIN {assay_biomaterial} AB ON A.assay_id = AB.assay_id
         INNER JOIN {biomaterial} B ON AB.biomaterial_id = B.biomaterial_id
         INNER JOIN {network_analysis} NA ON NA.analysis_id = Q.analysis_id
       WHERE F.feature_id = :feature_id and NA.network_id = :network_id
       ORDER BY B.biomaterial_id
    ";
    
    // TODO: what if the network analysis and the expression analysis are not the same!!
    
    // Get the first feature expression data.
    $f1_exp = [];
    $args = [':feature_id' => $source, ':network_id' => $network_id];
    $results = chado_query($sql, $args);
    while ($exp_vals = $results->fetchObject()) {
      $f1_exp[$exp_vals->biomaterial_id] = $exp_vals->signal;
      $sample_names[$exp_vals->biomaterial_id] = $exp_vals->bname;
    }
    
    // Get the second feature expression data.
    $f2_exp = [];
    $args = [':feature_id' => $target, ':network_id' => $network_id];
    $results = chado_query($sql, $args);
    while ($exp_vals = $results->fetchObject()) {
      $f2_exp[$exp_vals->biomaterial_id] = $exp_vals->signal;
      $sample_names[$exp_vals->biomaterial_id] = $exp_vals->bname;
    }
    
    // Get the properties of the biomaterials for this edge.
    $biomaterial_ids = array_keys($sample_names);
    $sql = "
      SELECT BP.biomaterial_id, CVT.name, BP.value
      FROM {biomaterialprop} BP
        INNER JOIN {cvterm} CVT on CVT.cvterm_id = BP.type_id
      WHERE BP.biomaterial_id IN (:biomaterial_ids)
    ";
    $args = [':biomaterial_ids' => $biomaterial_ids];
    $results = chado_query($sql, $args);
    $sample_attribute_names = [];
    $sample_attribute_values = [];
    $sample_attribute_labels = [];
    $sample_attribute_types = [];
    $uvalues = [];
    while($attr = $results->fetchObject()) {
      $attr_name = ucfirst($attr->name);
      $sample_attribute_names[$attr_name] = 1;
      $sample_attribute_values[$attr->biomaterial_id][$attr_name] = $attr->value;
      $uvalues[$attr_name][$attr->value] = 1;
    }
    $sample_attribute_names = array_keys($sample_attribute_names);
    
    // Creata unique list of lables (categories) that are present for each attribute.
    foreach ($uvalues as $attr_name => $values) {
      $ukvalues = array_keys($values);
      sort($ukvalues, SORT_NUMERIC );
      $sample_attribute_labels[$attr_name] = $ukvalues;
      $sample_attribute_types[$attr_name] = 'numeric';
    }    
    
    // Determine the type of value.
    foreach ($sample_attribute_values as $biomaterial_id => $attr) {
      foreach ($attr as $attr_name => $value) {        
        if (!is_numeric($value)) {
          $sample_attribute_types[$attr_name] = 'categorical';
        }
      }
    }     
    
    return [
      'expression' => [
        $source => $f1_exp,
        $target => $f2_exp,
      ],
      'samples' => [
        'names' => $sample_names,
        'attributes' => [
          'names'  => $sample_attribute_names,
          'values' => $sample_attribute_values,
          'types'  => $sample_attribute_types,
          'labels' => $sample_attribute_labels,
        ],
      ],
    ];
  }
}
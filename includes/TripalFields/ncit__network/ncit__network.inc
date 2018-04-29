<?php

class ncit__network extends ChadoField {

  // The default lable for this field.
  public static $default_label = 'Network';

  // The default description for this field.
  public static $description = 'An interconnected system of things or people.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'NCIT',
    // The name of the term.
    'term_name' => 'Network',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'C61377',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );

  // The default widget for this field.
  public static $default_widget = 'ncit__network_widget';

  // The default formatter for this field.
  public static $default_formatter = 'ncit__network_formatter';


  /**
   * @see TripalField::validate()
   */
  public function validate($entity_type, $entity, $langcode, $items, &$errors) {

    
  }

  /**
   * @see TripalField::load()
   */
  public function load($entity) {

    $record = $entity->chado_record;
    $settings = $this->instance['settings'];

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    // Get the terms for each of the keys for the 'values' property.
    $uname_term = tripal_get_chado_semweb_term('network', 'uniquename');
    $name_term = tripal_get_chado_semweb_term('network', 'name');
    $organism_term = tripal_get_chado_semweb_term('network', 'organism_id');
    $type_term = tripal_get_chado_semweb_term('network', 'type_id');
    
    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = array(
      'value' => array(),
    );
    
    // Find the networks for the given table.
    if ($field_table == 'organism') {
      $organism = $record->organism_id;
      $linker_field = 'chado-network__organism_id';
      $linker_value = $organism->organism_id;
      $networks = chado_generate_var('network', array('organism_id' => $organism->organism_id));
    }
    if ($field_table == 'analysis') {
      $analysis = $record->analysis_id;
      $linker_field = 'chado-network_analysis__analysis_id';
      $linker_value = $analysis->analysis_id;
      $network_analyses = chado_generate_var('network_analysis', array('analysis_id' => $analysis->analysis_id));
      $options = array(
        'return_array' => TRUE, 
        'order_by' => array('name', 'ASC'),        
      );
      $network_analyses = chado_expand_var($network_analysis, 'table', 'network_analysis', $options);
      $networks = $network_analysis->networks;
    }
    
    // Iterate through the networks and create an entry for each one.
    if ($networks and count($networks) > 0) {
      
      $i = 0;
      foreach ($networks as $network) {
        $entity->{$field_name}['und'][$i]['value'] = array(
          $uname_term => $network->uniquename,
          $name_term => $network->name,
          $organism_term => '',
          $type_term => $network->type_id->name,
        );      
        $entity->{$field_name}['und'][$i][$linker_field] = $linker_value;
  
        // Is there a published entity for this network?
        if (property_exists($network, 'entity_id')) {
          $entity->{$field_name}['und'][$i]['value']['entity'] = 'TripalEntity:' . $network->entity_id;
        }
      }
    }
  }

  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();

    // Get the terms for each of the keys for the 'values' property.
    $uname_term = tripal_get_chado_semweb_term('network', 'uniquename');
    $name_term = tripal_get_chado_semweb_term('network', 'name');
    $organism_term = tripal_get_chado_semweb_term('network', 'organism_id');
    $type_term = tripal_get_chado_semweb_term('network', 'type_id');
    
    return array(
      $field_term => array(
        'operations' => array('eq', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
        'readonly' => TRUE,
        'type' => 'xs:complexType',
        'elements' => array(
          $uname_term => array(
            'searchable' => TRUE,
            'name' => 'identifier',
            'operations' => array('eq', 'ne', 'contains', 'starts'),
            'sortable' => TRUE,
            'readonly' => FALSE,
            'type' => 'xs:string',
            'required' => TRUE,
          ),
          $name_term => array(
            'searchable' => TRUE,
            'name' => 'name',
            'operations' => array('eq', 'ne', 'contains', 'starts'),
            'sortable' => TRUE,
            'readonly' => FALSE,
            'type' => 'xs:string',
            'required' => TRUE,
          ),
          $organism_term => array(
            'searchable' => TRUE,
            'name' => 'organism',
            'operations' => array('eq', 'ne', 'contains', 'starts'),
            'sortable' => TRUE,
            'readonly' => FALSE,
            'type' => 'xs:integer',
            'required' => FALSE,
          ),
          $type_term => array(
            'searchable' => TRUE,
            'name' => 'type',
            'operations' => array('eq', 'ne', 'contains', 'starts'),
            'sortable' => TRUE,
            'readonly' => FALSE,
            'type' => 'xs:integer',
            'required' => FALSE,
          ),
          'entity' => array(
            'searchable' => FALSE,
          ),
        ),
      ),
    );
  }

  /**
   * @see ChadoField::query()
   */
  public function query($query, $condition) {
   
  }

  /**
   * @see ChadoField::queryOrder()
   */
  public function queryOrder($query, $order) {
   
  }
}
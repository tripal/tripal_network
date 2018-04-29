<?php

class local__network_module extends ChadoField {

  // The default lable for this field.
  public static $default_label = 'Network Modules';

  // The default description for this field.
  public static $description = 'Groups, clusters or communities of highly connected nodes within a network (or graph).';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'Network Module',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'Network Module',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );

  // The default widget for this field.
  public static $default_widget = 'local__network_module_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__network_module_formatter';


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
    $name_term = tripal_get_chado_semweb_term('networkmod', 'name');
    $type_term = tripal_get_chado_semweb_term('network', 'type_id');
            
    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = array(
      'value' => array(),
    );
    
    // Find the modules for the given network
    $options = array();
    $records = chado_generate_var('networkmod', array('nework_id' => $record->network_id), $options);
    $modules = array();
    if (is_array($records)) {
      $modules = $records;
    }
    else {
      $modules[] = $records;
    }

    
    // Iterate through the networks and create an entry for each one.
    if ($modules and count($modules) > 0) {
      
      $i = 0;
      foreach ($modules as $module) {
        
        $entity->{$field_name}['und'][$i]['value'] = array(
          $name_term => $module->name,
          $type_term => $module->type_id->name,
        );
        $i++;
      }
    }
  }

  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();

    // Get the terms for each of the keys for the 'values' property.
    $name_term = tripal_get_chado_semweb_term('networkmod', 'name');
    $type_term = tripal_get_chado_semweb_term('network', 'type_id');
    
    return array(
      $field_term => array(
        'operations' => array('eq', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
        'readonly' => TRUE,
        'type' => 'xs:complexType',
        'elements' => array(
          $name_term => array(
            'searchable' => TRUE,
            'name' => 'name',
            'operations' => array('eq', 'ne', 'contains', 'starts'),
            'sortable' => TRUE,
            'readonly' => FALSE,
            'type' => 'xs:string',
            'required' => TRUE,
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

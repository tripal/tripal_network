<?php

/**
 * Implements hook_bundle_fields_info().
 *
 * This is a Tripal defined hook that supports integration with the
 * TripalEntity field.
 */
function tripal_network_bundle_fields_info($entity_type, $bundle) {

  $chado_bundle = db_select('chado_bundle', 'cb')
    ->fields('cb')
    ->condition('bundle_id', $bundle->id)
    ->execute()
    ->fetchObject();

  $table_name = $chado_bundle->data_table;
  $type_table = $chado_bundle->type_linker_table;
  $type_column = $chado_bundle->type_column;
  $cvterm_id  = $chado_bundle->type_id;
  $type_value = $chado_bundle->type_value;

  $schema = chado_get_schema($table_name);

  if ($table_name == 'organism' or $table_name == 'analysis') {
    $field_name = 'ncit__network';
    $field_type = 'ncit__network';
    $info[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }

  if ($table_name == 'network') {
    $field_name = 'local__network_module';
    $field_type = 'local__network_module';
    $info[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }
  
  return $info;
}

/**
 * Impelments hook_create_tripalfield_instance().
 *
 * This is a Tripal defined hook that supports integration with the
 * TripalEntity field.
 */
function tripal_network_bundle_instances_info($entity_type, $bundle) {

  $chado_bundle = db_select('chado_bundle', 'cb')
    ->fields('cb')
    ->condition('bundle_id', $bundle->id)
    ->execute()
    ->fetchObject();

  $table_name = $chado_bundle->data_table;
  $type_table = $chado_bundle->type_linker_table;
  $type_column = $chado_bundle->type_column;
  $cvterm_id  = $chado_bundle->type_id;
  $type_value = $chado_bundle->type_value;
  
  $schema = chado_get_schema($table_name);

  if ($table_name == 'organism' or $table_name == 'analysis') {
    $field_name = 'ncit__network';
    $is_required = FALSE;
    if ($table_name == 'organism') {
      $table_column = 'organism_id';
    }
    if ($table_name == 'analysis') {
      $table_column = 'analysis_id';
    }
    $info[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Networks',
      'description' => 'Networks for this organism.',
      'required' => $is_required,
      'settings' => array(
        'auto_attach' => TRUE,
        'chado_table' => $table_name,
        'chado_column' => $table_column,
        'base_table' => $table_name,
      ),
      'widget' => array(
        'type' => 'ncit__network_widget',
        'settings' => array(
          'display_label' => 1,
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'inline',
          'type' => 'ncit__network_formatter',
          'settings' => array(),
        ),
      ),
    );
  }

  if ($table_name == 'network') {
    $field_name = 'local__network_module';
    $is_required = FALSE;
    $table_column = 'network_id';
    $info[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Network Modules',
      'description' => 'Modules identified in this network.',
      'required' => $is_required,
      'settings' => array(
        'auto_attach' => TRUE,
        'chado_table' => $table_name,
        'chado_column' => $table_column,
        'base_table' => $table_name,
      ),
      'widget' => array(
        'type' => 'local__network_module_widget',
        'settings' => array(
          'display_label' => 1,
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'inline',
          'type' => 'local__network_module_formatter',
          'settings' => array(),
        ),
      ),
    );
  }

  return $info;
}

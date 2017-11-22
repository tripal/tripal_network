<?php

/**
 * Implements hook_field_storage_info().
 */
function tripal_network_field_storage_info() {
  return array(
    'field_tn_neo4j_storage' => array(
      'label' => t('Tripal Network Neo4j'),
      'description' => t('Stores fields realted to networks in a Neo4j database.'),
      'settings' => array(),
      'logo_url' => '',
    ),
  );
}

/**
 * Implements hook_field_storage_write().
 */
function tripal_network_field_storage_write($entity_type, $entity, $op, $fields) {

}


/**
 * Implements hook_field_storage_pre_load().
 *
 */
function tripal_network_field_storage_pre_load($entity_type, $entities, $age,
    $fields, $options) {

}

/**
 * Implements hook_field_storage_load().
 */
function tripal_network_field_storage_load($entity_type, $entities, $age,
    $fields, $options) {

}

/**
 * Implements hook_field_storage_query().
 */
function tripal_network_field_storage_query($query) {

}

/**
 * Implements hook_field_storage_bundle_mapping_form().
 *
 * This is a Tripal added hook to the field storage API.
 */
function tripal_network_field_storage_bundle_mapping_form($form, &$form_state,
    $term, &$submit_disabled) {

}


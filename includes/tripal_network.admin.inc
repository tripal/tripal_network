<?php

/**
 *
 * @return string
 */
function tripal_network_admin_home($organism_id = NULL) {
  $params = [];
  $sql = "SELECT * FROM {network} N";
  if ($organism_id) {
    $sql .= " WHERE N.organism_id = :organism_id";
    $params[':organism_id'] = $organism_id;
  }
  
  $networks = chado_query($sql, $params);
  while ($network = $networks->fetchObject()) {
    
  }
}

function tripal_network_admin_form($form, &$form_state) {
  $form['settings'] = [
    '#type' => 'fieldset',
    '#title' => 'Settings',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['settings']['default_layer_by'] = [
    '#type' => 'select',
    '#title' => t('Default Layer By Attribute'),
    '#options' => [],
    '#default_value' => ''
  ];
  return $form;
}

/**
 * Provides the administrative help page for this module.
 */
function tripal_network_admin_help() {
  return 'Help text goes here';
}
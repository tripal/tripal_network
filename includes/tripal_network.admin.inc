<?php

/**
 *
 * @return string
 */
function tripal_network_admin_home($organism_id = NULL) {
  
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  drupal_set_breadcrumb($breadcrumb);
  
  $admin_form = drupal_get_form('tripal_network_admin_form');
  $admin_form = drupal_render($admin_form);
    
  return $admin_form;
}

/**
 * 
 */
function tripal_network_admin_form($form, &$form_state) {

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save Settings'),
  ];
  
  return $form;
}

/**
 * 
 */
function tripal_network_admin_form_submit($form, $form_state) {
  
}

/**
 * Provides the administrative help page for this module.
 */
function tripal_network_admin_help() {
  return 'Help text goes here';
}
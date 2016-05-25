<?php

/**
 * Performs the AJAX filters used by the JavaScript Visualization code.
 */
function tripal_network_ajax_filter() {
  // Network Filters
  $species = array_key_exists('species', $_POST) ? $_POST['species'] : '';
  $version = array_key_exists('version', $_POST) ? $_POST['version'] : '';
  // Condition Filters
  $conditions = array_key_exists('conditions', $_POST) ? $_POST['conditions'] : '';
  // Gene List Filter
  $genes = array_key_exists('genes', $_POST) ? $_POST['genes'] : '';
  // Trait List Filter
  $traits = array_key_exists('traits', $_POST) ? $_POST['traits'] : '';
  // Marker List Filter
  $markers = array_key_exists('markers', $_POST) ? $_POST['markers'] : '';
  // Sample List Filter
  $samples = array_key_exists('samples', $_POST) ? $_POST['samples'] : '';

  dpm($args);
  return '';
}
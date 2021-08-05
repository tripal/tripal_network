<?php

/**
 * Implements hook_tripal_default_title_format().
 *
 * Overrides the default titles.
 */
function tripal_network_tripal_default_title_format($bundle, $available_tokens) {
  $format = [];

  $table = $bundle->data_table;

  if ($table == 'network') {
    $format[] = [
      'format' => '[schema__name]',
      'weight' => -5,
    ];
  }
  return $format;
}
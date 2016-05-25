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


  //   {
  //     “nodes”: [
  //     {“name”: “Gene1” , “group” : [“group1”]},
  //     {“name”: “Gene2” , “group” : [“group1”]},
  //     {“name”: “Gene3” , “group” : [“group1”]},
  //     ],
  //     “links”: [
  //     {“source”: 0 , “target”: 1, “group” : [“group1”]},
  //     ]
  //   }

  $results = array(
    'nodes' => array(
      array('name' => 'gene1', 'group' => 'group1'),
      array('name' => 'gene2', 'group' => 'group1'),
      array('name' => 'gene3', 'group' => 'group2'),
      array('name' => 'gene4', 'group' => 'group2'),
      array('name' => 'gene5', 'group' => 'group2'),
    ),
    'links' => array(
      array('source' => 0, 'target' => 1)
    ),
  );
  return drupal_json_output($results);
}
<?php
/**
 * Implements hook_chado_schema_v1_3_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_3_organism() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network',
  );

  return $description;
}

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_2_organism() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network',
  );

  return $description;
}
/**
 * Implements hook_chado_schema_v1_3_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_3_feature() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_node',
  );

  return $description;
}

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_2_feature() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_node',
  );

  return $description;
}
/**
 * Implements hook_chado_schema_v1_3_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_3_analysis() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_analysis',
  );

  return $description;
}

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_2_analysis() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_analysis',
  );

  return $description;
}
/**
 * Implements hook_chado_schema_v1_3_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_3_pub() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_pub',
  );

  return $description;
}

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_2_pub() {
  $description = array();

  // add the 'network' table as a referring table to the organism table
  $description['referring_tables'] = array(
    'network_pub',
  );

  return $description;
}


/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_3_cvterm() {
  $description = array();
  
  $description['referring_tables'] = [
    'network_attr',
    'network_attrprop',
    'network_cvterm',
    'network_edgeprop',
    'network_nodeprop',
    'networkprop',
  ];
  return $description;
}

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
function tripal_network_chado_schema_v1_2_cvterm() {
  $description = array();
  
  $description['referring_tables'] = [
    'network_attr',
    'network_attrprop',
    'network_cvterm',
    'network_edgeprop',
    'network_nodeprop',
    'networkprop',
  ];
  return $description;
}


/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
// function tripal_network_chado_schema_v1_3_file() {
//   $description = array();
  
//   $description['referring_tables'] = [
//     'network_file',
//   ];
//   return $description;
// }

/**
 * Implements hook_chado_schema_v1_2_table()
 *
 * Purpose: To add descriptions and foreign keys to default table description
 * Note: This array will be merged with the array from all other implementations
 *
 * @return
 *    Array describing the network table
 *
 * @ingroup tripal_schema_api
 */
// function tripal_network_chado_schema_v1_2_file() {
//   $description = array();
  
//   $description['referring_tables'] = [
//     'network_file',
//   ];
//   return $description;
// }



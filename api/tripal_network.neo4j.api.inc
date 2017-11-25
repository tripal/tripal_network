<?php

require_once libraries_get_path('neo4j-php-client') . '/vendor/autoload.php';

use GraphAware\Neo4j\Client\ClientBuilder;

global $tn_neo4j_sessions;
global $tn_neo4j_active;

/**
 * Instatiates a connection to a Neo4J database.
 *
 * @param $driver
 *   Either 'default' or 'bolt'.
 */
function tripal_neo4j_connect($driver, $uri) {
  global $tn_neo4j_sessions;
  global $tn_neo4j_active;
  $tn_neo4j_active = $uri;

  if (!is_array($tn_neo4j_sessions)) {
    $tn_neo4j_sessions = array();
  }

  if (!array_key_exists($uri, $tn_neo4j_sessions)) {
    $client = ClientBuilder::create()
      ->addConnection($driver, $uri)
      ->build();
      $tn_neo4j_sessions[$tn_neo4j_active] = $client;
  }
}
/**
 * Executes a Cypher statement with the active Neo4j database.
 *
 * @param $query
 *   A Cypher statement.
 * @param $params
 *   The statement parameters.
 *
 * @return
 *   A CypherResult object.
 */
function tripal_neo4j_query($query, $params = array()) {
  global $tn_neo4j_sessions;
  global $tn_neo4j_active;

  drupal_debug($params);
  $client = $tn_neo4j_sessions[$tn_neo4j_active];
  return $client->run($query, $params);
}
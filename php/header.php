<?php

$edge_count = 0;

//Setting of maximum execution time so that the script runs for atleast 5 minutes
ini_set('max_execution_time', 300);

error_log('header.php: ' . print_r($_POST, TRUE), 3, "/tmp/php_log");

// Matching criteria for querying into Neo4j
if (isset($_POST["submit"])) {

  $status = 0;
  // Name of the species selected by the user
  $species= $_POST["species"];
  // Name of the module as selected by the user
  $module = $_POST["module"];

  $rel = $species."module";

  //Matching query for neo4j
  $que = "MATCH(n1:".$species.")-[rel:".$rel."]->(n2:".$species.") WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id";
  $data = array(
    "query" => $que,
    "params" => array(
      "value"=>$module
    )
  );

  // Neo4j REST API calls
  $data = json_encode($data);
  $curl = curl_init();
  curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
  curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
  $result1 = curl_exec($curl);

  error_reporting(E_ALL);
  error_log('RESULT: ' . print_r($result1, TRUE), 3, "/tmp/php_log");
  $result = json_encode($result1);
  $result1 = json_decode($result1, TRUE);

  //echo $result1;
  ////echo "<br /><br />";

  $nodes = array();
  $edges = array();

  $i = 0;
  $e = 0;

  // Storing the nodes in an array
  // Purpose of this is to use the index for mapping of ID's
  foreach ($result1['data'] as $row) {
    //Following is for the storage of names which are id's only with proper indexing
    $nodes[$i] = $row[1];
    $i++;
    $nodes[$i] =$row[2];
    $i++;

    $edge_count=$edge_count+1;
  }



  //Creating an array with unique ID's as we don't want repetition of nodes in the given array
  $nodes = array_unique($nodes);

  //print_r($nodes);
  //echo "<br />";
  sort($nodes);
  //print_r($nodes);



  $num = count($nodes);
  $n=0;
  $node=array();

  // Storage of the nodes in the proper format with the correct indexing
  $colors = array("#009999","#666","#003366","#660066","#00FF00");
  for ($x = 0; $x < $num; $x++) {
    $temp = array();
    $temp["id"]= (string)$x;
    $temp["label"]= (string)$nodes[$x];
    $temp["x"]=mt_rand();
    $temp["y"]=mt_rand();
    $temp["size"]=mt_rand(1,3)/1000;
    $val = mt_rand(0,4);
    $temp["color"]="gray";
    //$temp["color"]="gray";

    //$temp["name"] = $nodes[$x];
    //$temp["group"] = 2;
    $node[$n] = $temp;
    $n++;

  }


  // Storage of the nodes in proper format
  foreach($result1['data'] as $row) {
    //Following is for the storage of the relationships with source and targets mapped to proper indexes
    $temp=array();
    $temp["id"]=(string)$e;
    $key = array_search($row[1],$nodes);
    $temp["source"]= (string)$key;
    $source=$key;
    $key = array_search($row[2],$nodes);
    $temp["target"]=(string)$key;
    $target=$key;
    $temp["size"]=mt_rand();
    $temp["type"]="straight";
    $temp["color"]="#ccc";
    $temp["hover_color"]="#A0A0A0";
    $edges[$e]=$temp;
    $e++;
  }
  $return=array('nodes'=>$node,'edges'=>$edges);

  header('Content-Type : application/json');
  $jsonarray = json_encode($return);
  //echo $jsonarray;

  $fh= fopen("miss.json",'w') or die("Error opening file");

  fwrite($fh,$jsonarray);

  curl_close($curl);
}
else {
  error_log('no submit', 3, "/tmp/php_log");
  $fh  = fopen("miss.json","w") or die("Error opening file");
  fwrite($fh," ");
  $status=1;
}  ?>

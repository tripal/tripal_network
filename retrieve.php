<?php

//echo $_POST["module"];
$module=(string)$_POST["module"];
 $data=array(
 "query" => "MATCH(n1:rice)-[rel:ricemodule]->(n2:rice) WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id",
  "params" => array(
      "value"=>$module
  )
 );


$data=json_encode($data);
$curl = curl_init();
curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
$result1 = curl_exec($curl);
curl_close($curl);

$result=json_encode($result1);
$result1 = json_decode($result1, TRUE);

$nodes = array();
$edges = array();
foreach ($result1['data'] as $row) {
  $nodes[$row[1]] = count($nodes);
  $nodes[$row[2]] = count($nodes);

  $n1_index = $nodes[$row[1]];
  $n2_index = $nodes[$row[2]];
  $edges[$n1_index][$n2_index] = 1;
}
print_r($edges);

$return = array(
  'nodes' => $nodes,
  'links' => $edges
);

$jsonarray= json_encode($return);

header('Content-Type: application/json');
echo $jsonarray;





?>
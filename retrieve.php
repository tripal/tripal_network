<?php

//PHP script for querying Neo4j with the appropriate matching format and also 
//Conversion of the returned format to the format required by D3.js for rendering purpose


//echo $_POST["module"];

// Matching criteria for querying into Neo4j
$module=(string)$_POST["module"];
 $data=array(
 "query" => "MATCH(n1:rice)-[rel:ricemodule]->(n2:rice) WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id",
  "params" => array(
      "value"=>$module
  )
 );

// Neo4j REST API calls


$data=json_encode($data);  
$curl = curl_init();
curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
$result1 = curl_exec($curl);

$result=json_encode($result1);



$result1 = json_decode($result1,TRUE);

//echo $result1;
////echo "<br /><br />";

$nodes = array();
$edges = array();

$i=0;
$e=0;

// Storing the nodes in an array
// Purpose of this is to use the index for mapping of ID's
foreach($result1['data'] as $row)
{
	//Following is for the storage of names which are id's only with proper indexing
    $nodes[$i] = $row[1];
    $i++;
    $nodes[$i] =$row[2];
    $i++;


}



//Creating an array with unique ID's as we don't want repetition of nodes in the given array

 $nodes = array_unique($nodes);

  //print_r($nodes);
  //echo "<br />";
  sort($nodes);
  //print_r($nodes);


// Storage of the nodes in proper format

foreach($result1['data'] as $row)
{
	//Following is for the storage of the relationships with source and targets mapped to proper indexes
    $temp=array();

    $key = array_search($row[1],$nodes);
    $temp["source"]= $key;
    $key = array_search($row[2],$nodes);
    $temp["target"]=$key;
    $temp["value"]=3;

    $edges[$e]=$temp;
    $e++;

}



 

$num = count($nodes);
$n=0;
$node=array();

// Storage of the nodes in the proper format with the correct indexing
for($x=0;$x<$num;$x++)
{
	$temp = array();
	$temp["name"] = $nodes[$x];
	$temp["group"] = 2;
	$node[$n] = $temp;
	$n++;

}








$return=array('nodes'=>$node,'links'=>$edges);

header('Content-Type : application/json');
$jsonarray = json_encode($return);
echo $jsonarray;

curl_close($curl);




?>

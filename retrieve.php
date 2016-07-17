<?php

//match n-[r]-()
//return distinct type(r)

$species = $_GET["q"];

$que = "MATCH p=()-[r:".$species."module]->() RETURN distinct(r.modulename)";
 $data=array(
 "query" => $que
);

$data = json_encode($data);
$curl = curl_init();
curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl, CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
$result1 = curl_exec($curl);



//When TRUE returned objects will be converted into associative arrays
$result1 = json_decode($result1,TRUE);
error_log('retrieve.php: ' . print_r($result1, TRUE), 3, "/tmp/php_log");
$string = "<select>";

foreach($result1['data'] as $row)
{
  //Following is for the storage of names which are id's only with proper indexing
	echo "<option>". $row[0]."</option>";
    //echo $row[0];
    //echo "<br />";


}

$string = $string + "</select>";

//echo $string;




curl_close($curl);
?>


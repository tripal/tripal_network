<?php
//This script will do the following things : 
//The operations are written over the piece of code doing the specific operations

//Parse through the GraphML file
//Select a node from the file , check if that node is already present. If it is present then, it will not be inserted. Else it will be inserted.

//$species here is just a proxy data. It has be filled by the result of the $_POST variable which the user submits. eg. $_POST["species"];

//This script is ready to use, just some error handling issues needs to be taken care of


$species="rice";

//This will open the graphml file and put the cursor at the front

 $myfile=simplexml_load_file('test.gml');

//When file is not found. This has to be filled with a proper error alert instead of an echo

 if(!$myfile)
 {
 	echo "File cannot be loaded";
 }

foreach($myfile->node as $nodeinfo)
 {
    $nodeid=$nodeinfo['id'];
    $nodeid=(string)$nodeid;
// This statement is querying if the node with the given id exists;
$var="MATCH(u:".$species.") WHERE u.id = {id} RETURN u.id";
 $data=array(
 "query" => $var,
  "params" => array(
  	"id"=>$nodeid
  )
 );
// This piece of code, will use the Neo4j REST API to pass the request
$data=json_encode($data);  
$curl = curl_init();
curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
curl_setopt($curl, CURLOPT_POSTFIELDS,$data);

//The result from the request gets stored in the $result1 variable

$result1 = curl_exec($curl);

//echo $result1;
curl_close($curl); 
//Converting the result into a json array
$result=json_encode($result1);

// The result given by the REST API request gives an empty field with "data". 
$word="data";

if(strpos($result,$word)!==false)
{
	
	$pos=strpos($result,$word);
	
}

//Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present
$pos=$pos+11;

//
if($result[$pos]=="[")
{
	echo "";
}

//Here in the else statement the query for the insertion of the nodes will be there.
else
{
	$create_query="CREATE (x:".$species." { id : {id} }) RETURN x";
	$data=array(
 "query" => $create_query,
  "params" => array(
"id" => $nodeid
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
//echo $result1;
curl_close($curl);
}







}

//End of for loop for entering the required nodes


//The following piece of code will scrape through the edges present and create relationships among the existing nodes

foreach($myfile->edge as $edgeinfo)
{
	//storing the module name
    $module=(string)$edgeinfo->data;

    //storing the network name
    $networkname=(string)$edgeinfo->data[1];

    //storing the source
    $source=(string)$edgeinfo['source'];

    $target=(string)$edgeinfo['target'];

    //Naming the relationship 
    $module_rel = $species."module";


//This query is checking if a relationship corresponding to the source and the destination with the specific label of the relationship already exists

 $data=array(
 "query" => "MATCH(n1:rice{id:{id1}})-[rel:ricemodule]->(n2:rice{id:{id2}}) WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id",
  "params" => array(
  	"id1"=>$source,
  	"id2"=>$target,
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

//echo $result1;

curl_close($curl);


$result=json_encode($result1);
$word="data";
//Search for the word data in the string returned by the REST API call

if(strpos($result,$word)!==false)
{
	
	$pos=strpos($result,$word);
	
}

//Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present

//Here there has to be an else statement which will handle any sort of errors
$pos=$pos+11;


// if "[" exists then that means there has been something that has been returned and a relationship of the desired type already exists

if($result[$pos]=="[")
{
	echo "";
}

//
else
{
	
	//The following query will attach a relationship to two nodes with the source and target as extracted from the graphml file

    $insert_query="MATCH (u:".$species."{id:{source}}),(r:".$species."{id:{target}}) CREATE (u)-[:".$module_rel."{modulename:{mod},network:{net}}]->(r)";

    $data=array(
 "query" => $insert_query,
  "params" => array(
"source" => $source,
"target" =>$target,
"mod"=>$module,
"net"=>$networkname
  )
 ); 

//encoding the data in json format
$data=json_encode($data);  

//Sending data through the Neo4j REST API to the server

$curl = curl_init();
curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
$result1 = curl_exec($curl);
//echo $result1;
curl_close($curl);
}







}


//End of for loop for the insertion of relationships among the nodes 




//End of loader.php






?>
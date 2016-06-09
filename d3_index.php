<?php

 //Start of the PHP script which will generate the graphics with data from Neo4j
 //Currently this is working with a POST request
// However it will be working with an AJAX request soon !!

   if(isset($_POST["submit"]))
   {
     

//PHP script for querying Neo4j with the appropriate matching format and also 
//Conversion of the returned format to the format required by D3.js for rendering purpose
    $status=0;

//echo $_POST["module"];

// Matching criteria for querying into Neo4j



$module=$_POST["module"];
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
//echo $jsonarray;

//Writing the JSON object into a .json file for d3.js to access it

$fh= fopen("miss.json",'w') or die("Error opening file");

fwrite($fh,$jsonarray);

curl_close($curl);


 


   }


   else
   {
       $fh  = fopen("miss.json","w") or die("Error opening file");
       fwrite($fh," ");
       $status=1;

   }


   //End of PHP script for generating graphics with data from Neo4j


?>





<!DOCTYPE html>
<html>
<meta charset="utf-8">
<head>
<title>Tripal Home</title>
<meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

<link rel="stylesheet" href="http://yui.yahooapis.com/pure/0.6.0/pure-min.css">

<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css" integrity="sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7" crossorigin="anonymous">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/2.2.3/jquery.min.js"></script>
<!-- Optional theme -->
<!-- Optional theme -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap-theme.min.css" integrity="sha384-fLW2N01lMqjakBkx3l/M9EahuwpSfeNvV63J5ezn3uZzapT0u7EYsXMjQV+0En5r" crossorigin="anonymous">


<!-- Latest compiled and minified JavaScript -->
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js" integrity="sha384-0mSbJDEHialfmuBBQP6A4Qrprq5OVfW37PRR3j5ELqxss1yVqOtnepnHVP9aJ7xS" crossorigin="anonymous"></script>

<!--  <link rel="stylesheet" href="css/style.css" /> -->

<link rel="stylesheet" href="tablecss.CSS" />
<link href='http://fonts.googleapis.com/css?family=Lato&subset=latin,latin-ext' rel='stylesheet' type='text/css'>
<style>

body {
    padding-top: 70px; /* Required padding for .navbar-fixed-top. Change if height of navigation changes. */
    font-family:Lato;

}

.scrollit
{
  overflow: scroll;
  height:200px;
}

.navbar-fixed-top .nav {
    padding: 15px 0;
}

.navbar-fixed-top .navbar-brand {
    padding: 0 15px;
}

@media(min-width:768px) {
    body {
        padding-top: 100px; /* Required padding for .navbar-fixed-top. Change if height of navigation changes. */
    }

    .navbar-fixed-top .navbar-brand {
        padding: 15px 0;
    }
}

#accordion
{
  width:25%;
  position:absolute;
  top:13%;
  left:75%;

}


.panel-heading a:after {
    font-family:'Glyphicons Halflings';
    content:"\e114";
    float: right;
    color: grey;

}
.panel-heading a.collapsed:after {
    content:"\e080";
}



.node {
  stroke: gray;
  stroke-width: 0.5px;
}

.link {
  stroke: #999;
  stroke-opacity: 0.5;
}

.node text
{

font: 10px sans-serif;
color:#000;
}

#svg-id
{
  z-index:0;
}

#genomic-view
{
   
   position:absolute;
   top:64%;
   left:1%;
   
   
}

#dataset
{
  border : 1px solid #E0E0E0;
  position:absolute;
  width:100%;
  top:74%;
  height : 200px;
  z-index : 100;
  background-color: white;
}

.table1
{
  overflow-y:scroll;
  height:150px;
}


</style>
<!-- Latest compiled and minified CSS -->

</head>


<body>
  




<div id="page-content-wrapper">


   <nav class="navbar navbar-inverse navbar-fixed-top" role="navigation">
        <div class="container">
            <!-- Brand and toggle get grouped for better mobile display -->
            <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#bs-example-navbar-collapse-1">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <a class="navbar-brand" href="#">
                    <!-- <img src="tripal.png"  alt=""> -->
                </a>
            </div>
            <!-- Collect the nav links, forms, and other content for toggling -->
            <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
                <ul class="nav navbar-nav">
                    <li class="active">
                        <a href="#" style="font-weight:700;font-size:30px;">Tripal</a>
                    </li>
                   
                </ul>
            </div>
            <!-- /.navbar-collapse -->
        </div>
       
    </nav>
  
<script src="//d3js.org/d3.v3.min.js"></script>

<!-- This is the script that will render the given data into the visualization format -->


<div id="view">
<?php

   if($status==1)
   {
      echo "<p style='font-weight:100;font-size:40px;font-family:Lato;color:#E0E0E0;'>Please select Filters for visualization</p>";
   }


?>

<script type="text/javascript" src="checker.js">

 







</script>  




</div>




</div>

<div class="panel-group" id="accordion">
    
       
        
       


     <div class="panel panel-default" id="panel5">
        <div class="panel-heading">
             <h4 class="panel-title">
        <a data-toggle="collapse" data-target="#collapseFive"
           href="#collapseFive" class="collapsed">
          Filter By Trait
        </a>
      </h4>

        </div>
        <div id="collapseFive" class="panel-collapse collapse">
            <div class="panel-body">

               <form method="post" action="d3_index.php" id="species">
                 <select id="module" name="module" onchange="loadDoc(this.value)">
                    <option>Module1</option>
                    <option>Module2</option>
                    <option>Module3</option>
                    <option>Module4</option>
                    <option>Module5</option>
                    <option>Module6</option>
                
                 </select>
                
                 <input type="submit" name="submit" />

              </form>
              
             <br /><br /><br /><br /><br />
             <br /><br /><br /><br /><br />
             <br /><br /><br /><br /><br />
             <br /><br /><br />


            </div>
        </div>
    </div>

    
</div>

</div>

 <!-- making the tables -->

 

<button type="button" class="btn btn-info btn-lg" data-toggle="modal" data-target="#myModal" id="genomic-view" style="font-weight:100;">Genome View</button>

<!-- Modal -->
<div id="myModal" class="modal fade" role="dialog">
  <div class="modal-dialog">

    <!-- Modal content-->
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal">&times;</button>
        <h4 class="modal-title">Genomic View</h4>
      </div>
      <div class="modal-body">
        <p>The following picture gives the genomic view.</p>
        <img src="genome.jpg" height="380px" width="380px" />
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>

  </div>
</div>

<div id="dataset">
<ul class="nav nav-tabs">
  <li class="active"><a data-toggle="tab" href="#home">Edge List</a></li>
  <li><a data-toggle="tab" href="#menu1">Node List</a></li>
  <li><a data-toggle="tab" href="#menu2">Markers</a></li>
    <li><a data-toggle="tab" href="#menu3">Functions</a></li>

</ul>

<div class="tab-content">
  <div id="home" class="tab-pane fade in active">
        <div class="table1">

          <!-- The following is a proxy table - will be eliminated as we get data from chado and neo4j -->
            <table style="width:100%">
                  <tr>
                   <th>Number</th>
                   <th>Edges</th>
                   <th>Functional Characteristics</th>
                   <th>hello</th>
                   <th>hello</th>
                   <th>hello</th>
                   
                </tr>

                <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   <td>hello</td>
                   <td>hello</td>
                   <td>hello</td>
                </tr>
            </table>
        </div>

  </div>
  <div id="menu1" class="tab-pane fade">
    <div class="table1">
            <table style="width:100%;">
               <tr>
                  <th>Number</th>
                  <th>Node List</th>
                  <th>Functions </th>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>

               <tr>
                 <td>1</td>
                 <td>Gene1</td>
                 <td>Stress Conditions</td>
               </tr>


            </table>
    </div>
  </div>
  <div id="menu2" class="tab-pane fade">
    <div class="table1">
 
    </div>
  </div>

  <div id="menu3" class="tab-pane fade">
    <div class="table1">
 
        </div>
  </div>
</div>

</div>


<!--
<script>

function loadDoc(str)
 {
  var xhttp;
  if(window.XMLHttpRequest)
  {
    xhttp = new XMLHttpRequest();
  }
  else
  {
    xhttp = new ActiveXObject("Microsoft.XMLHTTP");
  }

  xhttp.open("POST","test.php?q="+str,true);
  xhttp.send();


  xhttp.onreadystatechange=function()
  {
    if(xhttp.readyState==4 && xhttp.status==200)
    {
       document.getElementById('view').innerHTML='hello';





    }
  };
  
  

  




 }


 

</script>
-->

</body>


</html>


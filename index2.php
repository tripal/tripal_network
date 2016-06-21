<?php

//PHP script for querying Neo4j with the appropriate matching format and also 
//Conversion of the returned format to the format required by D3.js for rendering purpose


//echo $_POST["module"];

// Matching criteria for querying into Neo4j
if(isset($_POST["submit"]))
{





  $status=0;
  $species= $_POST["species"];
$module=$_POST["module"];

$rel=$species."module";

$que="MATCH(n1:".$species.")-[rel:".$rel."]->(n2:".$species.") WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id";
 $data=array(
 "query" => $que,
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

$edge_count =0;
foreach($result1['data'] as $row)
{
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

for($x=0;$x<$num;$x++)
{
  $temp = array();
  $temp["id"]= (string)$x;
  $temp["label"]= (string)$nodes[$x];
  $temp["x"]=mt_rand();
  $temp["y"]=mt_rand();
  $temp["size"]=mt_rand();
  $temp["color"]="#666";

  //$temp["name"] = $nodes[$x];
  //$temp["group"] = 2;
  $node[$n] = $temp;
  $n++;

}


// Storage of the nodes in proper format

foreach($result1['data'] as $row)
{
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
    $temp["type"]="curve";
    $temp["color"]="#ccc";
    $temp["hover_color"]="rgba(240,230,140,0.5)";

    
    
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
<script src="center.js"></script>
<script src="grapher.js"></script>
<script src="grapher.min.js"></script>
<script src="zoom.js"></script>

<!--  Import of Javascript File dependencies for sigma js   -->

<script src="src/sigma.core.js"></script>
<script src="src/conrad.js"></script>
<script src="src/utils/sigma.utils.js"></script>
<script src="src/utils/sigma.polyfills.js"></script>
<script src="src/sigma.settings.js"></script>
<script src="src/classes/sigma.classes.dispatcher.js"></script>
<script src="src/classes/sigma.classes.configurable.js"></script>
<script src="src/classes/sigma.classes.graph.js"></script>
<script src="src/classes/sigma.classes.camera.js"></script>
<script src="src/classes/sigma.classes.quad.js"></script>
<script src="src/classes/sigma.classes.edgequad.js"></script>
<script src="src/captors/sigma.captors.mouse.js"></script>
<script src="src/captors/sigma.captors.touch.js"></script>
<script src="src/renderers/sigma.renderers.canvas.js"></script>
<script src="src/renderers/sigma.renderers.webgl.js"></script>
<script src="src/renderers/sigma.renderers.svg.js"></script>
<script src="src/renderers/sigma.renderers.def.js"></script>
<script src="src/renderers/webgl/sigma.webgl.nodes.def.js"></script>
<script src="src/renderers/webgl/sigma.webgl.nodes.fast.js"></script>
<script src="src/renderers/webgl/sigma.webgl.edges.def.js"></script>
<script src="src/renderers/webgl/sigma.webgl.edges.fast.js"></script>
<script src="src/renderers/webgl/sigma.webgl.edges.arrow.js"></script>
<script src="src/renderers/canvas/sigma.canvas.labels.def.js"></script>
<script src="src/renderers/canvas/sigma.canvas.hovers.def.js"></script>
<script src="src/renderers/canvas/sigma.canvas.nodes.def.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edges.def.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edges.curve.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edges.arrow.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edges.curvedArrow.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edgehovers.def.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edgehovers.curve.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edgehovers.arrow.js"></script>
<script src="src/renderers/canvas/sigma.canvas.edgehovers.curvedArrow.js"></script>
<script src="src/renderers/canvas/sigma.canvas.extremities.def.js"></script>
<script src="src/renderers/svg/sigma.svg.utils.js"></script>
<script src="src/renderers/svg/sigma.svg.nodes.def.js"></script>
<script src="src/renderers/svg/sigma.svg.edges.def.js"></script>
<script src="src/renderers/svg/sigma.svg.edges.curve.js"></script>
<script src="src/renderers/svg/sigma.svg.labels.def.js"></script>
<script src="src/renderers/svg/sigma.svg.hovers.def.js"></script>
<script src="src/middlewares/sigma.middlewares.rescale.js"></script>
<script src="src/middlewares/sigma.middlewares.copy.js"></script>
<script src="src/misc/sigma.misc.animation.js"></script>
<script src="src/misc/sigma.misc.bindEvents.js"></script>
<script src="src/misc/sigma.misc.bindDOMEvents.js"></script>
<script src="src/misc/sigma.misc.drawHovers.js"></script>




<script src="plugins/sigma.plugins.dragNodes/sigma.plugins.dragNodes.js"></script>


<link rel="stylesheet" href="tablecss.CSS" />
<link href='http://fonts.googleapis.com/css?family=Lato&subset=latin,latin-ext' rel='stylesheet' type='text/css'>
<style>

body {
    /* Required padding for .navbar-fixed-top. Change if height of navigation changes. */
    
    background-color:#202020;
}

.scrollit
{
  overflow: scroll;
  height:200px;
}



@media(min-width:768px) {
   

    .navbar-fixed-top .navbar-brand {
        padding: 15px 0;
    }
}

#accordion
{
  width:15%;
  position:absolute;
  top:5%;
  left:83%;

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






#genomic-view
{
   
   position:absolute;
   top:90%;
   left:1%;
   
   
}



#dataset
{
  border : 1px solid #E0E0E0;
  position:absolute;
  width:65%;
  top:72%;
  left:15%;
  
  z-index : 100;
  background-color: white;
}

.table1
{
  overflow-y:scroll;
  height:150px;
}



#filter
{
  position:absolute;
  top:10%;
  left:92%;
}


select {
    padding:3px;
    margin: 0;
    -webkit-border-radius:4px;
    -moz-border-radius:4px;
    border-radius:4px;
    -webkit-box-shadow: 0 3px 0 #ccc, 0 -1px #fff inset;
    -moz-box-shadow: 0 3px 0 #ccc, 0 -1px #fff inset;
    box-shadow: 0 0.5px 0 #ccc, 0 -1px #fff inset;
    background: #f8f8f8;
    color:#888;
    border:none;
    outline:none;
    display: inline-block;
    -webkit-appearance:none;
    -moz-appearance:none;
    appearance:none;
    cursor:pointer;
}

#get_table
{
  position:absolute;
  top:80%;
  left:1%;
}


#data
{
  
  position:absolute;
  top:80%;
  left:82%;
  font-family: Lato;
  width:250px;
  height:150px;
  font-weight:300;
  font-size:20px;
  color:white;
  background-color:#202020;
  border-radius : 2px;
  border: 1px solid gray;
}



</style>
<!-- Latest compiled and minified CSS -->

</head>


<body>
  






 
  
<script src="//d3js.org/d3.v3.min.js"></script>

<!-- This is the script that will render the given data into the visualization format -->



  <!--
<?php

   //if($status==1)
   {
      //echo "<p style='font-weight:100;font-size:40px;font-family:Lato;color:#E0E0E0;'>Please select Filters for visualization</p>";
   }


?> -->
<div id="container">
  <style>
    #graph-container {
      top: 0;
      bottom: 0;
      left: 0;
      right: 0;
      position: absolute;
      height:700px;
      width:1400px;
      
    }
  </style>
  <div id="graph-container"></div>
</div>
<script>
//Script for rendering graphics using the GPU computational power of Web GL and the force directed layout feature of d3.js

 //var width=window.innerWidth,height=750,i;

// Initializing variable g with the json object 
var g = <?php echo $jsonarray;?>;


// Initializing the sigma instance
s = new sigma({
  graph: g,
  renderer: {
    container: document.getElementById('graph-container'),
    type: 'canvas'
  },
  settings: {
    doubleClickEnabled: false,
    minEdgeSize: 0.5,
    maxEdgeSize: 2,
    enableEdgeHovering: true,
    edgeHoverColor: 'edge',
    defaultLabelColor : 'white',
    defaultEdgeHoverColor: '#000',
    edgeHoverSizeRatio: 4,
    edgeHoverExtremities: true,
  }
});



// Bind the events:
s.bind('overNode outNode clickNode doubleClickNode rightClickNode', function(e) {
  console.log(e.type, e.data.node.label, e.data.captor);
  //document.getElementById("data").innerHTML=e.data.node.label;
});


//All data that has to be shown when a node is clicked has to be passed through it

s.bind('clickNode',function(e){

   document.getElementById("data").innerHTML="ID: "+ e.data.node.label+"<br /> Nodes: "+ <?php echo $num; ?> + "<br />Edges: "+ <?php echo $edge_count;?>;
});



s.bind('clickEdge',function(e){
   document.getElementById("data").innerHTML = "Source : "+e.data.edge.source + "<br />Target: "+ e.data.edge.target;
});

// All data that has to be shown when an edge is clicked has to be passed through it 
s.bind('overEdge outEdge doubleClickEdge rightClickEdge', function(e) {
  console.log(e.type, e.data.edge, e.data.captor);
});

//
s.bind('clickStage', function(e) {
  console.log(e.type, e.data.captor);
});


s.bind('doubleClickStage rightClickStage', function(e) {
  console.log(e.type, e.data.captor);
});



// Calling the plugin for drag
var dragListener = sigma.plugins.dragNodes(s, s.renderers[0]);

//Starting the drag via mouse
dragListener.bind('startdrag', function(event) {
  console.log(event);
});
// Dragging duration
dragListener.bind('drag', function(event) {
  console.log(event);
});

//Dropping the drag

dragListener.bind('drop', function(event) {
  console.log(event);
});

//Ending of the drag
dragListener.bind('dragend', function(event) {
  console.log(event);
});




</script>  










<div id="data"></div>


  <!--   Button when clicked will open up the filters table. -->
   <button id="filter" class="pure-button pure-button-primary">Filter</button>
  


  <form role="form" id="dataform" method="post" action="index2.php" style="width:200px;background-color:#202020;padding:1.6%;border:0.1px solid #C0C0C0;border-radius:3px;font-weight:300;color:white;position:absolute;top:20%;left:85%;">

   <div class="form-group">
    <label for="pwd" style="font-weight:300;">Species</label>

     

    <select id="module" name="species" style="background-color:#202020;color:white;border:1px solid #C0C0C0;width:100%;">
                    <option>rice</option>
                    <option>arabidopsis</option>
                    <option>maize</option>
                    
                
    </select>
  </div>
  <div class="form-group">
    <label for="pwd" style="font-weight:300;">Module</label>

     

    <select id="module" name="module" style="background-color:#202020;color:white;border:1px solid #C0C0C0;width:100%;">
                    <option>Module1</option>
                    <option>Module2</option>
                    <option>Module3</option>
                    <option>Module4</option>
                    <option>Module5</option>
                    <option>Module6</option>
                    <option>Module7</option>
                    <option>Module8</option>
                    <option>Module9</option>
                    <option>Module10</option>
                    <option>Module11</option>
                    <option>Module12</option>
                    <option>Module13</option>
                    <option>Module14</option>
                    <option>Module15</option>
                    <option>Module16</option>
                    <option>Module17</option>
                    <option>Module18</option>
                    <option>Module19</option>
                    <option>Module20</option>
                    <option>Module21</option>

                
    </select>
  </div>
 
  <input type="submit" name="submit" value="Get graph" class="pure-button pure-button-primary"/>
</form>




   <button id="get_table" class="pure-button pure-button-primary">Get Data</button>

<!--
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

               <form method="post" action="index.php" id="species">
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

-->
 <!-- making the tables -->

 

<button type="button" class="btn btn-info btn-lg" data-toggle="modal" data-target="#myModal" id="genomic-view" style="font-weight:300;">Genome View</button>

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




<div id="dataset" style="visibility:hidden;">
<ul class="nav nav-tabs" style="background-color:#202020;color:#C0C0C0;">
  <li class="active"><a data-toggle="tab" href="#home" style="text-decoration:none;color:#A0A0A0;font-weight:300;">Edge List</a></li>
  <li><a data-toggle="tab" href="#menu1" style="color:#A0A0A0;font-weight:300;">Node List</a></li>
  <li><a data-toggle="tab" href="#menu2" style="color:#A0A0A0;font-weight:300;">Markers</a></li>
    <li><a data-toggle="tab" href="#menu3" style="color:#A0A0A0;font-weight:300;">Functions</a></li>

</ul>

<div class="tab-content" >
  <div id="home" class="tab-pane fade in active">
        <div class="table1">

            <table style="width:100%;">
                  <tr>
                   <th>Number</th>
                   <th>Edges</th>
                   <th>Functional Characteristics</th>
                  
                   
                   
                </tr>

                <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                  
                   
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                  
                   
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   
                  
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   
                   
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   
                   
                </tr>

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   
                  
                </tr>

               

                 <tr>
                   <td>1</td>
                   <td>Connection1</td>
                   <td>Gene</td>
                   
                  
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
  <div id="menu2" class="tab-pane fade" style="background-color:#202020;">
    <div class="table1">
 
    </div>
  </div>

  <div id="menu3" class="tab-pane fade" style="background-color:#202020;">
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


<div style="font-weight:300;color:#E0E0E0;font-size:40px;position:absolute;top:5px;left:5px;">
 Tripal
</div>

<div id="info" style="position:absolute;top:85%;left:85%;color:white;font-size:20px;font-weight:700;font-family:Lato;font-weight:300;"></div>

</body>


</html>

<script>
$(document).ready(function(){
    $("#filter").click(function(){
        $("#dataform").toggle();
    });
});


$(document).ready(function(){
    $("#get_table").click(function(){
        $("#dataset").toggle();
    });
});

</script>
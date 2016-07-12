<?php

//PHP script to load the whole network from Neo4j into the browser
//This software is built on the top of Sigma.js with certain modifications
//It relies on the forceLink layout algorithm for the layout of the structure
//The edges are all rendered together but with a loader which ensures that the script runs throughout without failing
//Some plugins from linkurious are also imported for further data analytics from the developed graph


//Setting of maximum execution time so that the script runs for atleast 5 minutes 
ini_set('max_execution_time', 300);

// Matching criteria for querying into Neo4j
if(isset($_POST["submit"]))
{




  
 $status=0;
 // Name of the species selected by the user 
 $species= $_POST["species"];
 // Name of the module as selected by the user 
 $module=$_POST["module"];
 
 $rel=$species."module";

//Matching query for neo4j
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
$colors = array("#009999","#666","#003366","#660066","#00FF00");
for($x=0;$x<$num;$x++)
{
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
<title>Home- Tripal </title>

<meta charset="utf-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="description" content="">
<meta name="author" content="">



<style type="text/css">

#dataform
{
  z-index:100;
}

#locator
{
 position:absolute;
 top:70%;
 left:90%;
}



.sigma-tooltip {
      max-width: 240px;
      max-height: 280px;
      background-color: rgba(255, 255, 255,0.8);
      box-shadow: 0 2px 6px rgba(0,0,0,0.3);
      border-radius: 6px;
    }

    .sigma-tooltip-header {
      font-variant: small-caps;
      font-size: 120%;
      color: #437356;
      border-bottom: 1px solid #aac789;
      padding: 10px;
    }

    .sigma-tooltip-body {
      padding: 10px;
    }

    .sigma-tooltip-body th {
      color: #999;
      text-align: left;
    }

    .sigma-tooltip-footer {
      padding: 10px;
      border-top: 1px solid #aac789;
    }

    .sigma-tooltip > .arrow {
      border-width: 10px;
      position: absolute;
      display: block;
      width: 0;
      height: 0;
      border-color: transparent;
      border-style: solid;
    }

    .sigma-tooltip.top {
      margin-top: -12px;
    }
    .sigma-tooltip.top > .arrow {
      left: 50%;
      bottom: -10px;
      margin-left: -10px;
      border-top-color: rgb(249, 247, 237);
      border-bottom-width: 0;
    }

    .sigma-tooltip.bottom {
      margin-top: 12px;
    }
    .sigma-tooltip.bottom > .arrow {
      left: 50%;
      top: -10px;
      margin-left: -10px;
      border-bottom-color: rgb(249, 247, 237);
      border-top-width: 0;
    }

    .sigma-tooltip.left {
      margin-left: -12px;
    }
    .sigma-tooltip.left > .arrow {
      top: 50%;
      right: -10px;
      margin-top: -10px;
      border-left-color: rgb(249, 247, 237);
      border-right-width: 0;
    }

    .sigma-tooltip.right {
      margin-left: 12px;
    }
    .sigma-tooltip.right > .arrow {
      top: 50%;
      left: -10px;
      margin-top: -10px;
      border-right-color: rgb(249, 247, 237);
      border-left-width: 0;
    }



#control-pane {
      top: 10px;
      /*bottom: 10px;*/
      right: 10px;
      position: absolute;
      width: 150px;
      background-color: rgba(255,255,255,0.7);
      box-shadow: 0 2px 6px rgba(0,0,0,0.3);
      position:absolute;
      top:15%;
      left:1%;
      font-family:Roboto;
      font-weight:100;
    }
    #control-pane > div {
      margin: 10px;
      overflow-x: auto;
    }
    .line {
      clear: both;
      display: block;
      width: 100%;
      margin: 0;
      padding: 12px 0 0 0;
      border-bottom: 1px solid #aac789;
      background: transparent;
    }
    h2, h3, h4 {
      padding: 0;
      font-variant: small-caps;
    }
    .green {
      color: #437356;
    }
    h2.underline {
      color: #437356;
      background: rgba(255,255,255,0.8);
      margin: 0;
      border-radius: 2px;
      padding: 8px 12px;
      font-weight: 100;
    }
    .hidden {
      display: none;
      visibility: hidden;
    }

    input[type=range] {
      width: 160px;
    }


 



</style>

<link rel="icon" href="http://tripal.info/sites/default/files/TripalLogo_dark.png" />

<link href='https://fonts.googleapis.com/css?family=Roboto' rel='stylesheet' type='text/css'>
<link rel="stylesheet" href="http://yui.yahooapis.com/pure/0.6.0/pure-min.css">

<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css" integrity="sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7" crossorigin="anonymous">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/2.2.3/jquery.min.js"></script>
<!-- Optional theme -->
<!-- Optional theme -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap-theme.min.css" integrity="sha384-fLW2N01lMqjakBkx3l/M9EahuwpSfeNvV63J5ezn3uZzapT0u7EYsXMjQV+0En5r" crossorigin="anonymous">


<!-- Latest compiled and minified JavaScript -->
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js" integrity="sha384-0mSbJDEHialfmuBBQP6A4Qrprq5OVfW37PRR3j5ELqxss1yVqOtnepnHVP9aJ7xS" crossorigin="anonymous"></script>
<link href='http://fonts.googleapis.com/css?family=Lato&subset=latin,latin-ext' rel='stylesheet' type='text/css'>


</head>
<!-- START SIGMA IMPORTS -->
<script src="../src/sigma.core.js"></script>
<script src="../src/conrad.js"></script>
<script src="../src/utils/sigma.utils.js"></script>
<script src="../src/utils/sigma.polyfills.js"></script>
<script src="../src/sigma.settings.js"></script>
<script src="../src/classes/sigma.classes.dispatcher.js"></script>
<script src="../src/classes/sigma.classes.configurable.js"></script>
<script src="../src/classes/sigma.classes.graph.js"></script>
<script src="../src/classes/sigma.classes.camera.js"></script>
<script src="../src/classes/sigma.classes.quad.js"></script>
<script src="../src/captors/sigma.captors.mouse.js"></script>
<script src="../src/captors/sigma.captors.touch.js"></script>
<script src="../src/renderers/sigma.renderers.canvas.js"></script>
<script src="../src/renderers/sigma.renderers.webgl.js"></script>
<script src="../src/renderers/sigma.renderers.svg.js"></script>
<script src="../src/renderers/sigma.renderers.def.js"></script>
<script src="../src/renderers/webgl/sigma.webgl.nodes.def.js"></script>
<script src="../src/renderers/webgl/sigma.webgl.nodes.fast.js"></script>
<script src="../src/renderers/webgl/sigma.webgl.edges.def.js"></script>
<script src="../src/renderers/webgl/sigma.webgl.edges.fast.js"></script>
<script src="../src/renderers/webgl/sigma.webgl.edges.arrow.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.labels.def.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.hovers.def.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.nodes.def.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edges.def.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edges.curve.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edges.arrow.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edges.curvedArrow.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edgehovers.def.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edgehovers.curve.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edgehovers.arrow.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.edgehovers.curvedArrow.js"></script>
<script src="../src/renderers/canvas/sigma.canvas.extremities.def.js"></script>
<script src="../src/renderers/svg/sigma.svg.utils.js"></script>
<script src="../src/renderers/svg/sigma.svg.nodes.def.js"></script>
<script src="../src/renderers/svg/sigma.svg.edges.def.js"></script>
<script src="../src/renderers/svg/sigma.svg.edges.curve.js"></script>
<script src="../src/renderers/svg/sigma.svg.edges.curvedArrow.js"></script>
<script src="../src/renderers/svg/sigma.svg.labels.def.js"></script>
<script src="../src/renderers/svg/sigma.svg.hovers.def.js"></script>
<script src="../src/middlewares/sigma.middlewares.rescale.js"></script>
<script src="../src/middlewares/sigma.middlewares.copy.js"></script>
<script src="../src/misc/sigma.misc.animation.js"></script>
<script src="../src/misc/sigma.misc.bindEvents.js"></script>
<script src="../src/misc/sigma.misc.bindDOMEvents.js"></script>
<script src="../src/misc/sigma.misc.drawHovers.js"></script>
<!-- END SIGMA IMPORTS -->
<script src="../plugins/sigma.parsers.gexf/gexf-parser.js"></script>
<script src="../plugins/sigma.parsers.gexf/sigma.parsers.gexf.js"></script>
<script src="../plugins/sigma.plugins.animate/sigma.plugins.animate.js"></script>
<script src="../plugins/sigma.layouts.forceLink/worker.js"></script>
<script src="../plugins/sigma.layouts.forceLink/supervisor.js"></script>

<script src="../plugins/sigma.parsers.json/sigma.parsers.json.js"></script>
<script src="../plugins/sigma.plugins.dragNodes/sigma.plugins.dragNodes.js"></script>

<script src="../plugins/sigma.helpers.graph/sigma.helpers.graph.js"></script>
<script src="../plugins/sigma.plugins.activeState/sigma.plugins.activeState.js"></script>
<script src="../plugins/sigma.plugins.select/sigma.plugins.select.js"></script>
<script src="../plugins/sigma.plugins.keyboard/sigma.plugins.keyboard.js"></script>

<script src="../plugins/sigma.renderers.linkurious/settings.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.labels.def.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.hovers.def.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.def.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.cross.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.diamond.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.equilateral.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.square.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.star.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.def.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curve.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.arrow.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curvedArrow.js"></script>
<script src="../plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.autoCurve.js"></script>
<script src="../plugins/sigma.layouts.fruchtermanReingold/sigma.layout.fruchtermanReingold.js"></script>
<script src="../plugins/sigma.plugins.locate/sigma.plugins.locate.js"></script>

<!-- ToolTip Plugin -->
<script src="../plugins/sigma.plugins.tooltips/sigma.plugins.tooltips.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/mustache.js/0.8.1/mustache.min.js"></script>
<script src="../plugins/sigma.plugins.lasso/sigma.plugins.lasso.js"></script>
<script src="../plugins/sigma.plugins.filter/sigma.plugins.filter.js"></script>


<div id="container">
  <style>
    #graph-container {
      top: 0;
      bottom: 0;
      left: 0;
      right: 0;
      position: absolute;
    }

    #layout-notification {
      display: block;
      position: absolute;
      top: 75%;
      left: 1em;
      visibility: hidden;
      font-weight: 200;
      font-family:Roboto;
      color:black;

    }
    #tripal
    {
      font-family:Roboto;
      font-weight:500;
      font-size:40px;
      color: #A0A0A0;
    }




#filter
{
  position:absolute;
  top:15%;
  left:94%;
}


#dataset
{
  border : 1px solid #E0E0E0;
  position:absolute;
  width:65%;
  top:75%;
  left:15%;
  
  z-index : 100;
  background-color:rgba(255,255,255,0.8);
  font-family:Roboto;
 
}

.table1
{
  overflow-y:scroll;
  height:150px;
  background-color:rgba(255,255,255,0.8);
}

th
{
  font-family:Roboto;
  font-weight:200;
}

#get_table
{
  position:absolute;
  top:94%;
  left:1%;
}

#layout-notification {
  border: 16px solid #f3f3f3;
  border-radius: 50%;
  border-top: 16px solid #3498db;
  width: 120px;
  height: 120px;
  -webkit-animation: spin 2s linear infinite;
  animation: spin 2s linear infinite;
}

@-webkit-keyframes spin {
  0% { -webkit-transform: rotate(0deg); }
  100% { -webkit-transform: rotate(360deg); }
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

  </style>

  <div id="graph-container"><span id="layout-notification"></span></div>
  <div id="tripal">Tripal.</div>
<div id="name" style="position:absolute;left:90%;color:#202020;top:1%;">
 <?php 
   if(isset($_POST["submit"]))
   {
     if(strlen($_POST["species"])<8)
     {
       echo "<span style='font-size:30px;font-weight:200;font-family:Roboto'>".strtoupper($species)."</span>";
     }
     else
     {
       echo "<span style='font-size:20px;font-weight:200;font-family:Roboto'>".strtoupper($species)."</span>";
     }
     
   }
 
   
  ?>

   <br />
   <span style="font-size:15px;font-weight:200;font-family:Roboto;"><?php if(isset($_POST["module"])){echo $module; }?></span>
</div>
<div id="filter">
  <button class="pure-button pure-button-primary">Filter</button>
</div>

  <form role="form" id="dataform" method="post" action="force.php" style="background-color:rgba(255,255,255,0.7);width:200px;padding:1.6%;border:0.1px solid #C0C0C0;border-radius:3px;font-weight:300;color:white;position:absolute;top:20%;left:85%;">

   <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Species</label>

     

    <select id="module" class="styled-select slate" name="species" style="color:#202020;;border:1px solid #C0C0C0;width:100%;" onchange="loadDoc(this.value)">
                    <option>Select Species</option>
                    <option>rice</option>
                    <option>arabidopsis</option>
                    <option>maize</option>
                    
                
    </select>
  </div>
  <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Module</label>

     
 
    <select id="modules" name="module" style="color:#202020;border:1px solid #C0C0C0;width:100%;">
                    


                
    </select>
  </div>
 
  <input type="submit" name="submit" value="Get graph" class="pure-button pure-button-primary"/>
</form>

 




<div id="dataset" style="" class="ui-widget-content">
<ul class="nav nav-tabs" style="color:#C0C0C0;">
  <li class="active"><a data-toggle="tab" href="#home" style="text-decoration:none;color:#A0A0A0;font-weight:300;">Edge List</a></li>
  <li><a data-toggle="tab" href="#menu1" style="color:#A0A0A0;font-weight:300;">Node List</a></li>
  <li><a data-toggle="tab" href="#menu2" style="color:#A0A0A0;font-weight:300;">Markers</a></li>
    <li><a data-toggle="tab" href="#menu3" style="color:#A0A0A0;font-weight:300;">Functions</a></li>
    <li><a data-toggle="tab" href="#menu4" style="color:#A0A0A0;font-weight:300;">Current Selection</a></li>
    


</ul>



<div class="tab-content" style="">
  <div id="home" class="tab-pane fade in active">
        <div class="table1" style="transition:all 0.6s ease;">



          
            <table style="width:100%">
                <tr>
                  <?php 
                  if(isset($_POST["submit"]))
                  {

                   echo"<th>Number</th>
                   <th>Source </th>
                   <th>Target</th>
                   <th>Weight</th>
                   <th>Direction</th>
                   <th>Selected Traits</th>";
                 }


                   ?>
                </tr>

                <?php 
                   if(isset($_POST["submit"]))
                   {
                     for($i=0;$i<$edge_count;$i++)
                     {
                        $j=$i+1;
                        $src = $edges[$i]["source"];
                        $source = $nodes[(int)$src];
                        $trg = $edges[$i]["target"];
                        $target = $nodes[(int)$trg];
                        echo "<tr><td>".$j."</td><td>".$source."</td><td>".$target."</td><td>1.5</td><td>undirected</td><td></td></tr>";
                     }
                   }
 
 
                ?>
            </table>
            
        </div>

  </div>
  <div id="menu1" class="tab-pane fade">
    <div class="table1">
            <table style="width:100%;">
               <tr>
                <?php
                 if(isset($_POST["submit"]))
                 { 
                 echo  "<th>Number</th>
                        <th>Node List</th>
                        <th>Functions </th>";
                }
                  ?>
               </tr>
         
              <?php
                if(isset($_POST["submit"])){
                for($i=0;$i< $num;$i++)
                {
                  $j = $i+1;
                  echo "<tr><td>".$j."</td><td>".$nodes[$i]."</td><td>Unknown</td></tr>";
                }

              }
  
              ?>


            </table>
    </div>
  </div>
  <div id="menu2" class="tab-pane fade" style="">
    <div class="table1">
    </div>
  </div>

  <div id="menu3" class="tab-pane fade" style="">
    <div class="table1">
 
        </div>
  </div>

   <div id="menu4" class="tab-pane fade" style="">
    <div class="table1">
          <div id="current_data">
             

          </div>
        </div>
  </div>




</div>



</div>


  <button id="get_table" class="pure-button pure-button-primary">Get Data</button>


<!--
<div id="locator">
   <select id="nodelist">
        <option value="" selected>All nodes</option>
   </select>
   <br /><br />
   <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>

</div>

-->

 <div id="control-pane">
    <h2 class="underline">locate</h2>

    <div>
      <h4>Select Node</h4>
      <select id="nodelist">
        <option value="" selected>All nodes</option>
      </select>
    </div>
   
    <span class="line"></span>
    <div>
      <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
    </div>
  </div>



<div id="info_basic" style="position:absolute;top:90%;left:90%;font-family:Roboto;background-color:rgba(255,255,255,0.7);">
  <span style="font-size:20px"><?php if(isset($_POST["submit"])){echo "Nodes:"; }?></span> <span style="font-size:25px;"><?php if(isset($_POST["submit"])){ echo "  ".$num; }?></span><br />
  <span style="font-size:20px"><?php if(isset($_POST["submit"])){echo "Edges:"; }?></span> <span style="font-size:25px;"><?php if(isset($_POST["submit"])){echo "  ".$edge_count; }  ?> </span>
<div>


</div>
<script>

var $ = function (id) {
  return document.getElementById(id);
};

'use strict';

var initializeGraph = function (sigmaInstance) {
        console.log("Into the function lasso");
        var r= sigmaInstance.graph.read("miss.json");
        sigmaInstance.refresh();
  console.log(r);
        var lasso = new sigma.plugins.lasso(sigmaInstance, sigmaInstance.renderers[0], {
          'strokeStyle': 'gray',
          'lineWidth': 2,
          'fillWhileDrawing': true,
          'fillStyle': 'rgba(41, 41, 41, 0.2)',
          'cursor': 'crosshair'
        });

        console.log("Just before sigma binding");

        // Listen for selectedNodes event
        lasso.bind('selectedNodes', function (event) {
          // Do something with the selected nodes
          var nodes = event.data;

          console.log('nodes', nodes);

          // For instance, reset all node size as their initial size
          sigmaInstance.graph.nodes().forEach(function (node) {
            node.color = 'gray';
            //node.size = 20;
          });

          //List of nodes which are selected
          var datas ="";
          nodes.forEach(function (node) {

            node.color = 'rgb(42, 187, 155)';
            //node.size *= 3;
            console.log(node.label);
            datas=datas + "<br />"+ node.label;
          });

          document.getElementById("current_data").innerHTML = datas;

          sigmaInstance.refresh();
        });

        console.log("End of lasso function");

        return lasso;
      };





//For filters 











//End of filters

sigma.parsers.json('miss.json', {
  // container: 'graph-container',
  renderer: {
    container: document.getElementById('graph-container'),
    type: 'canvas'
  },
  settings: {
    edgeColor: 'default',
    defaultEdgeColor: '#ccc',
    animationsTime: 5000,
    drawLabels: false,
    scalingMode: 'outside',
    batchEdgesDrawing: true,
    hideEdgesOnMove: true,
    sideMargin: 1,
    nodeHoverBorderSize: 3,
    defaultNodeHoverBorderColor: '#A0A0A0',
    nodeActiveBorderSize: 2,
    nodeActiveOuterBorderSize: 3,
    defaultNodeActiveBorderColor: '#A0A0A0',
    defaultNodeActiveOuterBorderColor: 'rgb(236, 81, 72)',
    enableEdgeHovering: true,

  }
}, function(s) {
  s.graph.nodes().forEach(function (n) {
    if (!s.graph.degree(n.id)) {
      s.graph.dropNode(n.id);
    }
    else {
      n.x = Math.random();
      n.y = Math.random();
    }
  });
  s.refresh();
  


  // Configure the ForceLink algorithm:

  var count = <?php echo (int)$edge_count; ?>;
  var grav; 
   if(count>5000)
   {
      grav =40;
   }
   else if(count>3000 && count<=5000)
   {
      grav = 30;
   }
   else if(count>2000 && count<=3000)
   {
      grav = 20;
   }
   else
   {
     grav = 3;
   }


  var fa = sigma.layouts.configForceLink(s, {
    worker: true,
    autoStop: true,
    background: true,
    scaleRatio: 30,
    gravity: grav,
    easing: 'cubicInOut'
      });
    
      // Bind the events:
      fa.bind('start stop', function(e) {
        console.log(e.type);
        document.getElementById('layout-notification').style.visibility = '';
    if (e.type == 'start') {
      document.getElementById('layout-notification').style.visibility = 'visible';
    }


   

  });

  // Start the ForceLink algorithm:
  sigma.layouts.startForceLink({linLogMode:true});


 


var activeState = sigma.plugins.activeState(s);
// Initialize the dragNodes plugin:
var dragListener = sigma.plugins.dragNodes(s, s.renderers[0], activeState);
// Initialize the Select plugin:
var select = sigma.plugins.select(s, activeState);
// Initialize the Keyboard plugin:
var keyboard = sigma.plugins.keyboard(s, s.renderers[0]);
// Bind the Keyboard plugin to the Select plugin:
select.bindKeyboard(keyboard);





//locate

var conf = {
    animation: {
      node: {
        duration: 800
      },
      edge: {
        duration: 800
      },
      center: {
        duration: 300
      }
    },
    //focusOut: true,
    zoomDef: 1
  };
  var locate = sigma.plugins.locate(s, conf);

  locate.setPadding({
    // top:250,
    // bottom: 250,
    right:250,
    // left:250
  });
console.log("locate has been initialized");
  if (!s.settings('autoRescale')) {
    sigma.utils.zoomTo(s.camera, 0, 0, conf.zoomDef);
  }

  var categories = {};

  // read nodes
  var nodelistElt = $('nodelist');
  s.graph.nodes().forEach(function(n) {
    var optionElt = document.createElement("option");
    optionElt.text =n.id;
    nodelistElt.add(optionElt);

    //categories[n.attributes.modularity_class] = true;
  });

  // node category
  console.log("Categories have vbeen filled");

  // reset button
  $('reset-btn').addEventListener("click", function(e) {
    $('nodelist').selectedIndex = 0;
    s.graph.nodes().forEach(function (n) {
      n.color="gray";
    });
    locate.center(conf.zoomDef);
  });

  function locateNode (e) {
    var nid = e.target[e.target.selectedIndex].value;
    if (nid == '') {
      console.log("Locating nodes via center");
      locate.center(1);
    }
    else {
      console.log("Locating nodes");
      s.graph.nodes(nid).color = "rgba(42, 187, 155,0.9)";
      locate.nodes(nid);

      console.log(s.graph.nodes(nid).color);
    }
  };

 

  $('nodelist').addEventListener("change", locateNode);
  

  // just for easy introspection
  //window.s = s;
  //window.locate = locate;




//end of locate




/*
  var activeState = sigma.plugins.activeState(s);

// Initialize the dragNodes plugin:
var dragListener = sigma.plugins.dragNodes(s, s.renderers[0], activeState);

// Initialize the Select plugin:
//var select = sigma.plugins.select(s, activeState);

// Initialize the Keyboard plugin:
var keyboard = sigma.plugins.keyboard(s, s.renderers[0]);
var select = sigma.plugins.select(s, activeState, s.renderers[0]);
select.bindKeyboard(keyboard);

if (sigma.plugins.keyboard) {
  document.getElementsByClassName('container')[0].style.display = 'block';
}





*/


s.bind('clickNode', function(e) {
  console.log(e.type, e.data.node.label, e.data.captor);
  //document.getElementById("data").innerHTML=e.data.node.label;
  //document.getElementById("current_data").innerHTML = e.data.node.label;
  
});


var j = main(s);

// Curve parallel edges:
sigma.canvas.edges.autoCurve(s);
s.refresh();

dragListener.bind('startdrag', function(event) {
  console.log(event);
});
dragListener.bind('drag', function(event) {
  console.log(event);
});
dragListener.bind('drop', function(event) {
  console.log(event);
});
dragListener.bind('dragend', function(event) {
  console.log(event);
});

// Snippet for node location



// Snippet for assigning Tooltips for information about the nodes to the end-user
var config = {
  node: [{
    show: 'hovers',
    hide: 'hovers',
    cssClass: 'sigma-tooltip',
    position: 'top',
    autoadjust: true,
    template:
    '<div class="arrow"></div>' +
    ' <div class="sigma-tooltip-header">{{label}}</div>' +
    '  <div class="sigma-tooltip-body">' +
    '    <table>' +
    '      <tr><th>Feature1</th> <td>{{label}}</td></tr>' +
    '      <tr><th>Feature2</th> <td>{{data.gender}}</td></tr>' +
    '      <tr><th>Feature3</th> <td>{{data.age}}</td></tr>' +
    '      <tr><th>Feature4</th> <td>{{data.city}}</td></tr>' +
    '    </table>' +
    '  </div>' +
    '  <div class="sigma-tooltip-footer">Number of connections: {{degree}}</div>',
    renderer: function(node, template) {
      // The function context is s.graph
      node.degree = this.degree(node.id);

      // Returns an HTML string:
      return Mustache.render(template, node);

      // Returns a DOM Element:
      //var el = document.createElement('div');
      //return el.innerHTML = Mustache.render(template, node);
    }
  }, {
    show: 'rightClickNode',
    cssClass: 'sigma-tooltip',
    position: 'right',
    template:
    '<div class="arrow"></div>' +
    ' <div class="sigma-tooltip-header">{{label}}</div>' +
    '  <div class="sigma-tooltip-body">' +
    '   <p> Context menu for {{label}} </p>' +
    '  </div>' +
    ' <div class="sigma-tooltip-footer">Number of connections: {{degree}}</div>',
    renderer: function(node, template) {
      node.degree = this.degree(node.id);
      return Mustache.render(template, node);
    }
  }],
  stage: {
    template:
    '<div class="arrow"></div>' +
    '<div class="sigma-tooltip-header"> Menu </div>'
  }
};

// Instanciate the tooltips plugin with a Mustache renderer for node tooltips:
var tooltips = sigma.plugins.tooltips(s, s.renderers[0], config);


// Manually open a tooltip on a node:
var n = s.graph.nodes('n10');
var prefix = s.renderers[0].camera.prefix;
tooltips.open(n, config.node[0], n[prefix + 'x'], n[prefix + 'y']);


tooltips.bind('shown', function(event) {
  console.log('tooltip shown', event);
});

tooltips.bind('hidden', function(event) {
  console.log('tooltip hidden', event);
});









});


var main = function(sig)
{
  //firstLasso = initializeGraph(sig);
  console.log("Into function main");
  var firstLasso = initializeGraph(sig);
  
document.addEventListener('keyup', function (event) {
  console.log("Into addEventListener");
  switch (event.keyCode) {
    case 76:
      if (event.altKey) {
        if (firstLasso.isActive) {
          firstLasso.deactivate();
          console.log("Deactivated");
        } else {
          firstLasso.activate();
          console.log("Activated");
        }
      }
      break;
 
  }
});
 return 1;
}



</script>
</html>



  <script src="//code.jquery.com/ui/1.11.4/jquery-ui.js"></script>
    <link rel="stylesheet" href="//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css">


<script type="text/javascript">

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

  xhttp.onreadystatechange=function()
  {
    if(xhttp.readyState==4 && xhttp.status==200)
    {
      document.getElementById("modules").innerHTML=xhttp.responseText;
      


    }
  };
  xhttp.open("POST","retrieve.php?q="+str,true);
  xhttp.send();
 }
 </script>


<script>
console.log("Into the laast script");
var $j = jQuery.noConflict();
$j(document).ready(function(){
    $j("#filter").click(function(){
      console.log("Filter is clicked");
        $j("#dataform").slideToggle("fast");
    });
});


$j(document).ready(function(){
    $j("#get_table").click(function(){
        $j("#dataset").slideToggle("fast");
    });
});

 console.log("Slide Toggle is done");
  $j(function() {
    $j( "#dataform" ).draggable();
    console.log("Draggable is enabled");
    $j("#dataset").draggable();
    $j(".table1").resizable();
  });


</script>

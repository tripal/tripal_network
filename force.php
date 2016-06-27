<?php

//PHP script for querying Neo4j with the appropriate matching format and also 
//Conversion of the returned format to the format required by D3.js for rendering purpose


//echo $_POST["module"];


ini_set('max_execution_time', 300);

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
  $temp["color"]=$colors[$val];
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

</style>



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
      top: 90%;
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

  </style>

  <div id="graph-container"><span id="layout-notification">Layout in progress...</span></div>
  <div id="tripal">Tripal</div>
<div id="name" style="position:absolute;left:90%;color:#202020;top:1%;">
   <span style="font-size:30px;font-weight:200;font-family:Roboto;"><?php if(isset($_POST["species"])){echo strtoupper($species); }?> </span><br />
   <span style="font-size:15px;font-weight:200;font-family:Roboto;"><?php if(isset($_POST["module"])){echo $module; }?></span>
</div>
<div id="filter">
  <button class="pure-button pure-button-primary">Filter</button>
</div>

  <form role="form" id="dataform" method="post" action="force.php" style="background-color:rgba(255,255,255,0.7);width:200px;padding:1.6%;border:0.1px solid #C0C0C0;border-radius:3px;font-weight:300;color:white;position:absolute;top:20%;left:85%;">

   <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Species</label>

     

    <select id="module" name="species" style="color:#202020;;border:1px solid #C0C0C0;width:100%;">
                    <option>rice</option>
                    <option>arabidopsis</option>
                    <option>maize</option>
                    
                
    </select>
  </div>
  <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Module</label>

     

    <select id="module" name="module" style="color:#202020;border:1px solid #C0C0C0;width:100%;">
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
                    <option>Module22</option>
                    <option>Module23</option>
                    <option>Module24</option>
                    <option>Module25</option>
                    <option>Module26</option>
                    <option>Module27</option>
                    <option>Module28</option>
                    <option>Module29</option>
                    <option>Module30</option>
                    <option>Module31</option>
                    <option>Module32</option>
                    <option>Module33</option>


                
    </select>
  </div>
 
  <input type="submit" name="submit" value="Get graph" class="pure-button pure-button-primary"/>
</form>





<div id="dataset" style="">
<ul class="nav nav-tabs" style="color:#C0C0C0;">
  <li class="active"><a data-toggle="tab" href="#home" style="text-decoration:none;color:#A0A0A0;font-weight:300;">Edge List</a></li>
  <li><a data-toggle="tab" href="#menu1" style="color:#A0A0A0;font-weight:300;">Node List</a></li>
  <li><a data-toggle="tab" href="#menu2" style="color:#A0A0A0;font-weight:300;">Markers</a></li>
    <li><a data-toggle="tab" href="#menu3" style="color:#A0A0A0;font-weight:300;">Functions</a></li>

</ul>

<div class="tab-content" >
  <div id="home" class="tab-pane fade in active">
        <div class="table1">
            <table style="width:100%">
                <tr>
                   <th>Number</th>
                   <th>Source </th>
                   <th>Target</th>
                   <th>Weight</th>
                   <th>Direction</th>
                   <th>Selected Traits</th>
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
                  <th>Number</th>
                  <th>Node List</th>
                  <th>Functions </th>
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
</div>

</div>


  <button id="get_table" class="pure-button pure-button-primary">Get Data</button>


</div>
<script>


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
    sideMargin: 1
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
});


</script>
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
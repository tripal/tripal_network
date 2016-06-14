<?php

//PHP script for querying Neo4j with the appropriate matching format and also
//Conversion of the returned format to the format required by D3.js for rendering purpose


//echo $_POST["module"];

// Matching criteria for querying into Neo4j
if(isset($_POST["submit"]))
{


  $status=0;
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



$num = count($nodes);
$n=0;
$node=array();

// Storage of the nodes in the proper format with the correct indexing
for($x=0;$x<$num;$x++)
{
  $temp = array();
  $temp["x"]=(mt_rand()/mt_getrandmax())*400 +150;
  $temp["y"]=(mt_rand()/mt_getrandmax())*300+100;
  $temp["r"]=10;
  $temp["weight"]=20;
  $temp["color"]='#43a2ca';

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

    $key = array_search($row[1],$nodes);
    $temp["from"]= $key;
    $source=$key;
    $key = array_search($row[2],$nodes);
    $temp["to"]=$key;
    $target=$key;
    $temp["source"]=$node[$source];
    $temp["target"]=$node[$target];
    $temp["color"]='#777777';



    $edges[$e]=$temp;
    $e++;

}













$return=array('nodes'=>$node,'links'=>$edges);

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
<script type="text/javascript" src="center.js"></script>
<script src="grapher.js"></script>
<script src="grapher.min.js"></script>
<script src="zoom.js"></script>

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

<script>

 var width=1200,height=600,i;

      var network = <?php echo $jsonarray; ?>

      var getNodeIdAt = function (point) {
        var node = -1,
            x = point.x, y = point.y;

        network.nodes.every(function (n, i) {
          var inX = x <= n.x + n.r && x >= n.x - n.r,
              inY = y <= n.y + n.r && y >= n.y - n.r,
              found = inX && inY;
          if (found) node = i;
          return !found;
        });

        return node;
      };

      // Helper function for offsets.
      function getOffset (e) {
        if (e.offsetX) return {x: e.offsetX, y: e.offsetY};
        var rect = e.target.getBoundingClientRect();
        var x = e.clientX - rect.left,
            y = e.clientY - rect.top;
        return {x: x, y: y};
      };

      // Create a grapher instance (width, height, options)
      var grapher = new Grapher({
        width: width,
        height: height
      });

      // Give it the network data
      grapher.data(network);

      // Variable to keep track of the node we're dragging and the current offset
      var dragging = null,
          offset = null;


      // onTick gets called on each tick of D3's force
      var onTick = function () {
        if (dragging && offset) {
          // update the node's position here so it's sticky
          dragging.node.x = offset.x;
          dragging.node.y = offset.y;
        }
        grapher.update(); // update the grapher
      };



      // Setup D3's force layout
      var force = d3.layout.force()
          .nodes(network.nodes)
          .links(network.links)
          .size([width, height])
          .on('tick', onTick)
          .charge(-60)
          .gravity(0.002)
          .linkStrength(0.2)
          .linkDistance(0.002)
          .friction(0.3)
          .start();



      // On mousedown, grab the node that was clicked.
      grapher.on('mousedown', function (e) {
        var eOffset = getOffset(e);
        var point = grapher.getDataPosition(eOffset);
        var nodeId = getNodeIdAt(point);
        if (nodeId > -1) {
          dragging = {node: network.nodes[nodeId], id: nodeId};
          offset = point;
          document.getElementById("data").innerHTML=nodeId;
          //Use the ID from here to parse get the required data that has to be displayed

        }
        else dragging = offset = null;
      });

      // When the user moves the mouse, we update the node's position
      grapher.on('mousemove', function (e) {
        var eOffset = getOffset(e);
        var point = grapher.getDataPosition(eOffset);
        if (dragging) {
          offset = point;
          force.alpha(1); // restart the force graph
        }
      });

      // Finally when the user lets go of the mouse, we stop dragging
      grapher.on('mouseup', function (e) { dragging = offset = null; });

      //The following will do the pan function for the canvas
      //
      /*
      var startPoint;

      function onMouseDown (e) {
        // Set the starting point
        startPoint = getOffset(e);

        // Start listening to other mouse events.
        grapher.on('mousemove', onMouseMove);
        grapher.on('mouseup', onMouseUp);
      };

      function onMouseMove (e) {
        // Adjust the translate based on the change in mouse location.
        if (startPoint) {
          var translate = grapher.translate(),
              offset = getOffset(e);

          translate[0] += (offset.x - startPoint.x);
          translate[1] += (offset.y - startPoint.y);

          startPoint = offset;
          grapher.translate(translate);
        }
      };

      function onMouseUp (e) {
        // Stop listening to mouse events, and cleanup startPoint
        startPoint = undefined;
        grapher.off('mousemove');
        grapher.off('mouseup');
      };

      grapher.on('mousedown', onMouseDown);

      */

      // Append the grapher's view onto the page
      document.body.appendChild(grapher.canvas);

      //Coloring the graph
      grapher.color();



      // Render the graph using play. This will call render in a requestAnimationFrame loop.
      grapher.play();

      //document.write(JSON.stringify(network));



      //Function for zooming in and out with mouse wheel
      grapher.on('wheel',function(e)
      {
         var center=getOffset(e);
         var delta=e.deltaY/50;

         //Calling zoom with ratio and center

         grapher.zoom(1+delta,center);
         grapher.play();

      });











</script>








</div>

<div id="data" style="background-color:#E0E0E0;width:100px"></div>
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


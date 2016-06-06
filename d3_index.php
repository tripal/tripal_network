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


<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css" integrity="sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7" crossorigin="anonymous">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/2.2.3/jquery.min.js"></script>
<!-- Optional theme -->
<!-- Optional theme -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap-theme.min.css" integrity="sha384-fLW2N01lMqjakBkx3l/M9EahuwpSfeNvV63J5ezn3uZzapT0u7EYsXMjQV+0En5r" crossorigin="anonymous">


<!-- Latest compiled and minified JavaScript -->
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js" integrity="sha384-0mSbJDEHialfmuBBQP6A4Qrprq5OVfW37PRR3j5ELqxss1yVqOtnepnHVP9aJ7xS" crossorigin="anonymous"></script>

<!--  <link rel="stylesheet" href="css/style.css" /> -->

<link rel="stylesheet" href="tablecss.CSS" />

<style>

body {
    padding-top: 70px; /* Required padding for .navbar-fixed-top. Change if height of navigation changes. */

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
                    <li>
                        <a href="#">About</a>
                    </li>
                    <li>
                        <a href="#">Services</a>
                    </li>
                    <li>
                        <a href="#">Contact</a>
                    </li>
                </ul>
            </div>
            <!-- /.navbar-collapse -->
        </div>
       
    </nav>
  
<script src="//d3js.org/d3.v3.min.js"></script>

<!-- This is the script that will render the given data into the visualization format -->
<script>

var width = 1200,
    height = 600;

 var color = d3.scale.category20();

var force = d3.layout.force()
    .charge(-200)
    .linkDistance(45)
    .size([width, height]);

  var randomX = d3.random.normal(width / 2, 80),
    randomY = d3.random.normal(height / 2, 80);

var data = d3.range(2000).map(function() {
  return [
    randomX(),
    randomY()
  ];
});

var svg = d3.select("body").append("svg")
    .attr("width", width)
    .attr("height", height).attr("id","svg-id").append("g").call(d3.behavior.zoom().scaleExtent([1,8]).on("zoom",zoom)).append("g");

d3.json("mis.json", function(error, graph) {
  if (error) throw error;

  force
      .nodes(graph.nodes)
      .links(graph.links)
      .start();

  var link = svg.selectAll(".link")
      .data(graph.links)
    .enter().append("line")
      .attr("class", "link")
      .style("stroke-width", function(d) { return Math.sqrt(d.value); });

  var node = svg.selectAll(".node")
      .data(graph.nodes)
    .enter().append("g")
      .attr("class", "node")
      .style("fill", function(d) { return color(d.group); })
      .call(force.drag);
node.append("circle").attr("r",7);
  node.append("text")
      .attr("dx", 12)
      .attr("dy", ".35em")
      .text(function(d) { return d.name });

  force.on("tick", function() {
    link.attr("x1", function(d) { return d.source.x; })
        .attr("y1", function(d) { return d.source.y; })
        .attr("x2", function(d) { return d.target.x; })
        .attr("y2", function(d) { return d.target.y; });

   node.attr("transform", function(d)
{
return "translate(" + d.x + "," + d.y + ")";
});
  


  });

 

});

function zoom() {
  svg.attr("transform", "translate(" + d3.event.translate + ")scale(" + d3.event.scale + ")");
}



</script>

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

               <form method="post" action="retrieve.php" id="species">
                 <select id="module" name="module">
                    <option>Rice</option>
                    <option>Arabidopsis</option>
                    <option>Orza Sativa</option>
                
                 </select>
                
                 <input type="submit" />

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

 

<button type="button" class="btn btn-info btn-lg" data-toggle="modal" data-target="#myModal" id="genomic-view" >Genomic View</button>

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

</body>


</html>


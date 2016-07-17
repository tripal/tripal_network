<?php

//PHP script to load the whole network from Neo4j into the browser
//This software is built on the top of Sigma.js with certain modifications
//It relies on the forceLink layout algorithm for the layout of the structure
//The edges are all rendered together but with a loader which ensures that the script runs throughout without failing
//Some plugins from linkurious are also imported for further data analytics from the developed graph

include "php/header.php";
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



<link rel="stylesheet" href="theme/css/tripal_network.css" />
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
<script src="src/renderers/svg/sigma.svg.edges.curvedArrow.js"></script>
<script src="src/renderers/svg/sigma.svg.labels.def.js"></script>
<script src="src/renderers/svg/sigma.svg.hovers.def.js"></script>
<script src="src/middlewares/sigma.middlewares.rescale.js"></script>
<script src="src/middlewares/sigma.middlewares.copy.js"></script>
<script src="src/misc/sigma.misc.animation.js"></script>
<script src="src/misc/sigma.misc.bindEvents.js"></script>
<script src="src/misc/sigma.misc.bindDOMEvents.js"></script>
<script src="src/misc/sigma.misc.drawHovers.js"></script>
<!-- END SIGMA IMPORTS -->
<script src="plugins/sigma.parsers.gexf/gexf-parser.js"></script>
<script src="plugins/sigma.parsers.gexf/sigma.parsers.gexf.js"></script>
<script src="plugins/sigma.plugins.animate/sigma.plugins.animate.js"></script>
<script src="plugins/sigma.layouts.forceLink/worker.js"></script>
<script src="plugins/sigma.layouts.forceLink/supervisor.js"></script>

<script src="plugins/sigma.parsers.json/sigma.parsers.json.js"></script>
<script src="plugins/sigma.plugins.dragNodes/sigma.plugins.dragNodes.js"></script>

<script src="plugins/sigma.helpers.graph/sigma.helpers.graph.js"></script>
<script src="plugins/sigma.plugins.activeState/sigma.plugins.activeState.js"></script>
<script src="plugins/sigma.plugins.select/sigma.plugins.select.js"></script>
<script src="plugins/sigma.plugins.keyboard/sigma.plugins.keyboard.js"></script>

<script src="plugins/sigma.renderers.linkurious/settings.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.labels.def.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.hovers.def.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.def.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.cross.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.diamond.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.equilateral.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.square.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.star.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.def.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curve.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.arrow.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curvedArrow.js"></script>
<script src="plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.autoCurve.js"></script>
<script src="plugins/sigma.layouts.fruchtermanReingold/sigma.layout.fruchtermanReingold.js"></script>
<script src="plugins/sigma.plugins.locate/sigma.plugins.locate.js"></script>

<!-- ToolTip Plugin -->
<script src="plugins/sigma.plugins.tooltips/sigma.plugins.tooltips.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/mustache.js/0.8.1/mustache.min.js"></script>
<script src="plugins/sigma.plugins.lasso/sigma.plugins.lasso.js"></script>
<script src="plugins/sigma.plugins.filter/sigma.plugins.filter.js"></script>


<body>
  
<?php 
include "php/html_wrap.php";
?>

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
    optionElt.text =n.id + " " + n.label;
    console.log(n.label);
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
    var tmp = (e.target[e.target.selectedIndex].value).split(" ");
    var nid = tmp[0];

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


<script type="text/javascript" src="theme/js/ajax.js"></script>


<script type="text/javascript" src="theme/js/dynamic.js"></script>

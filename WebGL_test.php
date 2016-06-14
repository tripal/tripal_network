<!DOCTYPE html>
<html>
<head>
 <title>WebGL testing</title>
</head>
<script src="http://d3js.org/d3.v3.min.js" charset="utf-8"></script>
<script src="grapher.js"></script>
<script src="grapher.min.js"></script>
<body>

  <script>


      // Generate some data



      var network = {nodes: [], links: []},
          width = 2400,
          height = 1200,
          numNodes = 1000,
          numLinks = 2000,
          i;

      for (i = 0; i < numNodes; i++) {
        network.nodes.push({
          x: Math.random() * width,
          y: Math.random() * height,
          r: Math.random() * 8,
          weight: 1
        });
      }

      for (i = 0; i < numLinks; i++) {
        var from = Math.floor(Math.random() * numNodes),
            to = Math.floor(Math.random() * numNodes);
        network.links.push({
          from: from,
          to: to,
          source: network.nodes[from],
          target: network.nodes[to]
        });
      }


      //var network = {"nodes":[{"x":340,"y":340,"weight":3},{"x":390,"y":390,"weight":4}],"links":[{"from" :340,"to":390}]};
      // We create a function that determines whether a click event falls on a node.

     //var width=1200,height=600,i;
    //var network = {"nodes":[{"x":340,"y":340,"weight":3,"r":13},{"x":390,"y":390,"weight":4,"r":14}],"links":[{"from" :0,"to":1,"source":{"x":340,"y":340,"weight":3,"r":13},"target":{"x":390,"y":390,"weight":4,"r":14}}]};
      //var network = {"nodes":[{"name":"1","group":2},{"name":"9","group":2},{"name":"10","group":2},{"name":"11","group":2},{"name":"13","group":2},{"name":"22","group":2},{"name":"24","group":2}],"links":[{"source":3,"target":2,"value":3},{"source":0,"target":2,"value":3},{"source":6,"target":4,"value":3},{"source":1,"target":4,"value":3},{"source":1,"target":5,"value":3}]};
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
          .charge(-500)
          .gravity(0.005)
          .linkStrength(0.2)
          .linkDistance(5)
          .friction(0.02)
          .start();

      // On mousedown, grab the node that was clicked.
      grapher.on('mousedown', function (e) {
        var eOffset = getOffset(e);
        var point = grapher.getDataPosition(eOffset);
        var nodeId = getNodeIdAt(point);
        if (nodeId > -1) {
          dragging = {node: network.nodes[nodeId], id: nodeId};
          offset = point;
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

      // Append the grapher's view onto the page
      document.body.appendChild(grapher.canvas);

      // Render the graph using play. This will call render in a requestAnimationFrame loop.
      grapher.play();

      document.write(JSON.stringify(network));

  </script>

  <!-- <canvas height="400px" width="400px"></canvas> -->
	</body>

</html>

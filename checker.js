var width = 1200,
    height = 600;

 var color = d3.scale.category20();

var force = d3.layout.force()
    .charge(-100)
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



d3.json("miss.json", function(error, graph) {
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

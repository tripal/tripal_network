(function($) {

  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {
        
    // TODO: describe this variable.
    //var $ = document.getElementById(id);

    // Add a click response to open and close the panels.
    $('.toggle-header').click(function(){
      $(this).parent().find('.toggle-content').slideToggle('fast');
    });

    // Use JQuery UI to format the data panel with tabs.
    $("#tripal-network-viewer-data-panel").tabs();
    
    //console.log("Into the laast script");
    //var $j = jQuery.noConflict();
    $("#filter").click(function(){
      //console.log("Filter is clicked");
        $("#tripal-network-viewer-filter-form").slideToggle("fast");
    });

    $("#get_table").click(function(){
      $("#dataset").slideToggle("fast");
    });

    // console.log("Slide Toggle is done");
    $(function() {
      $("#tripal-network-viewer-filter-form").draggable();
      //console.log("Draggable is enabled");
      $("#dataset").draggable();
      $(".table1").resizable();
      $("#control-pane").draggable();
   });

    /**
     * Use SigmaJS to load the network onto the canvas.
     */
    sigma.parsers.json(
      network_url + '/miss.json', 
      {
        // Indicate the object for graph rendering.
        renderer: {
          container: document.getElementById('graph-container'),
          type: 'canvas'
        },
        // Settings for SigmaJS.
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
        }, 
        // TODO: what is this
        function(s) {

          // Set the default x,y coordinate.  
          s.graph.nodes().forEach(function (n) {
            if (!s.graph.degree(n.id)) {
              s.graph.dropNode(n.id);
            }
            else {
              n.x = Math.random();
              n.y = Math.random();
            }
            n.color = '#0000FF';
          });
          s.refresh();

          // Set the gravity according to the number of nodes.
          var count = 0; //<?php echo (int)$edge_count; ?>;
          var grav; 
          if (count > 5000) {
            grav = 40;
          }
          else if (count > 3000 && count <= 5000) {
            grav = 30;
          }
          else if (count > 2000 && count <= 3000) {
            grav = 20;
          }
          else {
            grav = 3;
          }
          
          // Create an initial object for the forced link layout.
          var fa = sigma.layouts.configForceLink(s, {
            worker: true,
            autoStop: true,
            background: true,
            scaleRatio: 30,
            gravity: grav,
            easing: 'cubicInOut'
          });
      
          // When the forced link layout event is 'start stop', then we want to
          // make a spinning wheel visible to the user. This function will
          // be called anytime the 'fa' object starts or stops the layout.
          fa.bind('start stop', function(e) {
            //console.log(e.type);
            // By default the spinning wheel is invisible but on 'start' it 
            // is made visible.
            document.getElementById('layout-notification').style.visibility = '';
            if (e.type == 'start') {
              document.getElementById('layout-notification').style.visibility = 'visible';
            }
          });
      
          // Start the ForceLink algorithm:
          sigma.layouts.startForceLink({linLogMode:true});
      
          // TODO: add an explanation for each of these variables. 
          var activeState = sigma.plugins.activeState(s);
          // Initialize the dragNodes plugin:
          var dragListener = sigma.plugins.dragNodes(s, s.renderers[0], activeState);
          // Initialize the Select plugin:
          var select = sigma.plugins.select(s, activeState);
          // Initialize the Keyboard plugin:
          var keyboard = sigma.plugins.keyboard(s, s.renderers[0]);
          // Bind the Keyboard plugin to the Select plugin:
          select.bindKeyboard(keyboard);

          // Locate.
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

          //console.log("locate has been initialized");

          if (!s.settings('autoRescale')) {
            sigma.utils.zoomTo(s.camera, 0, 0, conf.zoomDef);
          }

          var categories = {};

          // read nodes
          var nodelistElt = $('nodelist');
          s.graph.nodes().forEach(function(n) {
            var optionElt = document.createElement("option");
            optionElt.text =n.id + " " + n.label;
            //console.log(n.label);
            nodelistElt.add(optionElt);
            // categories[n.attributes.modularity_class] = true;
          });

          // node category
          //console.log("Categories have vbeen filled");

          // reset button
//          $('#reset-btn').addEventListener("click", function(e) {
//            $('nodelist').selectedIndex = 0;
//            s.graph.nodes().forEach(function (n) {
//              n.color="gray";
//            });
//            locate.center(conf.zoomDef);
//          });

          function locateNode (e) {
            var tmp = (e.target[e.target.selectedIndex].value).split(" ");
            var nid = tmp[0];

            if (nid == '') {
              //console.log("Locating nodes via center");
              locate.center(1);
            }
            else {
              //console.log("Locating nodes");
              s.graph.nodes(nid).color = "rgba(42, 187, 155,0.9)";
              locate.nodes(nid);
      
              //console.log(s.graph.nodes(nid).color);
            }
          };

//          $('nodelist').addEventListener("change", locateNode);

          s.bind('clickNode', function(e) {
            //console.log(e.type, e.data.node.label, e.data.captor);        
          });

          // Add a lasso to the this object and add an event
          var firstLasso = createLasso(s);
          var j = addLassoActivateEvent(s, firstLasso);

          // Curve parallel edges:
          sigma.canvas.edges.autoCurve(s);
          s.refresh();

          dragListener.bind('startdrag', function(event) {
            //console.log(event);
          });

          dragListener.bind('drag', function(event) {
            //console.log(event);
          });
          dragListener.bind('drop', function(event) {
            //console.log(event);
          });
          dragListener.bind('dragend', function(event) {
            //console.log(event);
          });

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
//                return Mustache.render(template, node);
        
                // Returns a DOM Element:
                //var el = document.createElement('div');
                //return el.innerHTML = Mustache.render(template, node);
              }
           }, 
           {
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
         var n = s.graph.nodes('10');
         var prefix = s.renderers[0].camera.prefix;
         tooltips.open(n, config.node[0], n[prefix + 'x'], n[prefix + 'y']);    

         tooltips.bind('shown', function(event) {
           //console.log('tooltip shown', event);
         });

         tooltips.bind('hidden', function(event) {
           //console.log('tooltip hidden', event);
        });
      }
    ); // END sigma.parsers.json()

  }} // End Drupal.behaviors.tripal_network.
  
  /**
   * Adds an event handler for the lasso.
   * 
   * When our document loads we want the lass selection tool to work.
   * This code activates the lasso when a keyup event is captured with
   * the key combination of alt + l.
   * 
   * @param sigmaInstance
   *   An instance of a sigmaJS object.
   * @param lasso
   *   An instance of a lasso object.
   */
  var addLassoActivateEvent = function(sigmaInstance, lasso) {

    document.addEventListener('keyup', function (event) {
      //console.log("Into addEventListener");
      switch (event.keyCode) {
        case 76:
          if (event.altKey) {
            if (lasso.isActive) {
              firstLasso.deactivate();
              //console.log("Deactivated");
            } 
            else {
              lasso.activate();
              //console.log("Activated");
            }
          }
          break;
      }
    })
  };
  
  /**
   * Create the lasso object.
   * 
   * The Lasso is a linkurious plugin that allows for selection of nodes
   * in the graph.  This function is a wrapper for quick instanation of
   * a new lasso on a sigmaJS object.
   * 
   * @param sigmaInstance
   *   An instance of a sigmaJS object.
   *   
   * @return
   *   A instance of a lasso object.
   */
  var createLasso = function (sigmaInstance) {
    
    // Read the network
    //console.log("Into the function lasso");
    // TODO: find some otherway to get the graph without calling another
    // AJAX call. 
    var r = sigmaInstance.graph.read(network_url + "/miss.json");
    sigmaInstance.refresh();
    ////console.log(r);
  
    var lasso = new sigma.plugins.lasso(sigmaInstance, sigmaInstance.renderers[0], {
      'strokeStyle': 'gray',
      'lineWidth': 2,
      'fillWhileDrawing': true,
      'fillStyle': 'rgba(41, 41, 41, 0.2)',
      'cursor': 'crosshair'
    });
    //console.log("Just before sigma binding");
  
    // Listen for selectedNodes event.
    lasso.bind('selectedNodes', function (event) {
  
    // Do something with the selected nodes.
    var nodes = event.data;
  
    //console.log('nodes', nodes);
  
    // For instance, reset all node size as their initial size.
    sigmaInstance.graph.nodes().forEach(function (node) {
      node.color = 'gray';
      // node.size = 20;
    });
  
    // List of nodes which are selected.
    var datas ="";
    nodes.forEach(function (node) {
      node.color = 'rgb(42, 187, 155)';
      // node.size *= 3;
      //console.log(node.label);
      datas=datas + "<br />"+ node.label;
    });
  
    $("#current_data").innerHTML = datas;
      sigmaInstance.refresh();
    });
  
    //console.log("End of lasso function");
    return lasso;
  };

})(jQuery);
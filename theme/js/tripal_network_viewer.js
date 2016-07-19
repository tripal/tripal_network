(function($) {

  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {
        
    
    //This is a DOM utility function and it is used by locate plugin
    //var $ = function (id) {
     //return document.getElementById(id);
    //};

    $('.bg-color-picker').click(function(){
      $('body').css('background-color', $(this).css('background-color'));
    });
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
          defaultEdgeColor: '#888888',
          animationsTime: 5000,
          drawLabels: false,
          scalingMode: 'outside',
          batchEdgesDrawing: true,
          hideEdgesOnMove: true,
          sideMargin: 1,
          nodeBorderSize: 1,
          nodeBorderColor: '#000',
          nodeHoverBorderSize: 3,
          defaultNodeHoverBorderColor: 'yellow',
          nodeActiveBorderSize: 2,
          nodeActiveOuterBorderSize: 3,
          defaultNodeActiveBorderColor: 'yellow',
          defaultNodeActiveOuterBorderColor: 'yellow',
          enableEdgeHovering: true,
          defaultNodeType: 'border'
        },
      }, 
  /**
   * This function is scaffolding all the sub-functions needed for visualizations
   * It is passed with an argument 's' , which is needed by all the sub-functions to execute
   * It is defined according to the syntax of sigma.parsers.json
  */
      function(s) {


        var max_degree = 0;
        s.graph.nodes().forEach(function (n) {
          degree = s.graph.degree(n.id);
          if (degree > max_degree) {
            max_degree = degree;
          }
        });

        // Set the default x,y coordinate.  
        s.graph.nodes().forEach(function (n) {
          if (!s.graph.degree(n.id)) {
            s.graph.dropNode(n.id);
          }
          else {
            n.x = Math.random();
            n.y = Math.random();
          }
          degree = s.graph.degree(n.id);
          //n.color = "rgb(255,140,0)";
          cscale = chroma.scale(['yellow', 'orange', 'red']).domain([1, max_degree]);
          n.color = cscale(degree);
          n.size = degree * 2;
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




      
          // This plugin provides a new state called active to nodes and edges. 
          //Graphical elements in user interfaces have usually multiple states like hidden, hover, focus, and active. 
          //They are keys to enable great user interaction. 
          //For instance, one might create a plugin that drag all active nodes, which are eventually considered as "selected" elements. 
          var activeState = sigma.plugins.activeState(s);

          // Initialize the dragNodes plugin:
          //This needs the nodes to be in activeState
          var dragListener = sigma.plugins.dragNodes(s, s.renderers[0], activeState);

          // Initialize the Select plugin:
          var select = sigma.plugins.select(s, activeState);

          // Initialize the Keyboard plugin:
          var keyboard = sigma.plugins.keyboard(s, s.renderers[0]);


          // Bind the Keyboard plugin to the Select plugin:
          select.bindKeyboard(keyboard);







  /** Locate function
  * Responsible for populating a drop-down menu with the loaded nodes
  * On selection from dropdown, the selected node will be zoomed into
  * The color of selected node will also the changed to green

  */

          // Setting the configuration for zooming to the selected node
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
          
          // Initializing the instance for locate
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

          // This will read the nodes which have been loaded
          // The label and id of the node will be populated in the dropdown
          var nodelistElt = document.getElementById("nodelist");
          s.graph.nodes().forEach(function(n) {
            var optionElt = document.createElement("option");
            optionElt.text =n.id + " " + n.label;
            console.log(n.label);
            nodelistElt.add(optionElt);
            // categories[n.attributes.modularity_class] = true;
          });

         

          // Reset button
          //On 'reset-view' being clicked, the following function is triggered
          //This will reset the view along with coloring all the nodes gray
          //This will zoom out using the configuration settings, so that all nodes are seen

          document.getElementById("reset-btn").addEventListener("click", function(e) {
            document.getElementById("nodelist").selectedIndex = 0;
//            s.graph.nodes().forEach(function (n) {
//              n.color="gray";
//            });
            locate.center(conf.zoomDef);
          });


          //This function is responsible for zooming into the selected node
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

          document.getElementById("nodelist").addEventListener("change", locateNode);



  /**
   * End of locate function 
  */


  
          //This function is not completed, but created for future
          //On click of any node, this function will get activated
          s.bind('clickNode', function(e) {
            //console.log(e.type, e.data.node.label, e.data.captor);  

            //Action on clickNode has to be written here

          });

          // Add a lasso to the this object and add an event
          var firstLasso = createLasso(s);
          //Activating the lassoActivateEvent
          var j = addLassoActivateEvent(s, firstLasso);

          // Curve parallel edges: 
          // This is to distinguish between interconnections
          sigma.canvas.edges.autoCurve(s);
          s.refresh();

  /**
   *  The following event bindings are responsible for dragging the nodes
   *  On a node being clicked and dragged, the following event bindings are activated
  */



          //On the node being clicked, startdrag is activated
          dragListener.bind('startdrag', function(event) {
            //console.log(event);
          });
          //This is activated during the process of dragging
          dragListener.bind('drag', function(event) {
            //console.log(event);
          });
          //This is activated when the user just stops dragging
          dragListener.bind('drop', function(event) {
            //console.log(event);
          });
          //Activated just after drop event binding to end dragging
          dragListener.bind('dragend', function(event) {
            //console.log(event);
          });

  /**
    * End of dragging functionality
  */




  /**
   * The following code is responsible for assigning tool-tips to the nodes
   * In the first part, the configuration settings for the tooltips are set
   * In the second part, a tooltip instance is getting created
   * This tooltip instance is getting binded by events then
   * For rendering of tooltips, Mustache is used
  */


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
              lasso.deactivate();
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
      'fillStyle': 'rgba(100, 100, 100, 0.5)',
      'cursor': 'crosshair'
    });
    //console.log("Just before sigma binding");
  
    // Listen for selectedNodes event.
    lasso.bind('selectedNodes', function (event) {
  
    // Do something with the selected nodes.
    var nodes = event.data;
    //console.log('nodes', nodes);
  
    // List of nodes which are selected.
    var datas ="";
    nodes.forEach(function (node) {
      node.active = true;
      //datas=datas + "<br />"+ node.label;
    });
  
    $("#current_data").innerHTML = datas;
      sigmaInstance.refresh();
    });
  
    //console.log("End of lasso function");
    return lasso;
  };

})(jQuery);

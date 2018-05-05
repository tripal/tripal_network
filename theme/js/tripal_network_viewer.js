(function($) {

  // Globals
  // Holds the network data (edges and nodes)
  var Network_Data;
  
  // An instance of a sigmaJS object.
  var Sigma_Instance;


  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {
    } 
  } 

  // Code that must be executed when the document is ready and never again
  // goes in this section
  $(document).ready(function() {
    $('.bg-color-picker').click(function(){
      $('body').css('background-color', $(this).css('background-color'));
    });
    
    $('#tripal-network-viewer-accordion').css("height", $(document).height());
    $("#tripal-network-viewer-accordion").accordion({heightStyle: 'content', collapsible: true});

  });
 

  /**
   * Creates the visualization for the network.
   * 
   * The global variable 's' will be set to a new instance of a SigmaJS object.
   * 
   * @param network_data
   *   An associative array of 'nodes' and 'edges'.
   */
  function loadNetwork(network_data){

    // Clearing out data from the tables after every ajax call for module load
    $("#current_selection").html("");

    // If the sigma instance exists, then we need to clear the existing
    // graph, and kill the object so we can recreate it.
    if(Sigma_Instance){
      //sigma.layouts.startForceLink({linLogMode:true});
      //Sigma_Instance.stopForceLink();
      //$('#graph').remove(); 
      //$('#tripal-network-viewer-graph-container').html('<div id="graph"></div>');
 
      Sigma_Instance.graph.clear();
      Sigma_Instance.refresh();
      Sigma_Instance.kill();
    } 

    // Create our new sigma instance.
    Sigma_Instance = new sigma({
      graph: network_data,
      renderer: {
        container: document.getElementById('tripal-network-viewer-graph-container'),
        type: 'canvas'
      },
      settings: {
        // Edge Settings.
        edgeColor: '#888888',
        defaultEdgeColor: 'default',
        enableEdgeHovering: true,
        batchEdgesDrawing: false,
        hideEdgesOnMove: false,
        // Misc Settings.
        animationsTime: 5000,
        drawLabels: true,
        scalingMode: 'outside',
        sideMargin: 1,
        // Node Settings.
        nodeBorderSize: 1,
        nodeBorderColor: '#00000',
        nodeHoverBorderSize: 3,
        defaultNodeHoverBorderColor: 'yellow',
        nodeActiveBorderSize: 2,
        nodeActiveOuterBorderSize: 3,
        defaultNodeActiveBorderColor: 'yellow',
        defaultNodeActiveOuterBorderColor: 'yellow',
        defaultNodeType: 'border',
        // Node Labels
        defaultLabelSize: 20,
        labelSizeRatio: 2,
        labelThreshold: 2,
        labelSize: 'proportional',
        labelColor: '#888888'
      }
    });

    // Calculate the maximum degree in the network.
    var max_degree = 0;
    Sigma_Instance.graph.nodes().forEach(function (n) {
      degree = Sigma_Instance.graph.degree(n.id);
      if (degree > max_degree) {
        max_degree = degree;
      }
    });
    
    // Set properties for Edges.
    Sigma_Instance.graph.edges().forEach(function (e) {
      e.color = "#888888";
      if (e.sc < 0) {
        e.type = 'dashed';
        e.color = '#FF6666';
      }
    });

    // Set properties for Nodes.  
    Sigma_Instance.graph.nodes().forEach(function (n) {
      if (!Sigma_Instance.graph.degree(n.id)) {
        Sigma_Instance.graph.dropNode(n.id);
      }
      else {
        n.x = Math.random();
        n.y = Math.random();
      }
      degree = Sigma_Instance.graph.degree(n.id);
      cscale = chroma.scale(['yellow', 'orange', 'red']).domain([1, max_degree]);
      n.color = cscale(degree);
      n.size = Math.round((degree/max_degree) * 10) + 3;
    });
    Sigma_Instance.refresh();
    
    // Add an onClick event to each node.
    Sigma_Instance.bind('clickNode', function(e) {
      $.fn.retrieveNode(e.data.node['id']);
    });


    // Add the force link layout to this graph and start it.
    // Create a forced link layout object.
    // See https://github.com/mv15/graph-visualization/tree/master/src/plugins/sigma.layout.forceLink
    // for instructions.
    var fa = sigma.layouts.configForceLink(Sigma_Instance, {
      autoStop: true,
      maxIterations: 100,
      gravity: 5,
    });

    // Start the ForceLink algorithm:
    sigma.layouts.startForceLink();

    // Add a drag event listernet to this sigma object.
    addDragListner(Sigma_Instance);
    
    // Setup the node locator form element;
    setUpLocater();

    // Create a lasso object and and an activation event.
    var lasso = createLasso(Sigma_Instance, network_data);
    addLassoActivateEvent(Sigma_Instance, lasso);

    // Curve parallel edges: this is to distinguish between interconnections.
    sigma.canvas.edges.autoCurve(Sigma_Instance);
    Sigma_Instance.refresh();

  } 

  /**
   * Adds an activate event to a lasso for a given sigmaJS instance. 
   * 
   * @param sigmaInstace
   *   An instace of a sigmaJS object.
   * @param lasso
   *   An intance of a lasso object.
   */
  function addLassoActivateEvent(sigma_instance, lasso) {
    document.addEventListener('keyup', function (event) {
      switch (event.keyCode) {
        case 76:
          if (event.altKey) {
            if (lasso.isActive) {
              lasso.deactivate();
            } 
            else {
              lasso.activate();
            }
          }
          break;
      }
    })
  }

  /**
   * Create the lasso object.
   * 
   * The Lasso is a linkurious plugin that allows for selection of nodes
   * in the graph.  This function is a wrapper for quick instanation of
   * a new lasso on a sigmaJS object.
   * 
   * @param sigma_instance
   *   An instance of a sigmaJS object.
   *   
   * @return
   *   A instance of a lasso object.
   */
  function createLasso(sigma_instance,network_data) {
    var r = network_data;
    sigma_instance.refresh();

    var lasso = new sigma.plugins.lasso(sigma_instance, sigma_instance.renderers[0], {
      'strokeStyle': 'gray',
      'lineWidth': 2,
      'fillWhileDrawing': true,
      'fillStyle': 'rgba(100, 100, 100, 0.5)',
      'cursor': 'crosshair'
    });

    // Listen for selectedNodes event.
    lasso.bind('selectedNodes', function (event) {

      // Do something with the selected nodes.
      var nodes = event.data;
      var node_label = new Array();
      var i=0;
      // List of nodes which are selected.
      nodes.forEach(function (node) {
        node.active = true;
        node_label[i] = node.label;
        i++;
      });
      return lasso;
    })
  }

  /**
   * Retrieves the network using filtering criteria.
   * 
   * This function sets the 'network_data' variable which will have the
   * current network edges and nodes.
   * 
   * In order to execute functions after a Drupal AJAX callback we must create
   * new JQuery functions that serve as wrappers.  These functions
   * will be specified for execution in the Drupal callback functions.
   * Drupal can only call JQuery functions which is why we're adding this
   * function to the $.fn variable.
   * 
   * @param filters
   *   A JSON array of the filters to be used for retrieving the network
   */
  $.fn.retrieveNetwork = function(filters) {
    // The data needed to retreive the network is provided to this function
    // by the tripal_network module of Drupal via the data function. 
    var network_id = filters['network_id'];
    var module_id = filters['module_id'];
    var trait_id = filters['trait_id'];
    var node_id = filters['node_id'];
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/retrieve',
      type: "GET",
      dataType: 'json',
      data: {'network_id': network_id, 'module_id': module_id, 'trait_id': trait_id, 'node_id': node_id },
      success: function(json) {
        Network_Data = json;
        loadNetwork(Network_Data);
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  };

  /**
   * Retreives information about a node.
   */
  $.fn.retrieveNode = function(node_id) {
    $.ajax({
      url: baseurl + '/networks/retrieve/node',
      type: 'GET',
      dataType: 'json',
      data: {'node_id': node_id},
      success: function(json) {
        $('#tripal-network-viewer-data-panel').html(json['html']);
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      } 
    });
  }

  /**
   * Adds a drag listener to a SigmaJS object.
   * 
   * The drag listerner is responsible for handling of nodes, by adding
   * functions that should occur when start, drag, drop, and end events
   * occur.
   * 
   * @param sigma_instance
   *   An instance of a sigmaJS object.
   */
  function addDragListner(sigma_instance) {
    
    var activeState = sigma.plugins.activeState(sigma_instance);
    var dragListener = sigma.plugins.dragNodes(sigma_instance, sigma_instance.renderers[0], activeState);
    var keyboard = sigma.plugins.keyboard(sigma_instance, sigma_instance.renderers[0]);
    var select = sigma.plugins.select(sigma_instance, activeState);

    // Bind the Keyboard plugin to the Select plugin:
    select.bindKeyboard(keyboard);

    // On the node being clicked, startdrag is activated
    dragListener.bind('startdrag', function(event) {

    });

    //This is activated during the process of dragging
    dragListener.bind('drag', function(event) {

    });

    //This is activated when the user just stops dragging
    dragListener.bind('drop', function(event) {

    });

    //Activated just after drop event binding to end dragging
    dragListener.bind('dragend', function(event) {

    });
  }

  function setUpLocater(){
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
    var locate = sigma.plugins.locate(Sigma_Instance, conf);

    locate.setPadding({
      // top:250,
      // bottom: 250,
      right:250,
      // left:250
    });
  
    if (!Sigma_Instance.settings('autoRescale')) {
      sigma.utils.zoomTo(Sigma_Instance.camera, 0, 0, conf.zoomDef);
    }


    /*
    var reset = document.getElementById("reset-btn");
    reset.addEventListener("click", function(e) {
      Sigma_Instance.graph.nodes().forEach(function (n) {
        n.active=false;
      });
      locate.center(conf.zoomDef);
    }); */

    function locateNode (e) {
    };   
  }
})(jQuery);

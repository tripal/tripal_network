(function($) {

  // An instance of a sigmaJS object.
  var s;

  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {


    } // END attach: function (context, settings){
  } // End Drupal.behaviors.tripal_network.

  $(document).ready(function() {
    $('.bg-color-picker').click(function(){
      $('body').css('background-color', $(this).css('background-color'));
    });

    // Add a click response to open and close the panels.
    $('.toggle-header').click(function(){
      $(this).parent().find('.toggle-content').slideToggle('fast');
    });

    // Use JQuery UI to format the data panel with tabs.
    $("#tripal-network-viewer-data-panel").tabs();
  });

  
  /**
   * Resets all of the form elements and data tables.
   * 
   * At times it is necessary to clear out the data in some form elements
   * as well as the data tables. This function will clear everything.
   * 
   */
  function clearData() {
    // There is a form element containing the list of nodes on the graph. The
    // user can select a node from the drop down and the visualization will
    // focus on that node.
    $("#nodelist").html("<option>Select the node</option>");
  }

  /**
   * Creates the visualization for the network.
   * 
   * The global variable 's' will be set to a new instance of a SigmaJS object.
   * 
   * @param network_data
   *   An associative array of 'nodes' and 'edges'.
   */
  function loadNetwork(network_data){

    // Clear out any values in forl elements or the data table when we have
    // new network data.
    clearData();

    // If the sigma instance exists, then we need to clear the existing
    // graph, and kill the object so we can recreate it.
    if(s){
      s.graph.clear();
      s.refresh();
      s.kill();
    } 

    // Create our new sigma instance.
    s = new sigma({
      graph: network_data,
      renderer: {
        container: document.getElementById('graph-container'),
        type: 'canvas'
      },
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
      }
    });

    // Calculate the maximum degree in the network.
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
      cscale = chroma.scale(['yellow', 'orange', 'red']).domain([1, max_degree]);
      n.color = cscale(degree);
      n.size = degree * 2;
    });
    s.refresh();
    
    // Add an onClick event to each node.
    s.bind('clickNode', function(e) {
      // Currently, this event does nothing... just here for future use.
    });

    // Set the gravity according to the number of nodes.
    var count = network_data['edges'].length;
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

    // Add the force link layout to this graph and start it.
    addForceLinkLayout(s, grav);   

    // Add a drag event listernet to this sigma object.
    addDragListner(s);
   
    // Setup the node locator form element;
    setupNodeLocater(s);
    

    // Create a lasso object and and an activation event.
    var lasso = createLasso(s, network_data);
    addLassoActivateEvent(s, lasso);

    // Curve parallel edges: this is to distinguish between interconnections.
    sigma.canvas.edges.autoCurve(s);
    s.refresh();

  } 

  /**
   * Adds an activate event to a lasso for a given sigmaJS instance. 
   * 
   * @param sigmaInstace
   *   An instace of a sigmaJS object.
   * @param lasso
   *   An intance of a lasso object.
   */
  function addLassoActivateEvent(sigmaInstance, lasso) {
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
   * Populates the rows of the network table in the Data panel. 
   */
  function setNetworTable() {
    //document.getElementById("data-panel-node-list").innerHTML = dataset;
  }

  /**
   * Populates the rows of the nodes table in the Data panel.
   * 
   * @param selection
   *   An array of the nodes that should be displayed. If NULL then all nodes
   *   in the network data will be displayed.
   */
  function setNodesTable(selection) {
   //document.getElementById("data-panel-node-list").innerHTML = dataset;
  }

  /**
   * Populates the rows of the edges table in the Data panel.
   * 
   * @param selection
   *   An array of the edges that should be displayed. If NULL then all edges
   *   in the network data will be displayed.
   */
  function setEdgesTable(selection) {
    //document.getElementById("data-panel-node-list").innerHTML = dataset;
  }

  /**
   * Populates the rows of the functional table in the Data panel.
   * 
   * @param selection
   *   An array of the nodes that should be displayed. If NULL then all nodes
   *   in the network data will be displayed.
   */
  function setFunctionalTable(selection) {
    //document.getElementById("data-panel-node-list").innerHTML = dataset;
  }

  /**
   * Populates the rows of the markers table in the Data panel.
   * 
   * @param selection
   *   An array of the nodes that should be displayed. If NULL then all nodes
   *   in the network data will be displayed.
   */
  function setMarkersTable(selection) {
    //document.getElementById("data-panel-node-list").innerHTML = dataset;
  }

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
  function createLasso(sigmaInstance,network_data) {
    var r = network_data;
    sigmaInstance.refresh();

    var lasso = new sigma.plugins.lasso(sigmaInstance, sigmaInstance.renderers[0], {
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

    // List of nodes which are selected.
    var datas ="";
    nodes.forEach(function (node) {
      node.active = true;
      datas=datas + node.label+"<br />";
    });

    //document.getElementById("data-panel-node-list").innerHTML = datas;
    //populate(datas);

    $("#current_data").innerHTML = datas;
      sigmaInstance.refresh();
    });

    return lasso;
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
    var species = filters['species'];
    var module = filters['module'];
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/retrieve',
      type: "GET",
      dataType: 'json',
      data: {'species': species, 'module': module},
      success: function(json) {
        network_data = json;
        loadNetwork(network_data);
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  };

  /**
   * Adds the force link layout to a sigmaJS object and starts it.
   * 
   * TODO: perhaps layouts could be handled by a class with adding and
   * starting a layout handled through seperate functions of the class.
   * 
   * @param sigmaInstance
   *   An instance of a sigmaJS object.
   * @param gravity
   *   A integer that indicates the amount of gravity used by the force
   *   linked layout.
   */
  function addForceLinkLayout(sigmaInstance, gravity) {
    // Create a forced link layout object.
    var fa = sigma.layouts.configForceLink(sigmaInstance, {
      worker: true,
      autoStop: true,
      background: true,
      scaleRatio: 30,
      gravity: gravity,
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
  }

  /**
   * Adds a drag listener to a SigmaJS object.
   * 
   * The drag listerner is responsible for handling of nodes, by adding
   * functions that should occur when start, drag, drop, and end events
   * occur.
   * 
   * @param sigmaInstance
   *   An instance of a sigmaJS object.
   */
  function addDragListner(sigmaInstance) {
    
    var activeState = sigma.plugins.activeState(sigmaInstance);
    var dragListener = sigma.plugins.dragNodes(sigmaInstance, sigmaInstance.renderers[0], activeState);
    var keyboard = sigma.plugins.keyboard(sigmaInstance, sigmaInstance.renderers[0]);
    var select = sigma.plugins.select(sigmaInstance, activeState);

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
  
  /**
   * Adds functionality to zoom to a node from a form select box.
   * 
   * The display panel has a node drop down that lists all of the nodes
   * in the graph. If the user selects a node then the graph should 
   * zoom to that node.
   * 
   * @param sigmaInstance
   *   An instance of a sigmaJS object.
   */
  function setupNodeLocater(sigmaInstance) {
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

    // Initializing the instance for locate.
    var locate = sigma.plugins.locate(sigmaInstance, conf);

    locate.setPadding({
      right:250,
    });


    if (!sigmaInstance.settings('autoRescale')) {
      sigma.utils.zoomTo(sigmaInstance.camera, 0, 0, conf.zoomDef);
    }

    var categories = {};

    // Add the nodes to the nodelist form element. 
    $("#nodelist").html("<option>Select the node</option>");
    sigmaInstance.graph.nodes().forEach(function(n) {
      $('#nodelist').append($("<option></option>").attr("value", n.id).text(n.label)); 
    });
  
    document.getElementById("reset-btn").addEventListener("click", function(e) {
      document.getElementById("nodelist").selectedIndex = 0;

      // Set the default colors of the nodes after being reset 
      sigmaInstance.graph.nodes().forEach(function (n) {
        n.active = false;
      });
      locate.center(conf.zoomDef);
    });

    // Add an onChange event lister to the nodelist form element.  
    $('#nodelist').on('change', function(){
      nid = $("#nodelist").val();
      
      if (nid == '') {
        locate.center(1);
      }
      else {
        s.graph.nodes(nid).active = true;
        locate.nodes(nid);
      }
    });
  }


})(jQuery);

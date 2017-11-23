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

    // Add a click response to open and close the panels.
    $('.toggle-header').click(function(){
      $(this).parent().find('.toggle-content').slideToggle('fast');
    });

    // Use JQuery UI to format the data panel with tabs.
    $("#tripal-network-viewer-data-panel").tabs();
    
    // Loading of nodes data on clicking the nodes tab
    $("#nodes_info").click(function(){
      setNodesTable(Network_Data);

    });

    // Loading of edges data on clicking the edges tab
    $("#edges_info").click(function(){
      setEdgesTable(Network_Data);

    });



    
    
  });

  // Code that handles the data-tables of the node-list when the lasso tool is used
 
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
    //$("#nodelist").html("<option>Select the node</option>");
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

    // Clearing out data from the tables after every ajax call for module load
    $('#data-panel-node-list tbody').html("");
    $('#data-panel-edge-list tbody').html("");
    $("#current_selection").html("");

    // If the sigma instance exists, then we need to clear the existing
    // graph, and kill the object so we can recreate it.
    if(Sigma_Instance){
      //sigma.layouts.startForceLink({linLogMode:true});
      //Sigma_Instance.stopForceLink();
      //$('#graph').remove(); 
      //$('#graph-container').html('<div id="graph"></div>');
 
      Sigma_Instance.graph.clear();
      Sigma_Instance.refresh();
      //Sigma_Instance.kill();
    } 

    // Create our new sigma instance.
    Sigma_Instance = new sigma({
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
     
    //Setting the data in the Nodes Table 
    //setNodesTable(network_data);
    //Setting the data in the Edges Table
    //setEdgesTable(network_data);

    // Calculate the maximum degree in the network.
    var max_degree = 0;
    Sigma_Instance.graph.nodes().forEach(function (n) {
      degree = Sigma_Instance.graph.degree(n.id);
      if (degree > max_degree) {
        max_degree = degree;
      }
    });

    // Set the default x,y coordinate.  
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
      n.size = degree * 2;
    });
    Sigma_Instance.refresh();
    
    // Add an onClick event to each node.
    Sigma_Instance.bind('clickNode', function(e) {
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
    addForceLinkLayout(Sigma_Instance, grav);   

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

    
    var table1;
    var no_of_nodes = selection["nodes"].length;
    var i;
    var data ="";
    var node_labels=new Array();
    $('#data-panel-node-list tbody').html("");
    
    for(i=0;i<no_of_nodes;i++){
      node_labels[i] = ((selection["nodes"])[i]).label;
    }  

    
    // Making an Ajax call to retrieve the functional data

    

    // Ajax Call to retrieve the functional data
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/function',
      type: "POST",
      dataType: 'json',
      data: {'nodes': node_labels, 'genus': 'Oryza','species':'sativa','type':'gene'},
      success: function(json) {
        functional_data = json;
        populate_data(functional_data);
        
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    });
    
    function populate_data(functional_data){
      for(i=0;i<no_of_nodes;i++){
      var char_label,char_length;
      char_label = ((selection["nodes"])[i]).label;
      if(functional_data[char_label]){
        char_length = functional_data[char_label].length;
        var function_data="",j;
        for(j = 0;j < char_length; j++){
          var val = (functional_data[char_label])[j];
          for (var k in val){
            var sub_key = k;
            var sub_val = val[k];
            function_data = function_data + sub_key + ':' + sub_val +"<br />";
          }
        }

      }
      else{
        var function_data;
        function_data = '-';
      }

      
        $('#data-panel-node-list tbody').append('<tr><td>'+ (selection["nodes"][i]).label + '</td><td>' + function_data + '</td></tr>');
     }

     table1 = $('#data-panel-node-list-table').DataTable({
        "scrollY":        "140px",
        "ordering": false,
        "scrollCollapse": true,
        "paging": false,
        "retrieve" : true,
        "info": false,
        "searching" : false
     });


   }


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
    var table2;
    var no_of_edges = selection["edges"].length;
    var i;
    $('#data-panel-edge-list tbody').html('');
    for(i = 0; i < no_of_edges; i++){
      var temp_source = parseInt((selection["edges"][i]).source);
      var temp_target = parseInt((selection["edges"][i]).target);
      $('#data-panel-edge-list tbody').append('<tr><td>'+ (selection["nodes"][temp_source]).label + '</td><td>' + (selection["nodes"][temp_target]).label + '</td><td>1.5</td></tr>');

    }

    
    table2 = $('#data-panel-edge-list-table').DataTable( {
        "scrollY":        "140px",
        "retrieve": true,
        "ordering": false,
        "scrollCollapse": true,
        "paging": false,
        "info" : false
    });
    



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
    var datas ="<table id='current_selection' class='display' width='100%'' cellspacing='0'><thead><tr><th>Node</th><th>Functional Annotations</th></tr></thead><tbody>";
    nodes.forEach(function (node) {
      node.active = true;
      node_label[i] = node.label;
      i++;
      //datas=datas + "<tr><td>"+node.label+"</td><td>Unknown</td></tr>";
    });
    //datas = datas + "</tbody></table>";

    var functional_data;

    // Ajax Call to retrieve the functional data
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/function',
      type: "POST",
      dataType: 'json',
      data: {'nodes': node_label, 'genus': 'Oryza','species':'sativa','type':'gene'},
      success: function(json) {
        functional_data = json;
        populate_lasso_data(functional_data);
        
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    });

    function populate_lasso_data(functional_data){
      var j;
      for(j=0;j<i;j++){
        var function_data = "";
        if(functional_data[node_label[j]]){
          var functional_length = functional_data[node_label[j]].length;
          var k;
          for(k=0;k<functional_length;k++){
            var val = (functional_data[node_label[j]])[k];
            for(var m in val){
              var sub_key = m;
              var sub_val = val[m];
              function_data = function_data + sub_key + ":" + sub_val+"<br />";
            }
          }



        }
        else{
          function_data = '-';
        }

        datas=datas + "<tr><td>"+node_label[j]+"</td><td>"+function_data+"</td></tr>"; 
      }

          //Adding Dynamic content to build the table
      document.getElementById("data-panel-current-list").innerHTML = datas;

  
      $('#current_selection').DataTable( {
          "scrollY":        "100px",
          "ordering": false,
          "retrieve": true,
          "scrollCollapse": true,
          "paging": false
      });


    }
    

    //populate(datas);
    //alert(datas);
  
      sigma_instance.refresh();
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
    var genes = filters['genes'];
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/retrieve',
      type: "GET",
      dataType: 'json',
      data: {'species': species, 'genes': genes},
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
   * Adds the force link layout to a sigmaJS object and starts it.
   * 
   * TODO: perhaps layouts could be handled by a class with adding and
   * starting a layout handled through seperate functions of the class.
   * 
   * @param sigma_instance
   *   An instance of a sigmaJS object.
   * @param gravity
   *   A integer that indicates the amount of gravity used by the force
   *   linked layout.
   */
  function addForceLinkLayout(sigma_instance, gravity) {
    // Create a forced link layout object.
    var fa = sigma.layouts.configForceLink(sigma_instance, {
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

    var categories = {};

    // read nodes
    var nodelistElt = document.getElementById("nodelist");
    nodelistElt.innerHTML = "<option>All nodes</option>";
    Sigma_Instance.graph.nodes().forEach(function(n) {
      $('#nodelist').append($("<option></option>").attr("value", n.id).text(n.label)); 

      //categories[n.attributes.modularity_class] = true;
    });

    var reset = document.getElementById("reset-btn");
    reset.addEventListener("click", function(e) {
      var n_list = document.getElementById("nodelist");
      n_list.selectedIndex = 0;
      Sigma_Instance.graph.nodes().forEach(function (n) {
        n.active=false;
      });
      locate.center(conf.zoomDef);
    });

    function locateNode (e) {
      var nid = $("#nodelist").val();

      if (nid == '') {
        
        locate.center(1);
      }
      else {
        Sigma_Instance.graph.nodes(nid).active = true;
        locate.nodes(nid);
      }
    };

   
    var n_list2 = document.getElementById("nodelist");
    n_list2.addEventListener("change", locateNode);
  }
  
  

})(jQuery);

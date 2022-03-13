(function($) {
   
  // Holds key/value pairs indicating the current state of the viewer.
  var state = {
	'network_id': null,
    'organism_id': null,
    'feature_id': null,
    'network_session_id': null,
  };
   
  var sidebar_default_width = null;
  
  // When a node is clicked, this will hold the plotly information
  // so that the node menu knows which node was clicked. 
  var clicked_node = null;
  
  var selected_nodes = {};
  var selected_edge = null;
  var selected_edge_prev_color = null;
   
  // Holds the value of waiting ajax commands that are using the 
  // spinner. The spinner should turn off once all have completed.
  var spinner_waiting = 0;
   
  // Plotly has a bug. When a relayout is called within a listener then 
  // it triggers a click event which calls the listner and creates an
  // eternal loop. So, we'll use this to keep that from happening.
  var in_listener = false;
  
  // Indicates the openness state of the sidebar
  var sidebar_state = 'partial';

  // For storing the current position of the mouse.  
  var mouseX;
  var mouseY;

    
  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {	
	
	  $('#tripal-network-edge-expression-plot-color-by').change(function() {
		var z_axis = $('#tripal-network-edge-expression-plot-color-by').find(":selected").text();
		var edge_id = $('#tripal-network-edge-id').val();
		$.fn.updateEdgeExpressionPlot({z_axis: z_axis, edge_id: edge_id})
	  });
    } 
  } 

  // Code that must be executed when the document is ready and never again
  // goes in this section
  $(document).ready(function() {
	
	$(document).mousemove(function(e) {
      mouseX = e.pageX;
      mouseY = e.pageY;
    }).mouseover(); 
	
	$("#tripal-network-viewer-data-tabs").tabs();
	$("#tripal-network-viewer-node-menu").menu();
	
	// Node menu item handlers.
	$("#tripal-network-viewer-node-menu #node-select").click(function(){
	  $("#tripal-network-viewer-node-menu").hide();
	  if ($("#tripal-network-viewer-node-menu #node-select").text() == 'Unselect') {
		$.fn.unselectNode(clicked_node);
	  }
	  else {
		$.fn.selectNode(clicked_node);
	  }
	});
	$("#tripal-network-viewer-node-menu #node-inspect").click(function(){
		$("#tripal-network-viewer-node-menu").hide(); 
		$.fn.updateNodeDetails({'node_id': clicked_node['points'][0]['id']})		
	});

	
	// For displays that don't fit the navbar and the sidebar we will scale
	// the sidebar to fit the width. Otherwise we need to remember the
	// default sidebar width. 
	sidebar_default_width = $('#tripal-network-viewer-sidebar').width();
   
    // Get the size of the elements so we can resize things when
    // closing the sidebar.
    var window_width = $(window).width();
    var navbar_width = $('tripal-network-viewer-navbar').width();
    var sidebar_width =  $('#tripal-network-viewer-sidebar').width();
	var toggle_img_width =  $('#tripal-network-viewer-slide-icons').width();
    var control_width =  sidebar_width + navbar_width + toggle_img_width * 2 + 15;
    var display_width = window_width - (sidebar_width + navbar_width);
    
    // Set some defaults    
    if (control_width > window_width) {
	    
	    // Shrink the navigation bar.
		navbar_width = 50
		
		// Fill the rest of the space with the sidebar and toggle button.
		sidebar_width = window_width - ((toggle_img_width * 2) + navbar_width);
		sidebar_default_width = sidebar_width;
		toggle_width =  window_width - ((toggle_img_width * 2) + 15);
		display_width = 0;			
		
		// Now resize everthing.
		$('.tripal-network-viewer-navbar-icon').css({'height': 25, 'width': 25, 'margin': 12});
		$('#tripal-network-viewer-navbar').width(navbar_width);		
		$('#tripal-network-viewer-sidebar').css({'left': navbar_width, 'width': sidebar_width});
		$('#tripal-network-viewer-sidebar-toggle').css({'left': navbar_width, 'width': toggle_width});
		$('#tripal-network-viewer-display').css({'left': navbar_width + sidebar_width, 'width': display_width});
	}  
    
    // If the window size changes we need to update the size variables.
    $( window ).resize(function() {
	  var window_width = $(window).width() 
	  var sidebar_width = $('#tripal-network-viewer-sidebar').width();
	  var navbar_width = $('#tripal-network-viewer-navbar').width();
      var display_width = window_width - (sidebar_width + navbar_width);
      $('#tripal-network-viewer-display').css({'left': navbar_width + sidebar_width, 'width': display_width});
      Plotly.Plots.resize('tripal-network-viewer');
    });
      
    // Show the network box on load
    $.fn.showBox("tripal-network-viewer-network-select-box");
     
    // If any navbar icons are clicked show the corresponding box.
    $(".tripal-network-viewer-navbar-icon").click(function() {
      var id = $(this).attr('id');
      var box = id.replace('icon', 'box');
      $.fn.showBox(box);
    })
    
    // Add support for the slide icon.
    $("#tripal-network-viewer-slide-left-icon").click(function() {
      if (sidebar_state == 'partial') {        
        $.fn.closedSidebar();
      }
      else if (sidebar_state == 'full') {
        $.fn.partialSidebar();
      }
    })
    $("#tripal-network-viewer-slide-right-icon").click(function() {
	  if (sidebar_state == 'closed') {         
        $.fn.partialSidebar();
      }
      else if (sidebar_state == 'partial') {
		$.fn.fullSidebar();
	  }
	})
  });
    
  $.fn.setState = function(args) {
    for (var key in args) {
      state[key] = args[key];
    } 
  }
  
  $.fn.showBox = function(id) {
    $.fn.partialSidebar(id);
    var icon = id.replace('box', 'icon');
    $('.tripal-network-viewer-sidebar-box').hide();
    $('.tripal-network-viewer-navbar-icon').css('opacity', '0.5');
    $('#' + icon).css('opacity', '1');
    $('#' + id).show();
  }
  
  $.fn.partialSidebar = function(id = null) {
	var window_width = $(window).width() 
    var navbar_width = $('#tripal-network-viewer-navbar').width();   
    var sidebar_width = sidebar_default_width;
    var toggle_img_width =  $('#tripal-network-viewer-slide-icons').width();
    var display_width =  window_width - (sidebar_width + navbar_width);
    var toggle_width = sidebar_width + toggle_img_width + 20;

	$('#tripal-network-viewer-slide-left-icon').show();
	$('#tripal-network-viewer-slide-right-icon').show();
	$('#tripal-network-viewer-sidebar').animate({width: 'show'}, "fast");
	$('#tripal-network-viewer-sidebar').width(sidebar_default_width);
    $('#tripal-network-viewer-sidebar-toggle').width(toggle_width);
    $('#tripal-network-viewer-display').css({top: 0, left: navbar_width + sidebar_width, position:'absolute'});
    $('#tripal-network-viewer-display').width(display_width);
    Plotly.Plots.resize('tripal-network-viewer');
    sidebar_state = 'partial';
  }
  
  $.fn.closedSidebar = function() {
	var window_width = $(window).width() 
	var navbar_width = $('#tripal-network-viewer-navbar').width();   
    var display_width =  window_width - navbar_width;
    var toggle_width =  $('#tripal-network-viewer-slide-icons').width();
    $('#tripal-network-viewer-sidebar').animate({width: 'hide'}, "slow");
    $('#tripal-network-viewer-sidebar').width(0);
    $('#tripal-network-viewer-sidebar-toggle').width(toggle_width);
    $('#tripal-network-viewer-display').css({top: 0, left: navbar_width, position:'absolute'});
    $('#tripal-network-viewer-display').width((display_width));
    $('#tripal-network-viewer-slide-left-icon').hide();
    Plotly.Plots.resize('tripal-network-viewer');
    sidebar_state = 'closed';
  }
  
  $.fn.fullSidebar = function() {
	var window_width = $(window).width() 
    var navbar_width = $('#tripal-network-viewer-navbar').width();   
    var toggle_img_width =  $('#tripal-network-viewer-slide-icons').width();
    var sidebar_width = window_width - navbar_width - toggle_img_width;
    var toggle_width = window_width - navbar_width;  
    var display_width =  window_width - (sidebar_width + navbar_width);
 

    $('#tripal-network-viewer-sidebar').animate({width: 'show'}, "slow");
    $('#tripal-network-viewer-sidebar').width(sidebar_width);
    $('#tripal-network-viewer-sidebar-toggle').width(toggle_width);
    $('#tripal-network-viewer-display').css({top: 0, left: navbar_width, position:'absolute'});
    $('#tripal-network-viewer-display').width((display_width));
    $('#tripal-network-viewer-slide-right-icon').hide();
    Plotly.Plots.resize('tripal-network-viewer');
    sidebar_state = 'full';
  }

  $.fn.showSpinner = function() {
	spinner_waiting = spinner_waiting + 1;
	$('#tripal-network-viewer-loading').show();
  }
  $.fn.hideSpinner = function() {
	spinner_waiting = spinner_waiting - 1;
	if (spinner_waiting == 0) {
		$('#tripal-network-viewer-loading').hide();
	}
  }
  
  
  $.fn.updateDegreeDistPlot = function() {
	
	// The plotly data for the degree distribution plot
	// is set by the tripal_network Drupal code as form elements
	// in the network-details form when it is updated. We can
	// pull the data from those elements to create the plot.
	var dist_data = JSON.parse($('#tripal-network-degree-dist-plot-data').val())
	var dist_layout = JSON.parse($('#tripal-network-degree-dist-plot-layout').val())
	
	var settings = {
      'displaylogo': false, 
      'toImageButtonOptions' : {
        'filename': 'tripal_network_degree_distribution',
        'format': 'png',
        'scale' : 10
      }
    };
	Plotly.newPlot('tripal-network-degree-dist-plot', [dist_data], dist_layout, settings);
  }
  
  
  
  $.fn.updateEdgeExpressionPlot = function(args) {
	 $.fn.showSpinner()
	
	 var data = state;
     data['edge_id'] = args['edge_id'];
     data['z_axis'] = args['z_axis'];
     
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/edge-expression',
      type: "GET",
      dataType: 'json',
      data: data,
      success: function(response) {

		// The plotly data for the edge expression plot data comes within the callback.
		var plot_data = response['data'];
		var plot_layout = response['layout'];
		
		var settings = {
		  'displaylogo': false, 
		  'toImageButtonOptions' : {
		    'filename': 'tripal_network_edge-expression',
		    'format': 'png',
		    'scale' : 10
		  }
		};
		Plotly.newPlot('tripal-network-edge-expression-plot', plot_data, plot_layout, settings);	 
		 
        $.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
     
  }
  
  /**
   *
   */
  $.fn.updateNodeDetails = function(args) {
     var node_id = args['node_id'];
     var data = state;
     data['node_id'] = node_id;
     $.fn.showSpinner()
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/node',
      type: "GET",
      dataType: 'json',
      data: data,
      success: function(response) {
		$('#tripal-network-viewer-node-box form').replaceWith(response[1]['data']);
	    Drupal.attachBehaviors($('#tripal-network-viewer-node-box form'), response[0]['settings']);
        $.fn.showBox('tripal-network-viewer-node-box');
		$.fn.hideSpinner();

      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
  }


  /**
   *
   */
  $.fn.updateEdgeDetails = function(args) {
	 $.fn.showSpinner()
	
	 var data = state;
     data['edge_id'] = args['edge_id'];
     
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/edge',
      type: "GET",
      dataType: 'json',
      data: data,
      success: function(response) {
		 $('#tripal-network-viewer-edge-box form').replaceWith(response[1]['data']);
		 
		 // The plotly data for the edge expression plot data comes within the callback.
		 var plot_data = JSON.parse($('#tripal-network-edge-expression-plot-data').val());
		 var plot_layout = JSON.parse($('#tripal-network-edge-expression-plot-layout').val());
		
		 var settings = {
		   'displaylogo': false, 
		   'toImageButtonOptions' : {
		     'filename': 'tripal_network_edge-expression',
		     'format': 'png',
		     'scale' : 10
		   }
		 };
		 Plotly.newPlot('tripal-network-edge-expression-plot', plot_data, plot_layout, settings);	 
		 
	     Drupal.attachBehaviors($('#tripal-network-viewer-edge-box form'), response[0]['settings']);
         $.fn.showBox('tripal-network-viewer-edge-box');                           
         $.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
  }
    
  
  /**
   *
   */
  $.fn.updateLayersForm = function() {
    $.fn.showSpinner()
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/update/layers',
      type: "GET",
      dataType: 'json',
      data: state,
      success: function(response) {
	    $('#tripal-network-viewer-layers-box form').replaceWith(response[1]['data']);
        Drupal.attachBehaviors($('#tripal-network-viewer-layers-box form'), response[0]['settings']);
		$.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
   };
   
  /**
   *
   */
  $.fn.updateFilterForm = function() {
    $.fn.showSpinner()
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/update/filter',
      type: "GET",
      dataType: 'json',
      data: state,
      success: function(response) {
	    $('#tripal-network-viewer-filters-box form').replaceWith(response[1]['data']);
		Drupal.attachBehaviors($('#tripal-network-viewer-filters-box form'), response[0]['settings']);
		$.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
   };
   
  /**
   *
   */
  $.fn.updateEdgeDataForm = function(page, edata_includes, sort_by, sort_dir) {
	var data = state;
	data['page'] = page
	data['edata_includes'] = edata_includes
	data['sort_by'] = sort_by;
	data['sort_dir'] = sort_dir;
    $.fn.showSpinner()
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/update/edgedata',
      type: "GET",
      dataType: 'json',
      data: data,
      success: function(response) {
	    $('#tripal-network-viewer-edge-data-tab form').replaceWith(response[1]['data']);
		Drupal.attachBehaviors($('#tripal-network-viewer-edge-data-tab form'), response[0]['settings']);
		$.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
   };
   
   /**
   *
   */
  $.fn.updateNodeDataForm = function(page, ndata_includes, sort_by, sort_dir) {
	var data = state;
	data['page'] = page;
	data['ndata_includes'] = ndata_includes;
	data['sort_by'] = sort_by;
	data['sort_dir'] = sort_dir;
    $.fn.showSpinner()
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/update/nodedata',
      type: "GET",
      dataType: 'json',
      data: data,
      success: function(response) {
	    $('#tripal-network-viewer-node-data-tab form').replaceWith(response[1]['data']);
		Drupal.attachBehaviors($('#tripal-network-viewer-node-data-tab form'), response[0]['settings']);
		$.fn.hideSpinner();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
		$.fn.hideSpinner();
      }
    }) 
   };
      
   /**
    * Initializes the viewer state by retiieving defaults from the server.
    */
   $.fn.initViewer = function(args) {
     $.ajax({
       // The baseurl is a variable set by Tripal that indicates the
       // "base" of the URL for this site.
       url: baseurl + '/networks/viewer/init',
       type: "GET",
       dataType: 'json',
       data: {
	     'network_id': args['network_id'] ,
         'organism_id': args['organism_id'],
         'feature_id': args['feature_id'],
         'network_session_id': state['network_session_id']
	   },
       success: function(json) {
         response = json;
         $.fn.setState(response);
       },
       error: function(xhr, textStatus, thrownError) {
         alert(thrownError);
       }
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
   * @param args 
   */
  $.fn.getNetwork = function(args) {
   
    $.fn.showSpinner()
    
    $.fn.setState(args);

    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/retrieve',
      type: "GET",
      dataType: 'json',
      data: state,
      success: function(json) {
        $.fn.updateNetworkPlot(json);               

        // Add in the network details and switch to that box.
		$('#tripal-network-viewer-network-details-box form').replaceWith(json['details']);
		
		// Now update other elements on the page.		
		$.fn.updateDegreeDistPlot();
		$.fn.updateFilterForm();
        $.fn.updateLayersForm();
        $.fn.updateEdgeDataForm();
        $.fn.updateNodeDataForm();

        // Turn off the spinner.
        $.fn.hideSpinner();

      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
        // Turn off the spinner.
        $.fn.hideSpinner();
      }
    })
  };
  
  /**
   *
   */
  $.fn.updateNetworkPlot = function(data) {
	
	// Create the new plot.
    settings = {
      'displaylogo': false, 
      'toImageButtonOptions' : {
         'filename': 'tripal_network_view',
         'format': 'svg',
         'scale' : 10
       }
    };

    Plotly.newPlot('tripal-network-viewer', data['data'], data['layout'], settings);
    
    // Add event handlers. We don't use Jquery as the selector because 
    // it's not fully compatible with Plotly.
    var myPlot = document.getElementById('tripal-network-viewer')
    myPlot.on('plotly_click', function(data){ 
	  if (in_listener) {
		return;
	  }
	  in_listener = true;
      var mode = data['points'][0]['data']['mode'];  
      var id = data['points'][0]['id'];        
      if (mode == 'markers') {
        if (selected_nodes.hasOwnProperty(id)) {
	      $("#tripal-network-viewer-node-menu #node-select").text('Unselect');
        }
        else {
	      $("#tripal-network-viewer-node-menu #node-select").text('Select');
        }
		$("#tripal-network-viewer-node-menu").css('left', mouseX);
		$("#tripal-network-viewer-node-menu").css('top', mouseY);
		$("#tripal-network-viewer-node-menu").show();
		clicked_node = data;
      }
      if (mode == 'lines') {   			
        $.fn.selectEdge(data);     
        $.fn.updateEdgeDetails({'edge_id': id})
      }
      in_listener = false;
    });
    
    // If the graph is reloaded we want to preserve the selected 
    // node.
    if (Object.keys(selected_nodes).length > 0) {
      for (const id in selected_nodes) {
	    data = selected_nodes[id];
	    $.fn.selectNode(data);
	  }           
    }
    
    // If the graph is reloaded do not preserve the highlight of the 
    // selected node.
    if (selected_edge) {
      selected_edge = null;
      selected_edge_prev_color = null;
    }
  }
  
  /**
   *
   */
  $.fn.selectNodeByID = function(node_id) {
	
	var myPlot = document.getElementById('tripal-network-viewer');
	for (var i in myPlot.data) {
	  if (myPlot.data[i]['mode'] == 'markers') {		
	    for (var j in myPlot.data[i]['ids']) {
		  if (myPlot.data[i]['ids'][j] == node_id) {
			var data = [];
	        data['points'] = [];	
	        data['points'][0] = [];	
			data['points'][0]['curveNumber'] = i;
			data['points'][0]['id'] = node_id;
			data['points'][0]['pointNumber'] = j;
			data['points'][0]['text'] = myPlot.data[i]['text'][j];
			data['points'][0]['x'] = myPlot.data[i]['x'][j];
			data['points'][0]['y'] = myPlot.data[i]['y'][j];
			data['points'][0]['z'] = myPlot.data[i]['z'][j];
			data['points'][0]['marker.size'] = myPlot.data[i]['marker']['size'][j];
			data['points'][0]['marker.color'] = myPlot.data[i]['marker']['color'][j];
			data['points'][0]['data'] = [];
			data['points'][0]['data']['hovertemplate'] = myPlot.data[i]['hovertemplate'];
			data['points'][0]['data']['ids'] = myPlot.data[i]['ids'];
			data['points'][0]['data']['mode'] = myPlot.data[i]['mode'];
			data['points'][0]['data']['marker'] = myPlot.data[i]['marker'];
			data['points'][0]['data']['text'] = myPlot.data[i]['marker'];
			data['points'][0]['data']['type'] = myPlot.data[i]['type'];
			$.fn.selectNode(data);
			break;
		  }
	    }	
	  }
	}			
  }
  
  /**
   *
   */
  $.fn.unselectNode = function(node) {
	var id = node['points'][0]['id'];
    var point_number = node['points'][0]['pointNumber'];
    var trace_number = node['points'][0]['curveNumber'];
    var marker = node['points'][0]['data']['marker'];
    marker['color'][point_number] = '#AAAAAA';
    Plotly.restyle('tripal-network-viewer', {'marker': marker}, [trace_number]);
    delete selected_nodes[id];
  }
  
  
  /**
   *
   */
  $.fn.selectNode = function(node) {
	var id = node['points'][0]['id'];
    var point_number = node['points'][0]['pointNumber'];
    var trace_number = node['points'][0]['curveNumber'];
    var marker = node['points'][0]['data']['marker'];
    marker['color'][point_number] = '#FF0000';    
    Plotly.restyle('tripal-network-viewer', {'marker': marker}, [trace_number]);
    selected_nodes[id] = node;
  }
  
  /**
   *
   */
  $.fn.selectEdge = function(data) {       
    var update = {};
    var id = data['points'][0]['id'];
    
    if (selected_edge) {
      var sid = selected_edge['points'][0]['id'];
      var stn = selected_edge['points'][0]['curveNumber'];
      var sids = selected_edge['points'][0]['data']['ids'];
      var spn1 = sids.indexOf(sid);
      var slines = selected_edge['points'][0]['data']['line'];
      slines['color'][spn1] = selected_edge_prev_color;
      slines['color'][spn1+1] = selected_edge_prev_color;
      slines['color'][spn1+2] = selected_edge_prev_color;
      update = {'line': slines};
      selected_edge = null;
      selected_edge_prev_color = null;
      Plotly.restyle('tripal-network-viewer', update, [stn]);
    }
    
    var tn = data['points'][0]['curveNumber'];
    var ids = data['points'][0]['data']['ids'];
    var pn1 = ids.indexOf(id);
    var lines = data['points'][0]['data']['line'];
    selected_edge_prev_color = lines['color'][pn1];
    lines['color'][pn1] = '#FF0000';
    lines['color'][pn1+1] = '#FF0000';
    lines['color'][pn1+2] = '#FF0000';
    update = {'line': lines};
    selected_edge = data;
    Plotly.restyle('tripal-network-viewer', update, [tn]);
    
    in_selection = false;
  }

   
})(jQuery);

(function($) {
   
   // Holds key/value pairs indicating the current state of the viewer.
   var state = {};
   
   var selected_node = null;
   var selected_edge = null;
   var selected_edge_prev_color = null;
   
   var sidebar_width = 0;
   var display_offset = 0;
   var display_width = 0;
   
   // Plotly has a bug. When a relayout is called it triggers a click
   // event which causes an eternal loop. So, we'll use this to keep 
   // that loop from happening when nodes and edges are being selected.
   var in_selection = false;
  
  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {
      
    } 
  } 

  // Code that must be executed when the document is ready and never again
  // goes in this section
  $(document).ready(function() {
   
    // Get the size of the elements so we can resize things when
    // closing the sidebar.
    sidebar_width = $('#tripal-network-viewer-sidebar').width();    
    display_offset = $('#tripal-network-viewer-display').offset();
    display_width = $(window).width() - display_offset.left;
    $('#tripal-network-viewer-display').width(display_width);
    
    // If the window size changes we need to update the size variables.
    $( window ).resize(function() {
      display_width = $(window).width() - display_offset.left;
      $('#tripal-network-viewer-display').width(display_width);
      Plotly.Plots.resize('tripal-network-viewer');
    });
      
    // Show the network box on load
    $.fn.showBox("tripal-network-viewer-network-box");
     
    // If any navbar icons are clicked show the corresponding box.
    $(".tripal-network-viewer-navbar-icon").click(function() {
      var id = $(this).attr('id');
      var box = id.replace('icon', 'box');
      $.fn.showBox(box);
    })
    
    // Add support for the slide icon.
    $("#tripal-network-viewer-sidebar-toggle").click(function() {
      if ($('#tripal-network-viewer-sidebar-header').is(":visible")) {        
        $.fn.closeSidebar();
      }
      else  {
        $.fn.openSidebar();
      }
    })
  });
  
  $.fn.setState = function(args) {
    for (var key in args) {
      state[key] = args[key];
    } 
  }
  
  $.fn.showBox = function(id) {
    $.fn.openSidebar();
    var icon = id.replace('box', 'icon');
    $('.tripal-network-viewer-sidebar-box').hide();
    $('.tripal-network-viewer-navbar-icon').css('opacity', '0.5');
    $('#' + icon).css('opacity', '1');
    $('#' + id).show();
  }
  
  $.fn.openSidebar = function() {
     $('#tripal-network-viewer-sidebar').width(sidebar_width);
     $('#tripal-network-viewer-sidebar-header').show();
     $('#tripal-network-viewer-sidebar-boxes').show("fast"); 
     $('#tripal-network-viewer-display').css({top: 0, left: display_offset.left, position:'absolute'});
     $('#tripal-network-viewer-display').width(display_width);
     Plotly.Plots.resize('tripal-network-viewer');
  }
  
  $.fn.closeSidebar = function() {
     $('#tripal-network-viewer-sidebar-boxes').hide("fast");
     $('#tripal-network-viewer-sidebar-header').hide();
     $('#tripal-network-viewer-sidebar').width('40');
     $('#tripal-network-viewer-display').css({top: 0, left: 90, position:'absolute'});
     $('#tripal-network-viewer-display').width((display_width + sidebar_width) - 40);
     Plotly.Plots.resize('tripal-network-viewer');
  }
  
  
  /**
   *
   */
  $.fn.updateNodeDetails = function(args) {
     var node_id = args['node_id'];
     var data = state;
     data['node_id'] = node_id;
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/node',
      type: "GET",
      dataType: 'html',
      data: data,
      success: function(response) {
        $.fn.showBox('tripal-network-viewer-node-box');
        $('#tripal-network-viewer-node-box form').replaceWith(response);
        Drupal.attachBehaviors();

      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  }
  
  /**
   *
   */
  $.fn.updateEdgeDetails = function(args) {
     var edge_id = args['edge_id'];
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/edge',
      type: "GET",
      dataType: 'html',
      data: {'edge_id': edge_id},
      success: function(response) {
         $.fn.showBox('tripal-network-viewer-edge-box');
         $('#tripal-network-viewer-edge-box form').replaceWith(response);
         Drupal.attachBehaviors();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  }
  
  
  /**
   *
   */
  $.fn.updateLayersForm = function() {
    
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/layers',
      type: "GET",
      dataType: 'html',
      data: state,
      success: function(response) {
        $('#tripal-network-viewer-layers-box form').replaceWith(response);
        Drupal.attachBehaviors();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
   };
   
   /**
   *
   */
  $.fn.updateFilterForm = function() {
    
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/filter',
      type: "GET",
      dataType: 'html',
      data: state,
      success: function(response) {
        $('#tripal-network-viewer-filters-box form').replaceWith(response);
        Drupal.attachBehaviors();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
   };
   
   /**
   *
   */
  $.fn.updateNetworkDetailsForm = function(network_id) {
       
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/network-details',
      type: "GET",
      dataType: 'html',
      data: {'network_id':  network_id},
      success: function(response) {
        $('#tripal-network-viewer-network-details form').replaceWith(response);
        Drupal.attachBehaviors();
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
   };
   
   /**
    * Initializes the viewer state by retiieving defaults from the server.
    */
   $.fn.initViewer = function(network_id) {
     $.ajax({
       // The baseurl is a variable set by Tripal that indicates the
       // "base" of the URL for this site.
       url: baseurl + '/networks/viewer/init',
       type: "GET",
       dataType: 'json',
       data: {'network_id': network_id },
       success: function(json) {
         response = json;
         $.fn.getNetwork(json);
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
   
    $('#tripal-network-viewer-loading').show()
    
    $.fn.setState(args);

    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/retrieve',
      type: "GET",
      dataType: 'json',
      data: state,
      success: function(json) {
        response = json;
        
        // Create the new plot.
        settings = {
          'displaylogo': false, 
          'toImageButtonOptions' : {
             'filename': 'tripal_network_view',
             'format': 'svg',
             'scale' : 10
           }
        };
        Plotly.newPlot('tripal-network-viewer', response['data'], response['layout'], settings);
        
        // Add event handlers. We don't use Jquery because it's not fully
        // compatible with Plotly.
        var myPlot = document.getElementById('tripal-network-viewer')
        myPlot.on('plotly_click', function(data){ 
          var mode =  data['points'][0]['data']['mode'];
          var id = data['points'][0]['id'];
          if (mode == 'markers') {
            $.fn.selectNode(data);            
            $.fn.updateNodeDetails({'node_id': id})
          }
          if (mode == 'lines') {   
            $.fn.selectEdge(data);     
            $.fn.updateEdgeDetails({'edge_id': id})
          }
        });
        
        // If the graph is reloaded we want to preserve the selected 
        // node.
        if (selected_node) {
          var data = selected_node;
          selected_node = null;
          $.fn.selectNode(data);
        }
        
        // If the graph is reloaded do not preserve the highlight of the 
        // selected node.
        if (selected_edge) {
          selected_edge = null;
          selected_edge_prev_color = null;
        }

        // Turn off the spinner.
        $('#tripal-network-viewer-loading').hide();

      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    })
  };
  
  
    /**
   *
   */
  $.fn.selectNode = function(data) {       
    var update = {};
    
    if (in_selection) {
      return;
    }
    
    in_selection = true;
    
    if (selected_node ) {
      var spn = selected_node['points'][0]['pointNumber'];
      var stn = selected_node['points'][0]['curveNumber'];
      var smarker = selected_node['points'][0]['data']['marker'];
      smarker['color'][spn] = '#AAAAAA';
      update = {'marker': smarker};
      selected_node = null;
      Plotly.restyle('tripal-network-viewer', update, [stn]);
    }
    
    var pn = data['points'][0]['pointNumber'];
    var tn = data['points'][0]['curveNumber'];
    var marker = data['points'][0]['data']['marker'];
    marker['color'][pn] = '#FF0000';
    update = {'marker': marker};
    selected_node = data;
    Plotly.restyle('tripal-network-viewer', update, [tn]);
    
    in_selection = false;
  }
  
  /**
   *
   */
  $.fn.selectEdge = function(data) {       
    var update = {};
    var id = data['points'][0]['id'];
    
    if (in_selection) {
      return;
    }
    in_selection = true;
    
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

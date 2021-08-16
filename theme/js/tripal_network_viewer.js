(function($) {
   
   var selected_node = null;
   var selected_edge = null;
   var selected_edge_prev_color = null;
   
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
   
    $(".tripal-network-viewer-sidebar-box-header-toggle-on").click(function() {
      var parent = $(this).parent().parent().attr('id');
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-on").hide();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-off").show();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-content").hide();
    })
    
    $(".tripal-network-viewer-sidebar-box-header-toggle-off").click(function() {
      var parent = $(this).parent().parent().attr('id');
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-on").show();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-off").hide();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-content").show();
    })
    
    $.fn.showBox('#tripal-network-viewer-network-select');
   
  });
  
  /**
   * Hides all of the sidebar box contents and sets the toggle.
   */
  $.fn.hideAllBoxes = function() {
    $(".tripal-network-viewer-sidebar-box-header-toggle-on").hide();
    $(".tripal-network-viewer-sidebar-box-header-toggle-off").show();
    $(".tripal-network-viewer-sidebar-box-content").hide();
  }
  
  /**
   * Shows the specified sidebar box and sets the toggle.
   */
  $.fn.showBox = function(box, hide_others = true) {
    if (hide_others) {
      $.fn.hideAllBoxes();
    }
    $(box + " .tripal-network-viewer-sidebar-box-content").show();
    $(box + " .tripal-network-viewer-sidebar-box-header-toggle-on").show();
    $(box + " .tripal-network-viewer-sidebar-box-header-toggle-off").hide();
  }
  
  /**
   *
   */
  $.fn.updateNodeDetails = function(args) {
     var node_id = args['node_id'];
     $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/details/node',
      type: "GET",
      dataType: 'html',
      data: {'node_id': node_id},
      success: function(response) {
        $('#tripal-network-viewer-node-details form').replaceWith(response);
        Drupal.attachBehaviors();
        $.fn.showBox('#tripal-network-viewer-node-details');
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
        $('#tripal-network-viewer-edge-details form').replaceWith(response);
        Drupal.attachBehaviors();
        $.fn.showBox('#tripal-network-viewer-edge-details');
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  }
  
  
  /**
   *
   */
  $.fn.updateDisplayForm = function(args) {
    var network_id = args['network_id'];
    var layer_by = args['layer_by'];
    var color_by = args['color_by'];
    var limit = args['limit'];
    var limit_by = args['limit_by'];
    
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/display',
      type: "GET",
      dataType: 'html',
      data: {
         'network_id': network_id, 
         'layer_by': layer_by, 
         'color_by': color_by,
         'limit': limit,
         'limit_by': limit_by
      },
      success: function(response) {
        $('#tripal-network-viewer-display-details form').replaceWith(response);
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
  $.fn.updateNetworkDetailsForm = function(args) {
    var network_id = args['network_id'];
    
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/network-details',
      type: "GET",
      dataType: 'html',
      data: {
         'network_id': network_id, 
      },
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
    // The data needed to retreive the network is provided to this function
    // by the tripal_network module of Drupal via the data function. 
    var network_id = args['network_id'];
    var layer_by = args['layer_by'];
    var color_by = args['color_by'];
    var limit = args['limit'];
    var limit_by = args['limit_by'];
    
    $('#tripal-network-viewer-loading').show()

    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/retrieve',
      type: "GET",
      dataType: 'json',
      data: {
         'network_id': network_id, 
         'layer_by': layer_by, 
         'color_by': color_by,
         'limit': limit,
         'limit_by': limit_by
      },
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
        
        // Update the Display form so that it has appropriate items for this
        // network.
        $.fn.updateDisplayForm({
          'network_id': network_id, 
          'layer_by': layer_by, 
          'color_by': color_by,
          'limit': limit,
          'limit_by': limit_by
        });
        
        // Update the network details
        $.fn.updateNetworkDetailsForm({
          'network_id': network_id, 
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

(function($) {
   
  
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
      $("#" + parent + " .tripal-network-viewer-sidebar-box-content").show();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-content").show();
    })
    
    $(".tripal-network-viewer-sidebar-box-header-toggle-off").click(function() {
      var parent = $(this).parent().parent().attr('id');
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-on").show();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-header-toggle-off").hide();
      $("#" + parent + " .tripal-network-viewer-sidebar-box-content").hide();
    })
  
  });
  
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
        $("#tripal-network-viewer-node-details .tripal-network-viewer-sidebar-box-content").show();
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
  }
  
  
  /**
   *
   */
  $.fn.updateDisplayForm = function(args) {
    var network_id = args['network_id'];
    var layer_by = args['layer_by'];
    var display_by = args['display_by'];
    
    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/form/display',
      type: "GET",
      dataType: 'html',
      data: {
         'network_id': network_id, 
         'layer_by': layer_by, 
         'display_by': display_by
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
    var display_by = args['display_by'];
    
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
         'display_by': display_by
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
          if (data['points'][0]['data']['mode'] == 'markers') {        
            $.fn.updateNodeDetails({'node_id': data['points'][0]['id']})
          }
           if (data['points'][0]['data']['mode'] == 'lines') {        
            $.fn.updateEdgeDetails({'node_id': data['points'][0]['id']})
          }
        });
        
        // Update the Display form so that it has appropriate items for this
        // network.
        $.fn.updateDisplayForm({
          'network_id': network_id, 
          'layer_by': layer_by, 
          'display_by': display_by
        });

        // Turn off the spinner.
        $('#tripal-network-viewer-loading').hide();

      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    })
  };
    

   
})(jQuery);

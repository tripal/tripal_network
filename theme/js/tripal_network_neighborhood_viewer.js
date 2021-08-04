(function($) {
   
   var layout = {
        autosize: true,
        height: 480,
        scene: {
            aspectratio: {
                x: 1,
                y: 1,
                z: 1
            },
            camera: {
                center: {
                    x: 0,
                    y: 0,
                    z: 0
                },
                eye: {
                    x: 1.25,
                    y: 1.25,
                    z: 1.25
                },
                up: {
                    x: 0,
                    y: 0,
                    z: 1
                }
            },
            xaxis: {
                type: 'linear',
                zeroline: false,
                showbackground: false,
                showline: false,
                zeroline: false,
                showgrid: false,
                showticklabels: false,
                title: '',
                showspikes: false
            },
            yaxis: {
                type: 'linear',
                zeroline: false,
                showline: false,
                zeroline: false,
                showgrid: false,
                showticklabels: false,
                title: '',
                showspikes: false
            },
            zaxis: {
                type: 'linear',
                zeroline: false,
                showline: false,
                zeroline: false,
                showgrid: false,
                showticklabels: true,
                showspikes: false
            }
        },
        title: '3d point clustering',
        width: 477
    };
   
  // All code within this Drupal.behavior.tripal_network array is 
  // executed everytime a page load occurs or when an ajax call returns.
  Drupal.behaviors.tripal_network = {
    attach: function (context, settings) {
    } 
  } 

  // Code that must be executed when the document is ready and never again
  // goes in this section
  $(document).ready(function() {
   
  });
   
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
   * @param viewer
   *   The locater of the element that will display the network view.
   */
  $.fn.retrieveNetwork = function(filters, viewer) {
    // The data needed to retreive the network is provided to this function
    // by the tripal_network module of Drupal via the data function. 
    var network_id = filters['network_id'];
    var feature_id = filters['feature_id'];

    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/retrieve',
      type: "GET",
      dataType: 'json',
      data: {'network_id': network_id, 'feature_id': feature_id },
      success: function(json) {
        data = json;
        Plotly.newPlot(viewer, data, layout);
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  };
   
})(jQuery);

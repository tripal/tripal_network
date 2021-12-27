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
   * @param args
   *   a JSON array with the following allowd keys:  network_id, feature_id,
   *   and viewer. Where viewer_id is the name of the document element where
   *   the viewer will be drawn.
   */
  $.fn.getNetworkNeighborhood = function(args) {
    // The data needed to retreive the network is provided to this function
    // by the tripal_network module of Drupal via the data function. 
    var network_id = args['network_id'];
    var feature_id = args['feature_id'];
    var organism_id = args['organism_id'];
    var viewer_id = args['viewer_id'];

    $.ajax({
      // The baseurl is a variable set by Tripal that indicates the
      // "base" of the URL for this site.
      url: baseurl + '/networks/viewer/neighborhood/retrieve',
      type: "GET",
      dataType: 'json',
      data: {
	    'organism_id': organism_id,
	    'network_id': network_id, 
        'feature_id': feature_id },
      success: function(json) {
        response = json;
        Plotly.newPlot(viewer_id, response['data'], response['layout']);
      },
      error: function(xhr, textStatus, thrownError) {
        alert(thrownError);
      }
    }) 
  };
   
})(jQuery);

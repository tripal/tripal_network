<?php

class sio__network_diagram_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Network View';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('sio__network_diagram');

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    if (count($items) == 0) {
      return;
    }

    $field_table = $this->instance['settings']['chado_table'];
    $record = $entity->chado_record;

    $js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
    $css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));

    //drupal_add_css('file', $css_path . '/tripal_network_viewer.css');

    drupal_add_js('https://cdn.plot.ly/plotly-2.3.0.min.js');
    drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
    drupal_add_js($js_path . '/tripal_network_neighborhood_viewer.js');



    // If this field is attached to an entity that is associated with the
    // network table then add the viewer for that type of record.
    if ($field_table == 'network') {

//       // Call the JS function retrieveNetwork to initially load the network. This JS
//       // call is made by Drupal's Behaviors after it responds to this GET.
//       drupal_add_js("(function(\$) {\$.fn.retrieveNetwork({'network_id': $record->network_id }); })(jQuery);", 'inline');

//       // Now add the form for filtering the network view.
//       module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
//       $filter_form = drupal_get_form('tripal_network_viewer_form', $record->network_id);
//       $element[0] = array(
//         '#type' => 'markup',
//         '#markup' => drupal_render($filter_form),
//       );
    }
    // If the field is attached to an entity that is associated with the
    // feature table (i.e. a node page) then add the appropriate viewer.
    if ($field_table == 'feature') {
      $element[0] = $this->neighborhood_viewer($record);

//       // Call the JS function retrieveNetwork to initially load the network. This JS
//       // call is made by Drupal's Behaviors after it responds to this GET.
//       if (array_key_exists('network_id', $items[0])) {
//         $network_id = $items[0]['network_id'];
//         $feature_id = $record->feature_id;
//         drupal_add_js("(function(\$) {\$.fn.retrieveNetwork({'network_id': $network_id, 'node_id': $feature_id }); })(jQuery);", 'inline');

//         // Now add the form for filtering the network view.
//          module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
//          $filter_form = drupal_get_form('tripal_network_viewer_form', $network_id, $feature_id);
//          $element[0] = array(
//            '#type' => 'markup',
//            '#markup' => drupal_render($filter_form),
//          );
//       }
    }
  }

  private function neighborhood_viewer($record){
    $feature_id = $record->feature_id;
    $viewer_id = "tripal-network-neighborhood-viewer";
    drupal_add_js("(function(\$) {\$.fn.retrieveNetwork({'feature_id': $feature_id }, '$viewer_id'); })(jQuery);", 'inline');
    return [
      '#type' => 'markup',
      '#markup' => "<div id=\"$viewer_id\"></div>",
    ];
  }
}

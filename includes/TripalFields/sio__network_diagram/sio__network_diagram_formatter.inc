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


    // If this field is attached to an entity that is associated with the
    // network table then add the viewer for that type of record.
    if ($field_table == 'network') {
    }
    // If the field is attached to an entity that is associated with the
    // feature table (i.e. a node page) then add the appropriate viewer.
    if ($field_table == 'feature') {
      $element[0] = $this->buildNeighborhoodViewer($record);
    }
  }

  /**
   * Renders the network neighborhood viewer.
   */
  private function buildNeighborhoodViewer($record){
    $feature_id = $record->feature_id;
    $viewer_id = "tripal-network-neighborhood-viewer";

    // Load the necessary JS and CSS files.
    $js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
    $css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
    drupal_add_js('https://cdn.plot.ly/plotly-2.3.0.min.js');
    drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
    drupal_add_js($js_path . '/tripal_network_neighborhood_viewer.js');

    // Get the form for specifying the network to view.
    $network_form = drupal_get_form('tripal_network_neighborhood_viewer_form', $feature_id);

    // Return the page.
    return [
      '#type' => 'markup',
      '#markup' => drupal_render($network_form) . "<div id=\"$viewer_id\"></div>",
    ];
  }
}

<?php

class sio__network_diagram_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Network Viewer';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('sio__network_diagram');

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
  
    $field_table = $this->instance['settings']['chado_table'];
    $record = $entity->chado_record;

    $js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
    $css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
    $linkurius_path = url(libraries_get_path('linkurious.js'), array('absolute' => TRUE));

    drupal_add_css('file', $css_path . '/tripal_network_viewer.css');
    drupal_add_css('file', '//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css'); 
    drupal_add_css('file', 'https://cdn.datatables.net/1.10.12/css/jquery.dataTables.min.css');

    drupal_add_js($linkurius_path . '/dist/sigma.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.layouts.forceLink.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.activeState.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.dragNodes.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.keyboard.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.select.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.locate.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.animate.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.plugins.lasso.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.helpers.graph.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.renderers.edgeLabels.min.js');
    drupal_add_js($linkurius_path . '/dist/plugins/sigma.renderers.linkurious.min.js');
    drupal_add_js("//code.jquery.com/ui/1.11.4/jquery-ui.js");
    drupal_add_js("https://cdn.datatables.net/1.10.12/js/jquery.dataTables.min.js");
    drupal_add_js($js_path . '/chroma.min.js');
    drupal_add_js($js_path . '/tripal_network_viewer.js');



    // If this field is attached to an entity that is associated with the
    // network table then add the viewer for that type of record.
    if ($field_table == 'network') {

      // Call the JS function retrieveNetwork to initially load the network. This JS
      // call is made by Drupal's Behaviors after it responds to this GET.
      drupal_add_js("(function(\$) {\$.fn.retrieveNetwork({'network_id': $record->network_id }); })(jQuery);", 'inline');

      // Now add the form for filtering the network view.
      module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
      $filter_form = drupal_get_form('tripal_network_viewer_form', $record->network_id);
      $element[0] = array(
        '#type' => 'markup',
        '#markup' => drupal_render($filter_form),
      );
    }
    // If the field is attached to an entity that is associated with the
    // feature table (i.e. a node page) then add the appropriate viewer.
    if ($field_table == 'feature') {

      // Call the JS function retrieveNetwork to initially load the network. This JS
      // call is made by Drupal's Behaviors after it responds to this GET.
      $network_id = $items[0]['network_id'];
      $node_id = $record->feature_id;
      drupal_add_js("(function(\$) {\$.fn.retrieveNetwork({'network_id': $network_id, 'node_id': $node_id }); })(jQuery);", 'inline');

      // Now add the form for filtering the network view.
      module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
      $filter_form = drupal_get_form('tripal_network_viewer_form', $network_id, $node_id);
      $element[0] = array(
        '#type' => 'markup',
        '#markup' => drupal_render($filter_form),
      );
    }
  }
}

<?php

class ncit__network_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Network';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__network');

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    
    $rows = array();
    $headers = array($settings['title']);    
    foreach ($items as $delta => $item) {
      if (empty($item['value'])) {
        continue;
      }
      
      // Get the terms for each of the keys for the 'values' property.
      $uname_term = tripal_get_chado_semweb_term('network', 'uniquename');
      $name_term = tripal_get_chado_semweb_term('network', 'name');
      $organism_term = tripal_get_chado_semweb_term('network', 'organism_id');
      $type_term = tripal_get_chado_semweb_term('network', 'type_id');

      $rows[] = array(
        $items[$uname_term],
        $items[$name_term],
        $items[$organism_term],
        $items[$type_term],
      );
    }
    
    
    $content .= theme_table(array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'class' => array('tripal-data-table'),
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => $settings['empty'],
    ));
    
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => $content . $pager,
    );
  }
}

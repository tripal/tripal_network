<?php

class local__network_module_widget extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Network Modules';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__network_module');

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
  
    $field_table = $this->instance['settings']['chado_table'];
 
    $rows = array();
    $headers = array();
    
    
    $content = theme_table(array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'class' => array('tripal-data-table'),
      ),
      'sticky' => FALSE,
      'caption' => $caption,
      'colgroups' => array(),
      'empty' => t('There are no modules'),
    ));
    
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => $content . $pager,
    );
  }
}

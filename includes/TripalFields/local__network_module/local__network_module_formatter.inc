<?php

class local__network_module_formatter extends ChadoFieldFormatter {

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
    $headers = array('Module Name', 'Type');

    // Get the terms for each of the keys for the 'values' property.
    $name_term = tripal_get_chado_semweb_term('networkmod', 'name');
    $type_term = tripal_get_chado_semweb_term('networkmod', 'type_id');
  
    foreach ($items as $delta => $item) {
      if (empty($item['value'])) {
        continue;
      }  
      $row =  array(
        $item['value'][$name_term],
        $item['value'][$type_term],
      );
      $rows[] = $row;
    }

    // Build the pager
    $items_per_page = 10;
    $total_records = count($rows);
    $total_pages = (int) ($total_records / $items_per_page) + 1;
    $pelement = 0; 
    $current_page = pager_default_initialize($total_records, $items_per_page, $pelement);
    $pager = theme('pager', array(
      'tags' => array(),
      'element' => $pelement,
      'parameters' => array(),
      'quantity' => $total_pages,
    ));
    $pager = $this->ajaxifyPager($pager, $entity);
    $page_items = array_chunk($rows, $items_per_page);    

    $caption = '';
    if ($total_records == 1) {
        $caption = 'There is ' . count($rows) . ' module in this network.';
    }
    else {
      $caption = 'There are ' . count($rows) . ' modules in this network.';
    }
    
    $content = theme_table(array(
      'header' => $headers,
      'rows' => count($rows) > 0 ? $page_items[$current_page] : array(),
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

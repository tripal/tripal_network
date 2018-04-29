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
  
    $field_table = $this->instance['settings']['chado_table'];
 
    $rows = array();
    $headers = array();
    if ($field_table == 'organism') {
      $headers = array('Identifier', 'Name', 'Network Type');
    }
    else {
      $headers = array('Identifier', 'Name', 'Network Type', 'Organism');
    }

    foreach ($items as $delta => $item) {
      if (empty($item['value'])) {
        continue;
      }
      
      // Get the terms for each of the keys for the 'values' property.
      $uname_term = tripal_get_chado_semweb_term('network', 'uniquename');
      $name_term = tripal_get_chado_semweb_term('network', 'name');
      $organism_term = tripal_get_chado_semweb_term('network', 'organism_id');
      $type_term = tripal_get_chado_semweb_term('network', 'type_id');

      $uname = $item['value'][$uname_term];
      if (array_key_exists('entity', $item['value'])) {
        $eid = preg_replace('/TripalEntity:/', '', $item['value']['entity']);
        $uname = l($uname, 'bio_data/' . $eid);
      }

      $row =  array(
        $uname,
        $item['value'][$name_term],
        $item['value'][$type_term],
      );
      if ($field_table != 'organism') {
        $genus_term = tripal_get_chado_semweb_term('organism', 'genus');
        $species_term = tripal_get_chado_semweb_term('organism', 'species');
        $infraspecific_name_term = tripal_get_chado_semweb_term('organism', 'infraspecific_name');
        $infraspecific_type_term = tripal_get_chado_semweb_term('organism', 'type_id');
    
        $organism = $item['value'][$organism_term];
        $organism_name = '<i>' . $organism[$genus_term] . ' ' . $organism[$species_term]. '</i>';
        if (array_key_exists('infraspecific_type_term', $organism) and 
            $organism['infraspecific_type_term']) {
          $organism_name .= ' ' . $organism[infraspecific_type_term];
        }
        if (array_key_exists('infraspecific_name_term', $organism) and 
            $organism['infraspecific_name_term']) {
          $organism_name .= ' ' . $organism[infraspecific_name_term];
        }
        if (array_key_exists('entity', $organism)) {
          $eid = preg_replace('/TripalEntity:/', '',  $organism['entity']);
          $organism_name = l($organism_name, 'bio_data/' . $eid, array('html' => TRUE));
        }
        
        $row[] = $organism_name;
      }
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
        $caption = 'There is ' . count($rows) . ' network.';
    }
    else {
      $caption = 'There are ' . count($rows) . ' networks.';
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
      'empty' => t('There are no networks'),
    ));
    
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => $content . $pager,
    );
  }
}

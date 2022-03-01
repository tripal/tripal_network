<?php

class sio__attribute_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Network View';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('sio__attribute');

  /**
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    if (count($items) == 0) {
      return;
    }

    $attr_form = drupal_get_form('tripal_network_sio__attribute_formatter_form', $items);
    $element[0] = [
      '#type' => 'markup',
      '#markup' => drupal_render($attr_form),
    ];
  }
}

/**
 *
 */
function tripal_network_sio__attribute_formatter_form($form, &$form_state, $items = NULL) {

  $domain_term = chado_get_semweb_term('network_attr', 'domain_id');
  $attr_term = chado_get_semweb_term('network_attr', 'attr_id');

  $attribute = '';
  if (array_key_exists('values', $form_state)) {
    $attribute = array_key_exists('attribute', $form_state['values']) ? $form_state['values']['attribute'] : 0;
    $items = array_key_exists('items', $form_state['values']) ? $form_state['values']['items'] : $items;
  }

  $attributes = [
    0 => '-- Select --',
  ];

  // Organize the attributes:
  foreach ($items as $index => $item) {
    if (!empty($item['value'])) {
      $domain_type = $item['value'][$domain_term];
      $attributes[$domain_type . ' Attributes'][$index + 1] = ucfirst($item['value'][$attr_term]);
    }
  }

  $form['items'] = [
    '#type' => 'value',
    '#value' => $items,
    '#prefix' => '<div id="tripal-network-sio__attribute-form">',
  ];

  $form['info'] = [
    '#type' => 'markup',
    '#markup' => '<p>A network may have one or more attributes that are used
      to describe it and can be assigned to the graph, its nodes
      and/or edges. Use the drop-down below to explore the attributes in
      this network.'
  ];

  $form['attribute'] = [
    '#type' => 'select',
    '#title' => '',
    '#description' => t('Select an attribute of this network for more details about it.'),
    '#options' => $attributes,
    '#default_value' => $attribute,
    '#ajax' => array(
      'callback' => 'tripal_network_sio__attribute_form_ajax_callback',
      'wrapper'  => 'tripal-network-sio__attribute-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

  // If an attribute has been selected then show the details for it.
  if ($attribute) {
    $rows = [];
    $headers = ['Attribute Property', 'Value'];
    $item = $items[$attribute - 1];
    foreach ($item['value'] as $key => $value) {
      $key_term = chado_get_cvterm(['id' => $key]);
      if (is_array($value)) {
        $value = theme_item_list([
          'items' => $value,
          'title' => '',
          'attributes' => [],
          'type' => 'ul',
        ]);
      }
      $rows[] = [
        ucfirst($key_term->name),
        $value
      ];
    }
    $table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [
        'class' => ['table', 'tripal-network-attribute-table'],
      ],
      'caption' => '',
      'sticky' => FALSE,
      'colgroups' => [],
      'empty' => 'There is no information about this attribute.',
    ];
    $form['attr_details'] = [
      '#type' => 'markup',
      '#markup' => theme_table($table)
    ];
  }

  $form['suffix'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];

  // Make sure this file gets included so that the AJAX call back and form
  // functions can be found.
  form_load_include($form_state, 'inc', 'tripal_network', 'includes/TripalFields/sio__attribute/sio__attribute_formatter');
  return $form;
}

/**
 *
 */
function tripal_network_sio__attribute_form_ajax_callback($form, $form_state) {
  return $form;
}

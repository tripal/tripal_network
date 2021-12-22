<?php

/**
 *
 */
function tripal_network_viewer_page($organism_id = NULL, $network_id = NULL) {
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'drupal.form');
  $args = ['organism_id' => $organism_id, 'network_id' => $network_id];
  return theme('tripal_network_viewer', $args);
}

function tripal_network_viewer_ajax_init() {

  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;

  $init_state = [
    'network_id' => $network_id,
    'organism_id' => $organism_id,
    'selected_node' => null,
    'selected_edge' => null,
    'layer_by' => 'SWO:0000425',
    'color_by' => 'SWO:0000425',
    'limit' => 1000,
    'limit_by' => 'SWO:0000425'
  ];
  drupal_json_output($init_state);
}

/**
 *
 */
function tripal_network_viewer_ajax_get_layers_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_layers_form', $organism_id, $network_id, $state);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_filter_form() {
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_filter_form', $organism_id, $network_id, $state);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_network_details_form() {
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_network_details_form', $network_id);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_node_details() {
  $node_id = array_key_exists('node_id', $_GET) ? $_GET['node_id'] : NULL;
  $state = $_GET;

  $form = drupal_get_form('tripal_network_viewer_node_details_form', $node_id, $state);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_node_details_form($form, &$form_state, $node_id = NULL, $state = NULL) {

  $default_field = 'summary';
  if ($state and array_key_exists('node_field_name', $state)) {
    $default_field = $state['node_field_name'];
  }
  if (array_key_exists('values', $form_state)) {
    $node_id = array_key_exists('node_id', $form_state['values']) ? $form_state['values']['node_id'] : $node_id;
    $default_field = array_key_exists('field_name', $form_state['values']) ? $form_state['values']['field_name'] : $default_field;
  }

  $table_fields = [];
  $other_fields = [];
  $selectable_fields = [
    'summary' => 'Node Summary'
  ];
  $entity = NULL;

  $node_name = 'Click a node to view its details';
  if ($node_id) {
    $entity_id = chado_get_record_entity_by_table('feature', $node_id);
    $entities = tripal_load_entity('TripalEntity', [$entity_id]);
    field_attach_load('TripalEntity', $entities);
    $entity = NULL;
    if (count($entities) == 1) {
      $entity = $entities[$entity_id];
      $bundle = tripal_load_bundle_entity(['name' => $entity->bundle]);

      // Get information about the fields attached to this bundle and sort them
      // in the order they were set for the display.
      $instances = field_info_instances('TripalEntity', $bundle->name);
      foreach ($instances as $field_name => $instance) {

        // Skip hidden fields.
        if ($instance['display']['default']['type'] == 'hidden') {
          continue;
        }
        // Skip the network local viewer field
        if ($field_name == 'sio__network_diagram') {
          continue;
        }

        // Skip fields with no value
        $field = field_info_field($field_name);
        $field_items = field_get_items('TripalEntity', $entity, $field_name);
        if (tripal_field_is_empty($field, $field_items)) {
         // continue;
        }

        // Add this field for display in the summary table.
        if ($field['cardinality'] == 1) {
          $table_fields[$instance['display']['default']['weight']] = $instance;
        }
        else {
          $selectable_fields[$field_name] = $instance['label'];
          $other_fields[$field_name] = $instance;
        }
      }

      // Set the title for the panel.
      $node_name = $entity->title;
    }

    $details = '';
    if ($default_field == 'summary') {
      $rows = [];
      foreach ($table_fields as $instance) {
        $field_name  = $instance['field_name'];
        $display = $instance['display']['default'];
        $value = $entity->$field_name['und'][0]['value'];
        if ($instance['entity_type'] == 'TripalEntity') {
          $field = field_info_field($field_name);
          $items = field_get_items('TripalEntity', $entity, $field_name);
          $formatter_class = $display['type'];
          $element = [];
          if (tripal_load_include_field_class($formatter_class)) {
            $element=[];
            $formatter = new $formatter_class($field, $instance);
            $formatter->view($element, 'TripalEntity', $entity, 'und', $items, $display);
            $value = drupal_render($element);
          }
        }
        $rows[] = [
          [
            'data' => $instance['label'],
            'header' => TRUE,
            'width' => '20%',
          ],
          $value,
        ];
      }
      $headers = ['Attribute', 'Value'];
      $summary_table = [
        'header' => $headers,
        'rows' => $rows,
        'attributes' => [],
        'caption' => '',
        'sticky' => TRUE,
        'colgroups' => [],
        'empty' => 'No attributes for this node',
      ];
      $details = theme_table($summary_table);
    }
    else {
      $field = field_info_field($default_field);
      $instance = $other_fields[$default_field];
      $items = field_get_items('TripalEntity', $entity, $default_field);
      $display = $instance['display']['default'];
      $formatter_class = $display['type'];
      $element = [];
      if (tripal_load_include_field_class($formatter_class)) {
        $element = [];
        $formatter = new $formatter_class($field, $instance);
        $formatter->view($element, 'TripalEntity', $entity, 'und', $items, $display);
        $details = drupal_render($element);
      }
    }


    $form['feature_id'] = [
      '#type' => 'value',
      '#value' => $node_id,
      '#prefix' => '<div id="tripal-network-viewer-node-details-form">',
    ];
    $form['feature_name'] = [
      '#type' => 'item',
      '#title' => 'Node Name',
      '#markup' => $node_name,
    ];
    $form['field_name'] = [
      '#type' => 'select',
      '#title' => 'Node Information',
      '#options' => $selectable_fields,
      '#default_value' => $default_field,
      '#description' => t('Select the type of information to show about this node.'),
      '#ajax' => array(
        'callback' => 'tripal_network_viewer_node_details_form_ajax_callback',
        'wrapper'  => 'tripal-network-viewer-node-details-form',
        'effect'   => 'fade',
        'method'   => 'replace',
      ),
    ];


    $form['details'] = [
      '#type' => 'markup',
      '#markup' => $details,
      '#prefix' => '<div id="tripal-network-viewer-node-details-details">',
      '#suffix' => '</div>',
    ];
    $link = '';
    if ($entity) {
      $link = l("View full node page", 'bio_data/'. $entity->id, ['attributes' => ['target' => '_blank']]);
    }
    $form['link'] = [
      '#type' => 'markup',
      '#markup' => '<div id="tripal-network-viewer-node-link">' . $link  . '</div>',
      '#suffix' => '</div>',
    ];
  }
  else {
    $form['feature_name'] = [
      '#type' => 'markup',
      '#markup' => $node_name,
    ];
  }


  return $form;
}

/**
 *
 */
function tripal_network_viewer_ajax_get_edge_details() {
  $node_id = array_key_exists('edge_id', $_GET) ? $_GET['edge_id'] : NULL;

  $form = drupal_get_form('tripal_network_viewer_edge_details_form', $node_id);
  $form = drupal_render($form);

  $settings = FALSE;
  $javascript = drupal_add_js(NULL, NULL);
  if(isset($javascript['settings'], $javascript['settings']['data'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ';
    $settings .= drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data']));
    $settings .=  ');</script>';
  }
  print $form . $settings;
  die;
}

/**
 *
 */
function tripal_network_viewer_edge_details_form($form, &$form_state, $edge_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $edge_id = array_key_exists('edge_id', $form_state['values']) ? $form_state['values']['edge_id'] : $edge_id;
  }

  $edge_name = 'Click an edge to view its details';
  $net_name = '';
  $details = '';
  if ($edge_id) {
    $sql = "
      SELECT N.name as net_name, FS.name as source, FT.name as target
      FROM {network_edge} NE
        INNER JOIN {network} N on NE.network_id = N.network_id
        INNER JOIN {network_node} NNS on NNS.network_node_id = NE.source_id
        INNER JOIN {network_node} NNT on NNT.network_node_id = NE.target_id
        INNER JOIN {network_feature} NFS on NNS.network_node_id = NFS.network_node_id
        INNER JOIN {network_feature} NFT on NNT.network_node_id = NFT.network_node_id
        INNER JOIN {feature} FS on FS.feature_id = NFS.feature_id
        INNER JOIN {feature} FT on FT.feature_id = NFT.feature_id
     WHERE NE.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);
    $edge = $results->fetchObject();
    $edge_name = $edge->source . ' (-) ' . $edge->target;
    $net_name = $edge->net_name;

    $sql = "
      SELECT CVT.name, NEP.value, DBX.accession, DB.name as db_name
      FROM {network_edgeprop} NEP
        INNER JOIN {cvterm} CVT ON CVT.cvterm_id = NEP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE NEP.network_edge_id = :network_edge_id
    ";
    $args[':network_edge_id'] = $edge_id;
    $results = chado_query($sql, $args);

    $rows = [];
    while ($prop = $results->fetchObject()) {
      if ($prop->db_name . ':' . $prop->accession == 'local:KINC_sample_string') {
        continue;
      }
      $rows[] = [
        [
          'data' => ucfirst($prop->name),
          'header' => TRUE,
        ],
        $prop->value,
      ];
    }
    $headers = ['Attribute', 'Value'];
    $summary_table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [],
      'caption' => '',
      'sticky' => TRUE,
      'colgroups' => [],
      'empty' => 'No attributes for this edge',
    ];
    $details = theme_table($summary_table);
  }

  $form['edge_id'] = [
    '#type' => 'value',
    '#value' => $edge_id,
  ];
  $form['edge_name'] = [
    '#type' => 'item',
    '#title' => 'Edge Name',
    '#markup' => $edge_name,
  ];
  $form['net_name'] = [
    '#type' => 'item',
    '#title' => 'Network Name',
    '#markup' => $net_name,
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
    '#prefix' => '<div id="tripal-network-viewer-edge-details-details">',
    '#suffix' => '</div>',
  ];
  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_details_form($form, &$form_state, $network_id = NULL) {


  if (array_key_exists('values', $form_state)) {
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
  }

  $network_name = 'Select a network to view its details';
  $details = '';

  if ($network_id) {
    $network_name = 'Network Attributes';
    $sql = "
      SELECT CVT.name, NP.value, DBX.accession, DB.name as db_name
      FROM {networkprop} NP
        INNER JOIN {cvterm} CVT ON CVT.cvterm_id = NP.type_id
        INNER JOIN {dbxref} DBX on DBX.dbxref_id = CVT.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBX.db_id
      WHERE NP.network_id = :network_id
    ";
    $args[':network_id'] = $network_id;
    $results = chado_query($sql, $args);

    $rows = [];
    while ($prop = $results->fetchObject()) {
      $rows[] = [
        [
          'data' => ucfirst($prop->name),
          'header' => TRUE,
        ],
        $prop->value,
      ];
    }
    $headers = ['Attribute', 'Value'];
    $summary_table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [],
      'caption' => '',
      'sticky' => TRUE,
      'colgroups' => [],
      'empty' => 'No attributes for this edge',
    ];
    $details = theme_table($summary_table);
  }

  $form['network_id'] = [
    '#type' => 'value',
    '#value' => $network_id,
  ];
  $form['network_name'] = [
    '#type' => 'markup',
    '#markup' => $network_name,
  ];
  $form['details'] = [
    '#type' => 'markup',
    '#markup' => $details,
    '#prefix' => '<div id="tripal-network-viewer-network-details-details">',
    '#suffix' => '</div>',
  ];
  return $form;
}

function tripal_network_viewer_has_access($organism_id = NULL, $network_id = NULL) {
  global $user;

  if ($organism_id) {
    // Don't show an organism that the user doesn't have access to or isn't
    // published.
    $entity_id = chado_get_record_entity_by_table('organism', $organism_id);
    if (!$entity_id){
      return FALSE;
    }
    $entity = tripal_load_entity('TripalEntity', [$entity_id]);
    if (!tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
      return FALSE;
    }
  }

  if ($network_id) {
    // Don't show a network that the user doesn't have access to or isn't
    // published.
    $entity_id = chado_get_record_entity_by_table('network', $network_id);
    if (!$entity_id){
      return FALSE;
    }
    $entity = tripal_load_entity('TripalEntity', [$entity_id]);
    if (!tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
      return FALSE;
    }
  }
  return TRUE;
}
/**
 *
 */
function tripal_network_viewer_ajax_retrieve() {

  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $layer_by = array_key_exists('layer_by', $_GET) ? $_GET['layer_by'] : NULL;
  $color_by = array_key_exists('color_by', $_GET) ? $_GET['color_by'] : NULL;
  $limit = array_key_exists('limit', $_GET) ? $_GET['limit'] : 1000;
  $limit_by = array_key_exists('limit_by', $_GET) ? $_GET['limit_by'] : NULL;
  $features = array_key_exists('features', $_GET) ? $_GET['features'] : NULL;
  $vocab_terms = array_key_exists('vocab_terms', $_GET) ? $_GET['vocab_terms'] : NULL;

  $layout = tripal_network_get_default_layout();

  // TODO: this shoulnd't be hardcoded.
  if (!$layer_by) {
    $layer_by = 'SWO:0000425';
  }
  if (!$color_by) {
    $color_by = 'SWO:0000425';
  }

  if (!tripal_network_viewer_has_access($organism_id, $network_id)) {
    drupal_set_message('Permission denied', 'error');
    $organism_id = 0;
  }

  // If we don't have a organism ID then return an empty scene.
  $plotly_data = NULL;
  if (!$organism_id) {
    $plotly_data = tripal_network_make_plotly_data(NULL, $layout, $layer_by, $color_by, TRUE);
    drupal_json_output(['data' => $plotly_data, 'layout' => $layout]);
    return;
  }

  // If the user has request some feature names then limit the network
  // to only those features.
  $feature_ids = [];
  if ($features) {
    $features = preg_split('/\s+/', $features);
    $sql = "
      SELECT DISTINCT F.feature_id
      FROM {feature} F
        INNER JOIN {network_feature} NN on NN.feature_id = F.feature_id
        LEFT JOIN {feature_synonym} FS on FS.feature_id = F.feature_id
        LEFT JOIN {synonym} S on S.synonym_id = FS.synonym_id
      WHERE
        lower(F.name) = :name OR
        lower(F.uniquename) = :name OR
        lower(S.name) = :name
    ";
    foreach ($features as $fname) {
      $args = [':name' => strtolower($fname)];
      $feature = chado_query($sql, $args)->fetchObject();
      if ($feature) {
        $feature_ids[] = $feature->feature_id;
      }
    }
  }

  $term_ids = [];
  if ($vocab_terms) {
    $terms = preg_split('/\s+/', $vocab_terms);
    foreach ($terms as $term) {
      // TODO: add support for children temr
      $cvterm = chado_get_cvterm(['id' => $term]);
      if ($cvterm) {
        $term_ids[] = $cvterm->cvterm_id;
      }
    }
  }


  // Get the network Plotly data.
  $network_items = tripal_network_get_network_elements([
    'organism_id' => $organism_id, 'network_id' => $network_id,
    'feature_ids' => $feature_ids, 'term_ids' => $term_ids,
    'limit_by' => $limit_by, 'limit' => $limit]);
  $plotly_data = tripal_network_make_plotly_data($network_items, $layout, $layer_by, $color_by);

  drupal_json_output(['data' => $plotly_data, 'layout' => $layout]);
}

/**
 *
 */
function tripal_network_viewer_network_form($form, &$form_state, $organism_id = NULL,
    $network_id = NULL) {

  global $user;

  $features = '';
  $vocab_terms = '';
  $network_id = 0;
  $organism_id = 0;

  if (array_key_exists('values', $form_state)) {
    $features = array_key_exists('features', $form_state['values']) ? $form_state['values']['features'] : $features;
    $vocab_terms = array_key_exists('vocab_terms', $form_state['values']) ? $form_state['values']['vocab_terms'] : $vocab_terms;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
  }
  if (array_key_exists('input', $form_state)) {
    $features = array_key_exists('features', $form_state['input']) ? $form_state['input']['features'] : $features;
    $vocab_terms = array_key_exists('vocab_terms', $form_state['input']) ? $form_state['input']['vocab_terms'] : $vocab_terms;
    $network_id = array_key_exists('network_id', $form_state['input']) ? $form_state['input']['network_id'] : $network_id;
    $organism_id = array_key_exists('organism_id', $form_state['input']) ? $form_state['input']['organism_id'] : $organism_id;
  }
  
  $organism = NULL;
  if ($organism_id) {
    $organism = new ChadoRecord('organism');
    $organism->setValues(['organism_id' => $organism_id]);
    $organism->find();
  }
  
  $networks = [];
  $organisms = [
    0 => '-- Select --',
  ];
  
  // Get the list of organisms with networks.
  $args = [];
  $sql = "
      SELECT O.organism_id, O.genus, O.species
      FROM {network} N
        INNER JOIN {organism} O on O.organism_id = N.organism_id
      ORDER BY O.genus, O.species
  ";
  $results = chado_query($sql, $args);
  while ($record = $results->fetchObject()) {
    // Show networks that have been published and that the user has acces to.
    $entity_id = chado_get_record_entity_by_table('organism', $record->organism_id);
    if ($entity_id){
      $entity = tripal_load_entity('TripalEntity', [$entity_id]);
      if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
        $organisms[$record->organism_id] = $record->genus . ' ' . $record->species;
      }
    }
  }

  // Get the list of all networks
  if ($organism) {    
    $sql = '';
    $args = [];
    $sql = "
      SELECT N.name, N.network_id, CVT.name as cv_name
      FROM {network} N
        INNER JOIN {cvterm} CVT on CVT.cvterm_id = N.type_id
      WHERE N.organism_id = :organism_id
      ORDER BY N.name
    ";
    $args[':organism_id'] = $organism->getID();
    $results = chado_query($sql, $args);
    while ($record = $results->fetchObject()) {
      // Show networks that have been published and that the user has acces to.
      $entity_id = chado_get_record_entity_by_table('network', $record->network_id);
      if ($entity_id){
        $entity = tripal_load_entity('TripalEntity', [$entity_id]);
        if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
          $networks[$record->cv_name][$record->network_id] = $record->name;
        }
      }
    }
  }

  $form['organism_id'] = [
    '#type' => 'select',
    '#title' => 'Species',
    '#options' => $organisms,
    '#default_value' => $organism_id,
    '#description' => t('Select the orgnism to view networks.'),
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_network_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-network-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#prefix' => '<div id="tripal-network-viewer-network-form">',
  ];

  $title = 'Available Networks';
  if ($organism) {
    $title = 'Available ' . $organism->getValue('genus') . ' ' . $organism->getValue('species') . ' Networks';
  }

  $form['network_id'] = [
    '#type' => 'select',
    '#title' => $title,
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('Limit the edges to only the selected networks.'),
    '#multiple' => TRUE,
    '#size' => 5,
  ];

  $form['features'] = [
    '#type' => 'textarea',
    '#rows' => 3,
    '#title' => 'Feature Names',
    '#default_value' => $features,
    '#description' => t('Limit the edges to only those that connect
       features (e.g., gene, transcript, or protein names). Separate each
       name by a space or new line.'),
    '#multiple' => TRUE,
  ];
  $form['vocab_terms'] = [
    '#type' => 'textarea',
    '#rows' => 3,
    '#title' => 'Vocabulary Terms',
    '#default_value' => $vocab_terms,
    '#description' => t('Limit the edges to only those whose nodes are
       annotated with sepcific controlled vocabulary terms (e.g. GO Terms,
       IPR, KEGG ,etc).'),
    '#multiple' => TRUE,
  ];

  $form['button'] = [
    '#type' => 'submit',
    '#value' => 'Update Network View',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_network_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-network-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
  ];

  $form['done'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_network_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];
  $organism_id = $form_state['values']['organism_id'];
  $features = $form_state['values']['features'];
  $vocab_terms = $form_state['values']['vocab_terms'];

  $args = [
    'organism_id' => NULL,
    'network_id' => NULL,
    'features' => NULL,
    'vocab_terms' => NULL,
  ];
  if ($organism_id) {
    $args['organism_id'] = $organism_id;
  }

  if ($network_id) {
    $args['network_id'] = implode('|', $network_id);
  }
  if ($features) {
    $args['features'] = $features;
  }
  if ($vocab_terms) {
    $args['vocab_terms'] = $vocab_terms;
  }

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "updateFilterForm"));
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "updateLayersForm"));


  return $form;
}




/**
 *
 */
function tripal_network_viewer_filter_form($form, &$form_state,
    $organism_id = NULL, $network_id = NULL, $state = NULL) {

  $limit_by = '';
  $limit = '';

  if (is_array($state)){
    $organism_id = array_key_exists('organism_id', $state) ? $state['organism_id'] : $organism_id;
    $network_id = array_key_exists('network_id', $state) ? $state['network_id'] : $network_id;
    $limit_by = array_key_exists('limit_by', $state) ? $state['limit_by'] : $limit_by;
    $limit = array_key_exists('limit', $state) ? $state['limit'] : $limit;
  }

  if (array_key_exists('values', $form_state)) {
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : $limit;
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : $limit_by;
  }

  $props = [];
  if ($organism_id) {
    $props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, FALSE, TRUE, FALSE);
  }

  $form['network_id'] = [
    '#type' => 'value',
    '#value' =>  $network_id,
    '#prefix' => '<div id="tripal-network-viewer-filter-form">',
  ];
  $form['limit'] = [
    '#type' => 'textfield',
    '#title' => 'Limit Number of Edges',
    '#size' => 10,
    '#default_value' => $limit,
    '#description' => t('Limit the number of edges displayed.'),
  ];
  $form['limit_by'] = [
    '#type' => 'select',
    '#title' => 'Limit By',
    '#options' => $props,
    '#default_value' => $limit_by,
    '#description' => t('Select a property to limit the best edges (only works for numerical properties).'),
  ];

  $form['button'] = [
    '#type' => 'submit',
    '#value' => 'Perform Filter',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_filter_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-filter-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#suffix' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_layers_form($form, &$form_state,
    $organism_id = NULl, $network_id = NULL, $state = NULL) {

  $layer_by = '';
  $color_by = '';

  if (is_array($state)){
    $layer_by = array_key_exists('layer_by', $state) ? $state['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $state) ? $state['color_by'] : $color_by;
  }

  if (array_key_exists('values', $form_state)) {
    $organism_id = array_key_exists('organism_id', $form_state['values']) ? $form_state['values']['organism_id'] : $organism_id;
    $network_id = array_key_exists('network_id', $form_state['values']) ? $form_state['values']['network_id'] : $network_id;
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : $layer_by;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : $color_by;
  }
  $props = [];
  if ($organism_id) {
    $props = tripal_network_get_attribute_options($organism_id, $network_id, TRUE, TRUE, TRUE, FALSE);
  }

  $form['network_id'] = [
    '#type' => 'value',
    '#value' =>  $network_id,
    '#prefix' => '<div id="tripal-network-viewer-layers-form">',
  ];
  $form['layer_by'] = [
    '#type' => 'select',
    '#title' => 'Layer Edges By',
    '#options' => $props,
    '#default_value' => $layer_by,
    '#description' => t('Select a property to layer edges.'),
  ];

  $form['color_by'] = [
    '#type' => 'select',
    '#title' => 'Color Edges By',
    '#options' => $props,
    '#default_value' => $color_by,
    '#description' => t('Select a property to color edges.'),
  ];

  $form['button'] = [
    '#type' => 'submit',
    '#value' => 'Update Display',
    '#ajax' => array(
      'callback' => 'tripal_network_viewer_layers_form_ajax_callback',
      'wrapper'  => 'tripal-network-viewer-layers-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#suffix' => '</div>',
  ];

  return $form;
}

/**
 *
 */
function tripal_network_viewer_node_details_form_ajax_callback($form, $form_state){

  $field_name = NULL;
  if (array_key_exists('values', $form_state)) {
    $field_name = array_key_exists('field_name', $form_state['values']) ? $form_state['values']['field_name'] : NULL;
  }
  $args = [
    'node_field_name' => $field_name,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "setState", [$args]));
  return $form;
}

/**
 *
 */
function tripal_network_viewer_filter_form_ajax_callback($form, $form_state) {
  $limit = NULL;
  $limit_by = NULL;
  if (array_key_exists('values', $form_state)) {
    $limit = array_key_exists('limit', $form_state['values']) ? $form_state['values']['limit'] : NULL;
    $limit_by = array_key_exists('limit_by', $form_state['values']) ? $form_state['values']['limit_by'] : NULL;
  }

  $args = [
    'limit' => $limit,
    'limit_by' => $limit_by,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));

  return $form;
}
/**
 *
 */
function tripal_network_viewer_layers_form_ajax_callback($form, $form_state) {

  $layer_by = NULL;
  $color_by = NULL;
  if (array_key_exists('values', $form_state)) {
    $layer_by = array_key_exists('layer_by', $form_state['values']) ? $form_state['values']['layer_by'] : NULL;
    $color_by = array_key_exists('color_by', $form_state['values']) ? $form_state['values']['color_by'] : NULL;
  }

  $args = [
    'layer_by' => $layer_by,
    'color_by' => $color_by,
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetwork", [$args]));

  return $form;
}

/**
 *
 * @return
 */
function tripal_network_get_default_layout(){
  return [
    'autosize' => TRUE,
    'paper_bgcolor' => "#FFFFFF",
    'margin' => ['l' => 30, 'r' => 20, 't' => 50, 'b' => 20],
    'showlegend' => TRUE,
    'legend' => [
      'font' => [
        'color' => "#000000",
      ],
    ],
    'scene' => [
      'aspectmode' => 'manual',
      'aspectratio' => [
        'x' => 1,
        'y' => 1,
        'z' => 1
      ],
      'camera' => [
        'eye' => [
          'x' => 0,
          'y' => 0,
          'z' => 2
        ],

        'projection' => [
          'type' => 'orthographic'
        ]
      ],
      'xaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showbackground' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#000000"
      ],
      'yaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => FALSE,
        'title' => '',
        'showspikes' => FALSE,
        'color'=> "#000000"
      ],
      'zaxis' => [
        'type' => 'linear',
        'zeroline' => FALSE,
        'showline' => FALSE,
        'zeroline' => FALSE,
        'showgrid' => FALSE,
        'showticklabels' => TRUE,
        'showspikes' => FALSE,
        'title' => '',
        'color'=> "#000000"
      ]
    ],
    'title' => '',
  ];
}

/**
 *
 */
function tripal_network_neighborhood_viewer_ajax_retrieve() {
  global $user;

  // Get the fitering variables requested by the user.
  $network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
  $feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;

  // Set the Plotly layout sepcifications.
  // Set the Plotly layout sepcifications.
  $layout = tripal_network_get_default_layout();
  $layout['title'] = 'Connected Neighbors';

  // TODO: this shoulnd't be hardcoded.
  $layer_by = 'SWO:0000425';
  $color_by = 'SWO:0000425';

  if ($network_id) {
    $network = new ChadoRecord('network');
    $network->setValues(['network_id' => $network_id]);
    $network->find();
    $organism_id = $network->getValue('organism_id');
  }

  // Make sure the user has access.
  if (!tripal_network_viewer_has_access($organism_id, $network_id)) {
    $network_id = 0;
    $organism_id = 0;
  }

  if ($network_id) {

    // Make sure we have required filters.
    $plotly_data = NULL;
    if ($network_id == 0) {
      $plotly_data = tripal_network_make_plotly_data(NULL, $layout, $layer_by, $color_by, TRUE);
      drupal_json_output(['data' => $plotly_data, 'layout' => $layout]);
      return;
    }

    $network_items = tripal_network_get_network_elements([
      'organism_id' => $organism_id, 'network_id' => $network_id,
      'feature_ids' => [$feature_id]]);
    $plotly_data = tripal_network_make_plotly_data($network_items, $layout, $layer_by, $color_by);
    drupal_json_output(['data' => $plotly_data, 'layout' => $layout]);
  }
}

/**
 *
 */
function tripal_network_make_plotly_data($network_items, &$layout, $layer_by, $color_by, $is_empty = FALSE) {

  $edge_trace_defaults = [
    'mode' => 'lines',
    'type' => 'scatter3d',
    'line' => [
      'color' => [],
      'width' => 2,
      'opacity' => 0.9,
    ],
    'name' => 'Edge',
    'ids' => [],
    'hovertemplate'=> "%{text}",
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];

  $node_trace_defaults = [
    'mode'  => 'markers',
    'type' => 'scatter3d',
    'marker' => [
      'symbol' => 'circle',
      'color' => [],
      'size' => [],
      'sizemin' => 5,
      'opacity' => 0.9,
      'line' => [
        'width' => 2,
        'color' => 'rgb(100, 100, 100)',
      ],
    ],
    'hovertemplate'=> "%{text}",
    'name' => 'Node',
    'ids' => [],
    'x' => [],
    'y' => [],
    'z' => [],
    'text' => [],
  ];

  if ($is_empty) {
    return [$edge_trace_defaults, $node_trace_defaults];
  }

  $network_id = array_key_exists('network_id', $network_items) ? $network_items['network_id'] : NULL;
  $organism_id = array_key_exists('organism_id', $network_items) ? $network_items['organism_id'] : NULL;
  $edges = $network_items['edges'];
  $nodes = $network_items['nodes'];

  // Get properties of the nodes used for the viewer.
  $x_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id',
      'SIO:000400', $organism_id, $network_id);
  $y_coords = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id',
      'SIO:000401', $organism_id, $network_id);
  $degree = tripal_network_get_nodeprops(array_keys($nodes), 'feature_id',
      'OBI:0200130', $organism_id, $network_id);

  // Calculate teh x,y,z coordianates for nodes.
  $node_coords = [];
  foreach ($nodes as $node_id => $node_name) {
    $x = array_key_exists($node_id, $x_coords) ? $x_coords[$node_id] : mt_rand(0, 10);
    $y = array_key_exists($node_id, $y_coords) ? $y_coords[$node_id] : mt_rand(0, 10);
    $node_coords[$node_id] = [
      'x' => $x,
      'y' => $y,
      'z' => 0
    ];
  }

  // Get the layer and colors for the selected properties.
  $edge_ids = array_keys($edges);
  $layers = tripal_network_layer_edges($organism_id, $network_id, $edge_ids, $layer_by);
  $colors = tripal_network_color_edges($organism_id, $network_id, $edge_ids, $color_by);

  // Create the edges trace.
  $traces = [];
  foreach ($edges as $network_edge_id => $edge) {

    // Get the z-coord, bin and color for this edge.
    $layer = array_key_exists($network_edge_id, $layers['item_layer']) ? $layers['item_layer'][$network_edge_id] : 0;
    $color = array_key_exists($network_edge_id, $colors['item_color']) ? $colors['item_color'][$network_edge_id] : 0;
    $layer_name = $colors['labels'][$color];

    $z = 0;
    if ($layers['data_class'] == 'quantitative') {
      $z = array_key_exists($network_edge_id, $layers['adj_values']) ? $layers['adj_values'][$network_edge_id] : 0;
    }
    else {
      $z = $layer;
    }

    // Add a new trace if we haven't seen this bin yet.
    if (!array_key_exists($layer_name, $traces)) {
      $traces[$layer_name] = $edge_trace_defaults;
      $traces[$layer_name]['name'] = $layer_name;
      $traces[$layer_name]['line']['color'] = $color;
    }

    $source_feature_id = $edge['source_feature_id'];
    $target_feature_id = $edge['target_feature_id'];

    // Set the line x,y,z coordinates
    $traces[$layer_name]['x'][] = $node_coords[$source_feature_id]['x'];
    $traces[$layer_name]['y'][] = $node_coords[$source_feature_id]['y'];
    $traces[$layer_name]['z'][] = $z;
    $traces[$layer_name]['x'][] = $node_coords[$target_feature_id]['x'];
    $traces[$layer_name]['y'][] = $node_coords[$target_feature_id]['y'];
    $traces[$layer_name]['z'][] = $z;

    // Add an empty entry to keep Plotly from adding an extra line.
    $traces[$layer_name]['x'][] = NULL;
    $traces[$layer_name]['y'][] = NULL;
    $traces[$layer_name]['z'][] = NULL;

    // Set the line text.
    $text = $edge['source_name'] . ' (-) ' . $edge['target_name'];
    $traces[$layer_name]['text'][] = $text;
    $traces[$layer_name]['text'][] = $text;
    $traces[$layer_name]['text'][] = $text;

    // Set the line ids.
    $traces[$layer_name]['ids'][] = $network_edge_id;
    $traces[$layer_name]['ids'][] = $network_edge_id;
    $traces[$layer_name]['ids'][] = $network_edge_id;

    // Move the node's z-coordinate to the highest value score.
    if ($layers['data_class'] == 'quantitative') {
      $node_coords[$source_feature_id]['z'] = max($z, $node_coords[$source_feature_id]['z']);
      $node_coords[$target_feature_id]['z'] = max($z, $node_coords[$target_feature_id]['z']);
      $node_coords[$source_feature_id]['layer'] = $layer;
      $node_coords[$target_feature_id]['layer'] = $layer;

      // If the layer by is "rank" then put nodes at the lowest rank they
      // first appear. Otherwise, use the highest.
      if ($layer_by == 'NCIT:C48904' and $node_coords[$source_feature_id]['z'] != 0) {
        $node_coords[$source_feature_id]['z'] = min($z, $node_coords[$source_feature_id]['z']);
        $node_coords[$target_feature_id]['z'] = min($z, $node_coords[$target_feature_id]['z']);
        $node_coords[$source_feature_id]['layer'] = $layer;
        $node_coords[$target_feature_id]['layer'] = $layer;
      }
    }
    else {
      $node_coords[$source_feature_id]['z'] = $z;
      $node_coords[$target_feature_id]['z'] = $z;
      $node_coords[$source_feature_id]['layer'] = $layer;
      $node_coords[$target_feature_id]['layer'] = $layer;
    }
  }

  if ($colors['data_class'] == 'categorical') {
    ksort($traces);
  }
  else {
    ksort($traces, SORT_NUMERIC);
  }

  // Create the node trace
  $traces['nodes'] = $node_trace_defaults;
  foreach ($nodes as $node_id => $node) {

    // The bin for the node is the z coordinate assigned to the node.
    $layer = $node_coords[$node_id]['layer'];
    $traces['nodes']['x'][] = $node_coords[$node_id]['x'];
    $traces['nodes']['y'][] = $node_coords[$node_id]['y'];
    $traces['nodes']['z'][] = $node_coords[$node_id]['z'];
    $traces['nodes']['ids'][] = $node_id;
    $traces['nodes']['text'][] = $node['node_name'];
    $traces['nodes']['marker']['size'][] = max(round(log10($degree[$node_id])*10), 5);
    $traces['nodes']['marker']['color'][] = in_array($node_id, $network_items['feature_ids']) ? '#FFFF00': '#AAAAAA';
  }

  // Now set the tick labels
  if ($layers['data_class'] == 'categorical') {
    $layout['scene']['zaxis']['tickmode'] = "array";
    $layout['scene']['zaxis']['tickvals'] = $layers['layers'];
    $layout['scene']['zaxis']['ticktext'] = $layers['layer_labels'];
  }
  $layout['scene']['zaxis']['title'] = $layers['title'];

  return array_values($traces);
}

/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @param unknown $feature_id
 * @param unknown $network_id
 * @return unknown
 */
function tripal_network_neighborhood_viewer_form($form, &$form_state,
    $feature_id = NULL, $network_id = NULL) {

  global $user;

  // Get the list of modules
  $sql = '';
  $args = [':feature_id' => $feature_id];
  $sql = "
    SELECT * FROM {network} N
      INNER JOIN {network_node} NN on NN.network_id = N.network_id
      INNER JOIN {network_feature} NF on NF.network_node_id = NN.network_node_id
    WHERE NF.feature_id = :feature_id
    ORDER BY N.name
  ";
  $networks = [
    0 => '-- Select --',
  ];
  $results = chado_query($sql, $args);
  while ($record = $results->fetchObject()) {
    // Show networks that have been published and that the user has acces to.
    $entity_id = chado_get_record_entity_by_table('network', $record->network_id);
    if ($entity_id){
      $entity = tripal_load_entity('TripalEntity', [$entity_id]);
      if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
        $networks[$record->network_id] = $record->name;
      }
    }
  }

  $form['feature_id'] = [
    '#type' => 'value',
    '#value' => $feature_id,
    '#prefix' => '<div id="tripal-network-neighborhood-viewer-form">',
  ];
  $form['network_id'] = [
    '#type' => 'select',
    '#title' => 'Network',
    '#options' => $networks,
    '#default_value' => $network_id,
    '#description' => t('Select the network to view connections for this feature.'),
    '#ajax' => array(
      'callback' => 'tripal_network_neighborhood_viewer_form_ajax_callback',
      'wrapper'  => 'tripal-network-neighborhood-viewer-form',
      'effect'   => 'fade',
      'method'   => 'replace',
    ),
    '#suffix' => '</div>',
  ];
  return $form;
}

/**
 *
 */
function tripal_network_neighborhood_viewer_form_ajax_callback($form, $form_state) {
  $network_id = $form_state['values']['network_id'];
  $feature_id = $form_state['values']['feature_id'];

  $args = [
    'feature_id' => $feature_id,
    'network_id' => $network_id,
    'viewer_id' => 'tripal-network-neighborhood-viewer'
  ];

  // Tell Drupal to update the network
  tripal_network_add_ajax_command(ajax_command_invoke(NULL, "getNetworkNeighborhood", [$args]));

  return $form;
}
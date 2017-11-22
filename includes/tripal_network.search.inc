<?php

function theme_tripal_network_search_form($form) {
  $rows = array(
    array(
      // column 1
      drupal_render($form['species']) . '<br>' .
      drupal_render($form['networks']) .
      drupal_render($form['submit']) .
      drupal_render($form['reset']),
      // column 2
      drupal_render($form['instructions']) .
      drupal_render($form['filter_by_mod_name']) .
      drupal_render($form['filter_by_gene_name']) .
      drupal_render($form['filter_by_annotation']) .
      drupal_render($form['filter_by_topology']) .
      drupal_render($form['filter_by_trait']),
    ),
  );
  $headers = array('Select a Network', 'Filters');
  $form['criteria'] = array(
    '#type' => 'markup',
    '#value' =>  theme('table', $headers, $rows, array('id' => 'networkmod_search_form')),
    '#prefix' => '<div id="networkmod-search-form">',
    '#suffix' => '</div>',
  );

  return drupal_render($form);
}
/*
 *
 */
function tripal_network_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == "tripal_network_search_form") {
    // the tripal_network_search_form cann be updated through ajax
    // so the form #action gets set to the ajax URL.  This isn't
    // the correct submit URL so we need to fix it here.
    $form['#action'] = url("network/search/modules");
  }
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_form($form_state = NULL) {
  $form = array();

  $dmodule       = '';
  $dgenes        = '';
  $dannotation   = '';
  $denriched     = '';
  $dspecies      = '';
  $dnetworks     = '';
  $dtraits       = '';
  $dtypes        = '';
  $doverlaps     = '';
  $dnum_nodes    = '';
  $dnum_nodes_op = '';
  $dnum_edges    = '';
  $dnum_edges_op = '';

  if (array_key_exists('network_search_criteria', $_SESSION)) {
    $dmodule       = $_SESSION['network_search_criteria']['module'];
    $dgenes        = $_SESSION['network_search_criteria']['genes'];
    $dannotation   = $_SESSION['network_search_criteria']['annotation'];
    $denriched     = $_SESSION['network_search_criteria']['enriched'];
    $dspecies      = $_SESSION['network_search_criteria']['species'];
    $dnetworks     = $_SESSION['network_search_criteria']['networks'];
    $dtraits       = $_SESSION['network_search_criteria']['traits'];
    $dtypes        = $_SESSION['network_search_criteria']['types'];
    $doverlaps     = $_SESSION['network_search_criteria']['overlaps'];
    $dnum_nodes    = $_SESSION['network_search_criteria']['num_nodes'];
    $dnum_nodes_op = $_SESSION['network_search_criteria']['num_nodes_op'];
    $dnum_edges    = $_SESSION['network_search_criteria']['num_edges'];
    $dnum_edges_op = $_SESSION['network_search_criteria']['num_edges_op'];
  }

  // get default values, we always want to first choose the form
  // state values. This occurs when the form is submitted or the
  // form is changed through an AJAX call.  Second, we
  // want to choose what's in the session varriable.  This occurs
  // when the form is built due to a paging or sorting of the results
  // on the same page which causes the form to be reloaded.
  if (array_key_exists('values', $form_state)) {
    $dmodule       = $form_state['values']['module'] ? $form_state['values']['module'] : $dmodule;
    $dgenes        = $form_state['values']['genes'] ? $form_state['values']['genes'] : $dgenes;
    $dannotation   = $form_state['values']['annotation'] ? $form_state['values']['annotation'] : $dannotation;
    $denriched     = $form_state['values']['enriched'] ? $form_state['values']['enriched'] : $denriched;
    $dspecies      = $form_state['values']['species'] ? $form_state['values']['species'] : $dspecies;
    $dnetworks     = $form_state['values']['networks'] ? $form_state['values']['networks'] : $dnetworks;
    $dtraits       = $form_state['values']['traits'] ? $form_state['values']['traits'] : $dtraits;
    $dtypes        = $form_state['values']['types'] ? $form_state['values']['types'] : $dtypes;
    $doverlaps     = $form_state['values']['overlaps'] ? $form_state['values']['overlaps'] : $doverlaps;
    $dnum_nodes    = $form_state['values']['num_nodes'] ? $form_state['values']['num_nodes'] : $dnum_nodes;
    $dnum_nodes_op = $form_state['values']['num_nodes_op'] ? $form_state['values']['num_nodes_op'] : $dnum_nodes_op;
    $dnum_edges    = $form_state['values']['num_edges'] ? $form_state['values']['num_edges'] : $dnum_edges;
    $dnum_edges_op = $form_state['values']['num_edges_op'] ? $form_state['values']['num_edges_op'] : $dnum_edges_op;
  }

  // we need to make sure we have default network, type and traits set to be
  // all inclusive
  if (!is_array($dnetworks)) {
    if (!$dnetworks) {
      $dnetworks = 0;  // set the default to 'Any Network'
    }
    $dnetworks = array($dnetworks);
  }
  if (!is_array($dtypes)) {
    if (!$dtypes) {
      $dtypes = 'Any';  // set the default to 'Any Type'
    }
    $dtypes = array($dtypes);
  }
  if (!is_array($dtraits)) {
    if (!$dtraits) {
      $dtraits = 0;  // set the default to 'Any Trait'
    }
    $dtraits = array($dtraits);
  }

  // collapse the search fields if we have results
  $is_collaped = FALSE;
  if (isset($_SESSION['network_search_criteria']['has_results'])) {
    $is_collapsed = TRUE;
  }
  $species = array();
  $sql = "SELECT organism_id, genus, species FROM {organism} ORDER BY genus, species";
  $results = chado_query($sql);
  while ($organism = $results->fetchObject()) {
    // if we don't have a default species use the first one
    if (!$dspecies or $dspecies == 0) {
      $dspecies = $organism->organism_id;
    }
    $species[$organism->organism_id] = $organism->genus . " " . $organism->species;
  }

  $form['criteria'] = array(
    '#type' => 'markup',
    '#value' => '',
    '#prefix' => '<div id="networkmod-search-form">',
    '#suffix' => '</div>',
  );

  $form['species'] = array(
    '#title'         => t('Species'),
    '#type'          => t('select'),
    '#description'   => t("Select a species to limit results."),
    '#default_value' => $dspecies,
    '#options'       => $species,
    '#multiple'      => FALSE,
    '#prefix' => '<div id="networkmod-search-species">',
    '#suffix' => '</div>',
    '#ahah'   => array(
      'event'   => 'change',
      'path'    => 'network/search/modules/networks',
      'wrapper' => 'networkmod-search-form',
      'method'  => 'replace',
      //'effect'  => 'fade',
    ),
    '#size' => 5,
  );

  $networks = array();
  if ($dspecies) {
    $networks[0] = 'Any network';
    $sql = "SELECT network_id, name FROM {network} WHERE organism_id = :organism ";
    $sql .= "ORDER BY name";

    $results = chado_query($sql, array(':organism' => $dspecies));
    while ($network = $results->fetchObject()) {
      $networks[$network->network_id] = $network->name;
    }
  }
  else {
    $networks[] = 'Select a species';
  }
  $form['networks'] = array(
    '#title'         => t('Networks'),
    '#type'          => t('select'),
    '#description'   => t("Select a network to limit results."),
    '#default_value' => $dnetworks,
    '#options'       => $networks,
    '#multiple'      => TRUE,
    '#prefix' => '<div id="networkmod-search-networks">',
    '#suffix' => '</div>',
    '#ahah'   => array(
      'event'   => 'change',
      'path'    => 'network/search/modules/traits',
      'wrapper' => 'networkmod-search-form',
      'method'  => 'replace',
      //'effect'  => 'fade',
    ),
    '#size' => 10,
  );

  $form['instructions'] = array(
    '#type'        => 'markup',
    '#value'       => '<p>' . t('You may filter the modules using any of the criteria specified below') . '</p>',
  );

  ($dmodule) ? $collapsed = FALSE : $collapsed = TRUE;
  $form['filter_by_mod_name'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('I want to find a specific module by name'),
    '#collapsible' => TRUE,
    '#collapsed'   => $collapsed,
    '#prefix'      => '<div id="filter_by_mod_name">',
    '#suffix'      => '</div>',
  );
  $form['filter_by_mod_name']['module'] = array(
    '#type' => 'textfield',
    '#title' => t('Module name'),
    '#description' => t('Enter a module name'),
    '#default_value' => $dmodule,
    '#size' => 30,
    '#prefix' => '<div id="networkmod-search-module">',
  );

  ($dgenes) ? $collapsed = FALSE : $collapsed = TRUE;
  $form['filter_by_gene_name'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('I want to find modules that contain a specific gene'),
    '#collapsible' => TRUE,
    '#collapsed'   => $collapsed,
    '#prefix'      => '<div id="filter_by_gene_name">',
    '#suffix'      => '</div>',
  );
 $form['filter_by_gene_name']['genes'] = array(
    '#type' => 'textarea',
    '#title' => t('Nodes (Genes)'),
    '#description' => t('Enter one or more gene identifiers to find modules where they are co-expressed (e.g. LOC_Os01g61780). Separate by comma, new line.'),
    '#default_value' => $dgenes,
    '#rows' => 3,
    '#cols' => 15,
    '#prefix' => '<div id="networkmod-search-genes">',
    '#suffix' => '</div></div>'
  );

  ($dannotation) ? $collapsed = FALSE : $collapsed = TRUE;
  $form['filter_by_annotation'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('I want to find modules enriched for a specific annotation'),
    '#collapsible' => TRUE,
    '#collapsed'   => $collapsed,
    '#prefix'      => '<div id="filter_by_annotation">',
    '#suffix'      => '</div>',
  );
  $form['filter_by_annotation']['annotation'] = array(
    '#type' => 'textarea',
    '#title' => t('Annotations'),
    '#description' => t('Enter one or more annotation descriptions or term identifiers to find modules where the nodes are annotated with these terms. Separate each functional description or term on new line. Spaces can be used between words of a description.'),
    '#default_value' => $dannotation,
    '#rows' => 3,
    '#cols' => 15,
    '#prefix' => '<div id="networkmod-search-annotation">',
  );
 $form['filter_by_annotation']['enriched'] = array(
    '#type' => 'checkbox',
    '#title' => t('Find Only Enriched Terms'),
    '#description' => t('Only find annotations that are enriched (or significantly over-represented in comparision to genomic background).'),
    '#default_value' => $denriched,
    '#size' => 30,
    '#suffix' => '</div>'
  );

  ($dnum_edges or $dnum_nodes) ? $collapsed = FALSE : $collapsed = TRUE;
  $form['filter_by_topology'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('I want to filter modules by topological properties'),
    '#collapsible' => TRUE,
    '#collapsed'   => $collapsed,
    '#prefix'      => '<div id="filter_by_topology">',
    '#suffix'      => '</div>',
  );
  $op_options = array(
    'gt' => 'greater than',
    'lt' => 'less than',
    'eq' => 'equal to',
  );
 $form['filter_by_topology']['num_edges-title'] = array(
    '#type' => 'markup',
    '#value' => '<b>Number of Edges</b>',
    '#prefix' => '<div id="networkmod-search-num-edges">',
  );
 $form['filter_by_topology']['num_edges_op'] = array(
    '#type' => 'select',
    '#options' => $op_options,
    '#default_value' => $dnum_edges_op,
  );
 $form['filter_by_topology']['num_edges'] = array(
    '#type' => 'textfield',
    '#description' => t('Find modules by number of edges.'),
    '#default_value' => $dnum_edges,
    '#size' => 10,
    '#suffix' => '</div>'
  );
 $form['filter_by_topology']['num_nodes-title'] = array(
    '#type' => 'markup',
    '#value' => '<b>Number of Nodes</b>',
    '#prefix' => '<div id="networkmod-search-num-nodes">',
  );
 $form['filter_by_topology']['num_nodes_op'] = array(
    '#type' => 'select',
    '#options' => $op_options,
    '#default_value' => $dnum_nodes_op,
  );
 $form['filter_by_topology']['num_nodes'] = array(
    '#type' => 'textfield',
    '#description' => t('Find modules by number of nodes.'),
    '#default_value' => $dnum_nodes,
    '#size' => 10,
    '#suffix' => '</div>'
  );

  $traits = array();
  $types = array();
  if ($dspecies or $dnetworks) {
    $traits[0] = 'Any trait';
    $types['Any'] = 'Any type';
    // TODO: These are hard coded and need not be
    $types['QTL'] = 'QTL';
    $types['SNP'] = 'GWAS SNP';
    $sql = "SELECT * FROM networkmod_traits WHERE 1=1 ";
    $args = array();
    if ($dspecies) {
      $sql .= "AND organism_id = %d ";
      $args[] = $dspecies;
    }
    if ($dnetworks) {
      if (count($dnetworks) > 1 or (count($dnetworks) ==1 and !in_array(0, $dnetworks))) {
        $sql .= "AND network_id IN ( ";
        foreach ($dnetworks as $network_id) {
          $sql .= "%d, ";
          $args[] = $network_id;
        }
        $sql = substr($sql, 0, -2);
        $sql .= ") ";
      }
    }
    if ($dtypes) {
      if (count($dtypes) > 1 or (count($dtypes) == 1 and !in_array('Any', $dtypes))) {
        $sql .= "AND feature_type IN ( ";
        foreach ($dtypes as $feature_type) {
          $sql .= "'%s', ";
          $args[] = $feature_type;
        }
        $sql = substr($sql, 0, -2);
        $sql .= ") ";
      }
    }
    $sql .= " ORDER BY trait_name";
    $results = chado_query($sql, $args);
    while ($trait = db_fetch_object($results)) {
      $traits[$trait->trait_dbxref_id] = ucfirst($trait->trait_name);
    }
    if (count($traits) == 1) {
      $traits = array('No Traits Available');
    }
  }
  else {
    $traits[] = 'Please choose a species and network';
  }

  $collapsed = TRUE;
  if (count($dtraits) > 1 or count($dtypes) > 1 ) {
    $collapsed = FALSE;
  }
  if (count($dtraits) == 1 and !in_array(0, $dtraits)) {
    $collapsed = FALSE;
  }
  if (count($dtypes) == 1 and !in_array('Any', $dtypes)) {
    $collapsed = FALSE;
  }
  $form['filter_by_trait'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('I want to search for modules overlapping mapped traits'),
    '#collapsible' => TRUE,
    '#collapsed'   => $collapsed,
    '#prefix'      => '<div id="filter_by_trait">',
    '#suffix'      => '</div>',
  );
  $form['filter_by_trait']['types'] = array(
    '#title'         => t('Genetic Feature Types'),
    '#type'          => t('select'),
    '#description'   => t("Select a genetic feature type to limit traits."),
    '#default_value' => $dtypes,
    '#options'       => $types,
    '#multiple'      => TRUE,
    '#prefix' => '<div id="networkmod-search-types">',
    '#suffix' => '</div>',
    '#ahah'   => array(
      'event'   => 'change',
      'path'    => 'network/search/modules/traits',
      'wrapper' => 'networkmod-search-form',
      'method'  => 'replace',
      //'effect'  => 'fade',
    ),
  );
  $form['filter_by_trait']['traits'] = array(
    '#title'         => t('Traits'),
    '#type'          => t('select'),
    '#description'   => t("Select a trait to limit results."),
    '#default_value' => $dtraits,
    '#options'       => $traits,
    '#multiple'      => TRUE,
    '#prefix' => '<div id="networkmod-search-traits">',
    '#suffix' => '</div>',
    '#size' => 10,
  );
  if (!$doverlap) {
    $doverlap = 2;
  }
  /*
  $form['filter_by_trait']['overlaps'] = array(
    '#title'         => t('Genetic Overlap'),
    '#type'          => 'textfield',
    '#description'   => t("By default a trait is shown in the report if a node in an edge
       overlaps with at least one genetic feature. Change this number to require that an
       edge overlap with more than just one feature."),
    '#default_value' => $doverlaps,
    '#prefix'        => '<div id="networkmod-search-overlap">',
    '#suffix'        => '</div>',
    '#size'          => 5,
  );*/

 $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Search',
    '#prefix' => '<div id="networkmod-search-submit">',
    '#suffix' => '</div>'
  );
   $form['reset'] = array(
    '#type' => 'submit',
    '#value' => 'Reset',
    '#prefix' => '<div id="networkmod-search-reset">',
    '#suffix' => '</div>'
  );
  return $form;
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_get_networks() {
  $status = TRUE;

  // get the networks form element. It will be populated
  // with networks because the form was prepared with the
  // form values when the ahah call was made.
  $form = tripal_core_ahah_prepare_form();
  $data = theme('tripal_network_search_form', $form);

  // bind javascript events to the new objects that will be returned
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status,
      'data'     => $data,
      'settings' => $settings,
    )
  );
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_get_traits() {
  $status = TRUE;

  // get the networks form element. It will be populated
  // with networks because the form was prepared with the
  // form values when the ahah call was made.
  $form = tripal_core_ahah_prepare_form();
  $data = theme('tripal_network_search_form', $form);

  // bind javascript events to the new objects that will be returned
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status,
      'data'     => $data,
      'settings' => $settings,
    )
  );
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_form_validate($form, &$form_state) {
  $module        = trim($form_state['values']['module']);
  $genes         = trim($form_state['values']['genes']);
  $annotation    = trim($form_state['values']['annotation']);
  $enriched      = trim($form_state['values']['enriched']);
  $species       = $form_state['values']['species'];
  $networks      = $form_state['values']['networks'];
  $traits        = $form_state['values']['traits'];
  $num_edges     = trim($form_state['values']['num_edges']);
  $num_edges_op  = trim($form_state['values']['num_edges_op']);
  $num_nodes     = trim($form_state['values']['num_nodes']);
  $num_nodes_op  = trim($form_state['values']['num_nodes_op']);
  $types         = $form_state['values']['types'];
  $overlaps      = trim($form_state['values']['overlaps']);

  if ($num_edges and !is_numeric($num_edges)) {
    form_set_error('num_edges', "Please enter a numeric value.");
    unset($_SESSION['network_search_criteria']);
  }
  if ($num_nodes and!is_numeric($num_nodes)) {
    form_set_error('num_edges', "Please enter a numeric value.");
    unset($_SESSION['network_search_criteria']);
  }
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_form_submit(&$form, &$form_state) {
  $module        = trim($form_state['values']['module']);
  $genes         = trim($form_state['values']['genes']);
  $annotation    = trim($form_state['values']['annotation']);
  $enriched      = trim($form_state['values']['enriched']);
  $species       = trim($form_state['values']['species']);
  $networks      = $form_state['values']['networks'];
  $traits        = $form_state['values']['traits'];
  $num_edges     = trim($form_state['values']['num_edges']);
  $num_edges_op  = trim($form_state['values']['num_edges_op']);
  $num_nodes     = trim($form_state['values']['num_nodes']);
  $num_nodes_op  = trim($form_state['values']['num_nodes_op']);
  $types         = $form_state['values']['types'];
  $overlaps      = trim($form_state['values']['overlaps']);
  $op            = $form_state['values']['op'];

  if ($op == 'Reset') {
    unset($_SESSION['network_search_criteria']);
    unset($form_state['values']);
  }
  else {
    $_SESSION['network_search_criteria'] = array(
      'module' => $module,
      'genes' => $genes,
      'annotation' => $annotation,
      'enriched' => $enriched,
      'species' => $species,
      'networks' => $networks,
      'traits' => $traits,
      'num_nodes' => $num_nodes,
      'num_nodes_op' => $num_nodes_op,
      'num_edges' => $num_edges,
      'num_edges_op' => $num_edges_op,
      'has_results' => FALSE,
      'types' => $types,
      'overlaps' => $overlaps,
    );
  }
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_page() {
  $form = drupal_get_form('tripal_network_search_form');
  $results = tripal_network_search_page_results();
  return $form . '<p><b>Results</b></br>' . $results . '</p>';
}
/**
 *
 *
 * @ingroup tripal_network
 */
function tripal_network_search_page_results() {

  if(!isset($_SESSION['network_search_criteria'])) {
    return;
  }

  // get the form values
  $module       = $_SESSION['network_search_criteria']['module'];
  $genes        = $_SESSION['network_search_criteria']['genes'];
  $annotation   = $_SESSION['network_search_criteria']['annotation'];
  $enriched     = $_SESSION['network_search_criteria']['enriched'];
  $species      = $_SESSION['network_search_criteria']['species'];
  $networks     = $_SESSION['network_search_criteria']['networks'];
  $traits       = $_SESSION['network_search_criteria']['traits'];
  $types        = $_SESSION['network_search_criteria']['types'];
  $overlaps     = $_SESSION['network_search_criteria']['overlaps'];
  $num_nodes    = $_SESSION['network_search_criteria']['num_nodes'];
  $num_nodes_op = $_SESSION['network_search_criteria']['num_nodes_op'];
  $num_edges    = $_SESSION['network_search_criteria']['num_edges'];
  $num_edges_op = $_SESSION['network_search_criteria']['num_edges_op'];

  if (!is_numeric($overlaps)) {
    $overlaps = 0;
  }

  $order = $_GET['order'];
  $sort = $_GET['sort'];

  $args = array();
  $out_args = array();
  $select = "
    SELECT N.name as network_name, NM.name as module_name,
      NM.networkmod_id, O.genus, O.species, CN.nid as network_nid,
      (SELECT count(*)
       FROM networkmod_node NN
       WHERE NN.networkmod_id = NM.networkmod_id) as num_nodes,
      (SELECT count(*)
       FROM networkmod_edge NE
       WHERE NE.networkmod_id = NM.networkmod_id) as num_edges
  ";
  $from = "
    FROM networkmod NM
      INNER JOIN network N on NM.network_id = N.network_id
      INNER JOIN organism O on O.organism_id = N.organism_id
      LEFT JOIN chado_network CN on CN.network_id = N.network_id
  ";
  $group = "\nGROUP BY N.name, NM.name, NM.networkmod_id, O.genus, O.species, CN.nid ";
  $where = "";
  $where_out = "";

  // if a species is provided, add it to the where clause.
  if ($species) {
    $where .= "O.organism_id = %d AND ";
    $args[] = $species;
  }

  // if a network is provided, add it to the where clause
  if (count($networks) > 0) {
    if (count($networks) == 1 and !in_array(0, $networks)) {
      $block = '';
      foreach ($networks as $network_id) {
        if ($network_id != 0) {
          $block .= "N.network_id = %d OR ";
          $args[] = $network_id;
        }
      }
      if ($block) {
        $block = substr($block, 0, -3);
        $where .= "($block) AND ";
      }
    }
  }

  // if traits are provided, add join to the from and items to the where clause
  if (count($traits) > 0) {
    if (count($traits) > 1 or (count($traits) == 1 and !in_array(0, $traits))) {
      $block = '';
      $select .= ", array_agg(NMET.feature_type || ':' || NMET.feature_name || ':' || NMET.trait_name) as matching_traits ";
      $from .= " INNER JOIN networkmod_exp_traits NMET ON NMET.networkmod_id = NM.networkmod_id ";

      foreach ($traits as $trait_dbxref_id) {
        if ($trait_dbxref_id != 0) {
          $block .= "NMET.trait_dbxref_id = %d OR ";
          $args[] = $trait_dbxref_id;
        }
      }
      if ($block) {
        $block = substr($block, 0, -3);
        $where .= " ($block) AND ";
      }
      // if the user wants to limit by genetic feature types (e.g. SNP or QTL)
      if (count($types) > 0) {
        if (count($types) > 1 or (count($types) == 1 and !in_array('Any', $types))) {
          $where .= " NMET.feature_type IN ( ";
          foreach ($types as $feature_type) {
             $where .= "'%s', ";
             $args[] = $feature_type;
          }
          $where = substr($where, 0, -2);
          $where .= ") AND ";
        }
      }
    }
  }

  // if a module is provided, add it to the where clause
  if ($module) {
    $where .= "NM.name = '%s' AND ";
    $args[] = $module;
  }

  // if searching by gene or annotations we need some additional tables
  if ($genes or $annotation) {
    $from .= "
      INNER JOIN networkmod_node NMN ON NMN.networkmod_id = NM.networkmod_id
      INNER JOIN feature F on NMN.node = F.feature_id
    ";
  }

  // if a gene is provided, add it to the where and join clauses
  if ($genes) {
    $items = $genes;

    // remove extra spaces and new lines
    $items = preg_replace("/\s+,/", ",", $items);
    $items = preg_replace("/\s+\n/", "\n", $items);
    $items = preg_replace("/,\n/", "\n", $items);

    // split values on newline or comma
    $values = preg_split("/[\n,]+/", $items);
    if(count($values) == 0){
      $values = array($genes);
    }

    $where .= "(  ";
    foreach ($values as $val){
      $where .= "F.name = '%s' OR F.uniquename = '%s' OR ";
      $args[] = $val;
      $args[] = $val;
    }
    $where = substr($where, 0, -3); // remove trailing 'OR ';
    $where .= ") AND ";
  }

  // if annotations are provided
  if ($annotation) {
    $items = $annotation;
    // remove extra spaces and new lines
    $items = preg_replace("/\s+,/", ",", $items);
    $items = preg_replace("/\s+\n/", "\n", $items);
    $items = preg_replace("/,\n/", "\n", $items);

    // split values on newline
    $values = preg_split("/[\n]+/", $items);
    if(count($values) == 0){
      $values = array($annotation);
    }
    // add an aggregator to the select
    $select .= ", array_agg(F.name) as matching_features";
    $select .= ", array_agg(FCVT_DB.name || ':' ||  FCVT_DBX.accession || ' ' || FCVT_CVT.name) as matching_terms";

    // add in the feature_dbxref records
    $from .= "
      INNER JOIN feature_cvterm FCVT ON FCVT.feature_id = F.feature_id
      INNER JOIN cvterm FCVT_CVT ON FCVT_CVT.cvterm_id = FCVT.cvterm_id
      INNER JOIN dbxref FCVT_DBX ON FCVT_DBX.dbxref_id = FCVT_CVT.dbxref_id
      INNER JOIN db FCVT_DB ON FCVT_DB.db_id = FCVT_DBX.db_id
    ";
     $where .= "(  ";
     foreach ($values as $val){
       // some accession are a concatenation of a numeric ID and the db
       // name. Others are just the text itself. Look to see if the
       // accession is simply numeric and then concatenate the DB
       $matches = array();
       if (preg_match('/^(.*?):(.*?)$/', $val, $matches)) {
         $where .= " (FCVT_DB.name = '%s' AND FCVT_DBX.accession = '%s') OR ";
         $args[] = $matches[1];
         $args[] = $matches[2];
       }
       else {
         $where .= " (FCVT_DBX.accession = '%s') OR ";
         $args[] = $val;
       }
       // also look for this term as a keyword
       $where .= "lower(FCVT_CVT.name) like lower('%%%s%%') OR ";
       $args[] = $val;
     }
     $where = substr($where, 0, -3); // remove trailing 'OR ';
     $where .= ") AND ";
  }

  if ($enriched and $annotation){
    $from .= "
      INNER JOIN networkmod_cvterm NMCVT on FCVT.cvterm_id = NMCVT.cvterm_id
        AND NMCVT.networkmod_id = NM.networkmod_id
      INNER JOIN networkmod_cvtermprop NMCVTP on NMCVTP.networkmod_cvterm_id = NMCVT.networkmod_cvterm_id
      INNER JOIN cvterm NMCVTP_CVT on NMCVTP_CVT.cvterm_id = NMCVTP.type_id
        AND (NMCVTP_CVT.name = 'fishers_pvalue')
    ";
  }

  // filter by node count
  if ($num_nodes) {
    if($num_nodes_op == 'lt') {
      $where_out .= " num_nodes < %d AND ";
    }
    if($num_nodes_op == 'gt') {
      $where_out .= " num_nodes > %d AND ";
    }
    if($num_nodes_op == 'eq') {
      $where_out .= " num_nodes = %d AND ";
    }
    $out_args[] = $num_nodes;
  }

  // filter by edge count
  if ($num_edges) {
    if($num_edges_op == 'lt') {
      $where_out .= " num_edges < %d AND ";
    }
    if($num_edges_op == 'gt') {
      $where_out .= " num_edges > %d AND ";
    }
    if($num_edges_op == 'eq') {
      $where_out .= " num_edges = %d AND ";
    }
    $out_args[] = $num_edges;
  }

  // clean up the where clause
  if(strlen($where) > 0) {
    $where = substr($where, 0, -4); # remove trailing 'AND'
    $where = "WHERE $where";
  }
  if(strlen($where_out) > 0) {
    $where_out = substr($where_out, 0, -4); # remove trailing 'AND'
    $where_out = "WHERE $where_out";
  }
  $orderby = '';
  if ($order == "Num Nodes") {
    $orderby = "ORDER BY num_nodes $sort";
  }
  elseif ($order == "Num Edges") {
    $orderby = "ORDER BY num_edges $sort";
  }
  elseif ($order == "&lt;<i>k</i>&gt;") {
    $orderby = "ORDER BY avg_degree $sort";
  }
  else {
    $orderby = "ORDER BY genus, species, network_name, module_name";
  }

  // build the final SQL
  $num_per_page = 25;
  $sql = "SELECT *, (cast(num_edges as float8) * 2) / cast(num_nodes as float8) avg_degree
          FROM ($select $from $where $group) AS t1 $where_out $orderby";
  $count_sql =  "SELECT count(*) FROM ($sql) as t2";
  $args = array_merge($args, $out_args);

  // execute the query
  $previous = tripal_db_set_active('chado');
//  dpm(array($sql, $args));
  $results = pager_query($sql, $num_per_page, 0, $count_sql, $args);
  $counts = db_fetch_object(db_query($count_sql, $args));
  tripal_db_set_active($previous);


  $rows = array();
  while($module = db_fetch_object($results)) {
    // build the network name w/ link if available.
    $network_name = $module->network_name;
    if ($module->network_nid) {
      $network_name = "<a href=\"" . url("node/" . $module->network_nid) . "\" target=\"_blank\">" . $module->network_name . "</a>";
    }

    // build the module name w/ link if available
    $module_name = $module->module_name;
    if ($module->network_nid) {
      $vars = '';
      if (count($traits) > 0) {
         foreach ($traits as $trait_dbxref_id) {
           $vars .= "&traits=$trait_dbxref_id";
         }
      }
      if ($overlaps) {
        $vars .= "&overlaps=$overlaps";
      }
      if (count($types) > 0) {
        foreach ($types as $feature_type) {
          $vars .= "&types=$feature_type";
        }
      }
      $module_name = "<a href=\"" . url("node/" . $module->network_nid) . "?block=explorer&module_id=" . $module->networkmod_id . "$vars\" target=\"_blank\">" . $module->module_name . "</a>";
    }

    // calculate average degree
    $avg_degree = 'N/A';
    if ($module->num_nodes > 0) {
      $avg_degree = number_format($module->avg_degree, 2);
    }

    // format the list of matching nodes (remove duplicates);
    $matching_features  = 'N/A';
    if($module->matching_features){
      $matching_features = preg_replace("/[{}]/",'', $module->matching_features);
      $matches = preg_split('/,/', $matching_features);
      $matching_features = '<div id="networkmod-search-match-list">' . implode(', ', array_unique($matches)) . '</div>';
    }

    // format the list of matching terms (remove duplicates);
    $matching_terms  = 'N/A';
    if($module->matching_terms){
      $matching_terms = preg_replace("/[{}]/", '', $module->matching_terms);
      $matches = preg_split('/(^"|","|"$)/', $matching_terms);
      $matching_terms = '<div id="networkmod-search-match-list"><ul><li>' . implode('</li><li>', array_unique($matches)) . '</li></ul></div>';
    }

    // format the list of matching traits, count the instances of each
    // trait and then (remove duplicates):
    $matching_traits  = 'N/A';
    if($module->matching_traits){
      $mtraits = preg_replace("/[{}]/", '', $module->matching_traits);
      $matches = array_unique(preg_split('/(^"|","|"$)/', $mtraits));
      $trait_counts = array();
      foreach ($matches as $match) {
        if (!$match) { continue; }
        $t = preg_split('/:/', $match);
        // t[0] = genetic feature type
        // t[1] = genetic feature name
        // t[2] = trait name
        if (!array_key_exists($t[2], $trait_counts)) {
          $trait_counts[$t[2]][$t[0]] = 0;
        }
        $trait_counts[$t[2]][$t[0]]++;
      }

      // if overlaps is set then throw out matches where the trait count
      // is less than the overlaps.  If none of the traits match then
      // don't include this row
      $matching_traits = '';
      foreach ($trait_counts as $tname => $ttypes) {
        foreach ($ttypes as $ttype => $num) {
          // if we are filtering on genetic feature type then ignore
          // those types that aren't in the filter
          if (count($types) > 0 and !array_key_exists($ttype, $types)) {
            continue;
          }
          if ($num >= $overlaps) {
            $matching_traits .=  "<li>$ttype: $tname ($num)</li>";
          }
        }
      }
      if ($matching_traits) {
        $matching_traits = "<div id=\"networkmod-search-match-list\"><ul>$matching_traits</ul></div>";
      }
    }

    // put the fields togther for the row of the table
    $rows[] = array(
      $network_name,
      $module_name,
      "<i>" . $module->genus . " " . $module->species . "</i>",
      $module->num_nodes,
      $module->num_edges,
      $avg_degree,
      //$matching_features,
      $matching_terms,
      $matching_traits,
    );
  }

  if (count($rows) > 0) {
    $_SESSION['network_search_criteria']['has_results'] = TRUE;
  }
  else {
    $_SESSION['network_search_criteria']['has_results'] = FALSE;
    $rows[] = array(
      array(
        'data' => 'No results found that match the provided criteria',
        'colspan' => 9,
        'align' => 'center'
      ),
    );
  }

  $header = array(
    "Network",
    array(
      'data' => 'Module',
      'field' => 'module_name',
    ),
    "Species",
    array(
      'data' => 'Num Nodes',
      'field' => 'num_nodes',
    ),
    array(
      'data' => 'Num Edges',
      'field' => 'num_edges',
    ),
    array(
      'data' => "&lt;<i>k</i>&gt;",
      'field' => 'avg_degree',
    ),
    //'Nodes With Matched Annotation',
    'Matched Terms',
    'Matched Traits<sup>&dagger;</sup>',
  );
  $pager = theme_pager();
  $output = "Found <b>" . number_format($counts->count) . "</b> matching records.  Click column headers to sort.";
  $output .= $pager;
  $output .= theme('table', $header, $rows);
  $output .= $pager;
  $output .= "<sup><b>&dagger;</b></sup> The number in parentheses is the number of overlapping genetic features for the given trait and marker type. ";
  return $output;
}
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// SOMETHING IS WRONG. THE FOLLOWING CODE IS PART OF THE
// tripal_core_ahah.inc file but for some reason it is not
// being included.  So a temporary fix was to put these
// functions here.
/*
 * @file
 * Part of the Tripal Core API.
 *
 * Provides support for dynamic forms through AHAH
 */

/*
 * This function simply gets the posted form ID, builds the form
 * and retrieves the specified element
 */
function tripal_core_ahah_prepare_form() {

  // Retrieve the form from the cache
  $form_state = array('storage' => NULL);
  $form_build_id = filter_xss($_POST['form_build_id']);
  $form = form_get_cache($form_build_id, $form_state);

  // Preparing to process the form
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;

  // we don't want to submit the form or have required fields validated on
  // an ahah callback.
  $form_state['submitted'] = TRUE;
  $form['#validate'] = NULL;
  $form['#submit'] = NULL;
  $form_state['submit_handlers'] = NULL;
  $form_state['validate_handlers'] = NULL;
  tripal_core_ahah_form_element_disable_validation($form);

  // Sets the form_state so that the validate and submit handlers can tell
  // when the form is submitted via AHAH
  $form_state['ahah_submission'] = TRUE;

  // Process the form with drupal_process_form. This function calls the submit
  // handlers, which put whatever was worthy of keeping into $form_state.
  drupal_process_form($form_id, $form, $form_state);

  // You call drupal_rebuild_form which destroys $_POST.
  // The form generator function is called and creates the form again but since
  // it knows to use $form_state, the form will be different.
  // The new form gets cached and processed again, but because $_POST is
  // destroyed, the submit handlers will not be called again.
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);


  return $form;
}

/*
 *
 */
function tripal_core_ahah_bind_events() {

  // Get the JS settings so we can merge them.
  $javascript = drupal_add_js(NULL, NULL, 'header');
  $settings = call_user_func_array('array_merge_recursive', $javascript['setting']);

  return array('ahah' => $settings['ahah']);
}

/*
 *
 */
function tripal_core_ahah_form_element_disable_validation(&$form) {
  // --START code borrowed from ahah_helper module
  foreach (element_children($form) as $child) {
    $form[$child]['#validated'] = TRUE;
    tripal_core_ahah_form_element_disable_validation($form[$child]);
  }
  // --END code borrowed from ahah_helper module
}

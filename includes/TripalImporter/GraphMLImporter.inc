<?php

class GraphMLImporter extends TripalImporter {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Chado GraphML Loader';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'gml_loader';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Import a biological network from a file in GraphML format.';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['gml', 'graphml'];


  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'Please provide the GraphML file';

  /**
   * The title that should appear above the upload button.
   */
  public static $upload_title = 'GraphML File';

  /**
   * Text that should appear on the button at the bottom of the importer
   * form.
   */
  public static $button_text = 'Import GraphML file';

  /**
   * This is a multi-step form so we will overide the default form.
   */
  public static $methods = [
    // Allow the user to upload a file to the server.
    'file_upload' => FALSE,
    // Allow the user to provide the path on the Tripal server for the file.
    'file_local' => FALSE,
    // Allow the user to provide a remote URL for the file.
    'file_remote' => FALSE,
  ];

  /**
   * We need the file to not be required so there is no validation error on
   * non-file containing form pages.
   */
  public static $file_required = FALSE;

  /**
   * If the loader should require an analysis record.  To maintain provenance
   * we should always indiate where the data we are uploading comes from.
   * The method that Tripal attempts to use for this by associating upload files
   * with an analysis record.  The analysis record provides the details for
   * how the file was created or obtained. Set this to FALSE if the loader
   * should not require an analysis when loading. if $use_analysis is set to
   * true then the form values will have an 'analysis_id' key in the $form_state
   * array on submitted forms.
   */
  public static $use_analysis = FALSE;

  /**
   * If the $use_analysis value is set above then this value indicates if the
   * analysis should be required.
   */
  public static $require_analysis = FALSE;

  /**
   * We remove the button in order to control the label on it.
   */
  public static $use_button = FALSE;

  /**
   * A handle to a temporary file for caching the GFF features. This allows for
   * quick lookup of parsed features without having to store it in RAM.
   */
  private $cache_file = NULL;

  /**
   * The name of the temporary cache file.
   */
  private $cache_file_name = NULL;


  /**
   * The graphML file.
   */
  private $graphML_file = NULL;


  /**
   * The graphML XML reader object.
   */
  private $graphml = NULL;

  /**
   * The graph details array.
   */
  private $graph = NULL;

  /**
   * The organism ChadoRecord object that corresponds to the $organism_id value.
   */
  private $organism = NULL;

  /**
   * The organism ID that the network belongs to.
   */
  private $organism_id = NULL;

  /**
   * The analysis ID for this GFF file
   */
  private $analysis_id = NULL;

  /**
   * The network type id.
   */
  private $network_type_id = NULL;


  /**
   * Indicates if the node names match on feature.name or feature.uniquename
   */
  private $match_type = '';

  /**
   * An associative array to hold attribute keys from the GraphML header
   * section.
   */
  private $attr_keys = [];


  /**
   * Holds the nubmer of elements in the graphML file.
   */
  private $num_elements = 0;

  /**
   * Holds the list of nodes that need to be added.
   */
  private $nodes = [];


  /**
   * Maps the node IDs from the graphML to the feature names
   */
  private $nodeMapping = [];

  /**
   * The CVTerm ID for the node type.
   */
  private $node_type_id = '';

  /**
   * Holds the list of edges that need to be added
   */
  private $edges = [];

  /**
   * Holds the ChadoRecord for the network.
   */
  private $network = NULL;


  /**
   * Set this to TRUE to only parse the header and graph info. This is only
   * set when the user is running the form for loading the file.
   */
  private $quick_scan = FALSE;


  /**
   * The list of controlled vocabulary terms used for the attributes
   * in the GraphML file.
   */
  private $terms = [];

  /**
   * See TripalImporter::form)
   */
  public function form($form, &$form_state) {

    if (array_key_exists('triggering_element', $form_state) and
        $form_state['triggering_element']['#type'] == 'submit' and
        ($form_state['triggering_element']['#value'] == 'Next Step' or
         $form_state['triggering_element']['#value'] == 'Previous Step')) {
      $form_state['storage']['current_step'] = $form_state['triggering_element']['#name'];
    }

    $current_step = 'step1';
    if (array_key_exists('storage', $form_state)) {
      $current_step = $form_state['storage']['current_step'];
    }

    // Based on the current stage, return the correct form.
    switch ($current_step) {
      case 'step1':
        $form = $this->formStep1($form, $form_state);
        break;
      case 'step2':
        $form = $this->formStep2($form, $form_state);
        break;
      case 'step3':
        $form = $this->formStep3($form, $form_state);
        break;
    }

    return $form;
  }

  /**
   * Step1 form elements.
   */
  private function formStep1($form, &$form_state) {
    global $user;

    $class = get_called_class();

    // Now to add the file + description based on our class.
    $form['file'] = [
      '#type' => 'fieldset',
      '#title' => t($class::$upload_title),
      '#description' => t($class::$upload_description),
    ];

    $existing_files = tripal_get_user_uploads($user->uid, $class::$file_types);
    $fids = [];
    if (count($existing_files) > 0) {
      $fids = [0 => '--Select a file--'];
      foreach ($existing_files as $fid => $file) {
        $fids[$fid] = $file->filename . ' (' . tripal_format_bytes($file->filesize) . ') ';
      }
    }
    $form['file']['file_upload_existing'] = [
      '#type' => 'select',
      '#title' => t('Existing Files'),
      '#description' => t('You may select a file that is already uploaded.'),
      '#options' => $fids,
    ];

    $form['file']['file_upload'] = [
      '#type' => 'html5_file',
      '#title' => '',
      '#description' => 'Remember to click the "Upload" button below to send ' .
      'your file to the server.  This interface is capable of uploading very ' .
      'large files.  If you are disconnected you can return, reload the file and it ' .
      'will resume where it left off.  Once the file is uploaded the "Upload ' .
      'Progress" will indicate "Complete".  If the file is already present on the server ' .
      'then the status will quickly update to "Complete".',
      '#usage_type' => 'tripal_importer',
      '#usage_id' => 0,
      '#allowed_types' => $class::$file_types,
      '#cardinality' => $class::$cardinality,
    ];

    $form['file']['file_local'] = [
      '#title' => t('Server path'),
      '#type' => 'textfield',
      '#maxlength' => 5120,
      '#description' => t('If the file is local to the Tripal server please provide the full path here.'),
    ];

    // get the list of organisms
    $sql = "SELECT * FROM {organism} ORDER BY genus, species";
    $org_rset = chado_query($sql);
    $organisms = [];
    $organisms[''] = '';
    while ($organism = $org_rset->fetchObject()) {
      $organisms[$organism->organism_id] = "$organism->genus $organism->species ($organism->common_name)";
    }
    $form['organism_id'] = [
      '#title' => t('Organism'),
      '#type' => 'select',
      '#description' => t("Choose the organism to which this network belongs."),
      '#required' => TRUE,
      '#options' => $organisms,
    ];

    // get the list of analyses
    $sql = "SELECT * FROM {analysis} ORDER BY name";
    $org_rset = chado_query($sql);
    $analyses = [];
    $analyses[''] = '';
    while ($analysis = $org_rset->fetchObject()) {
      $analyses[$analysis->analysis_id] = "$analysis->name ($analysis->program $analysis->programversion, $analysis->sourcename)";
    }
    $form['analysis_id'] = [
      '#title' => t('Analysis'),
      '#type' => t('select'),
      '#description' => t('Choose the analysis to which the uploaded data will be associated. ' .
          'Why specify an analysis for a data load?  All data comes from some place, even if ' .
          'downloaded from a website. By specifying analysis details for all data imports it ' .
          'provides provenance and helps end user to reproduce the data set if needed. At ' .
          'a minimum it indicates the source of the data.'),
      '#required' => TRUE,
      '#options' => $analyses,
    ];

    $wgcna = chado_get_cvterm(['id' => 'operation:3766']);
    $ppi = chado_get_cvterm(['id' => 'operation:3094']);
    $gcn = chado_get_cvterm(['id' => 'local:GCN']);
    $csgcn = chado_get_cvterm(['id' => 'local:csGCN']);
    $mcn = chado_get_cvterm(['id' => 'local:MCN']);
    $grn = chado_get_cvterm(['id' => 'operation:2437']);

    $form['network_type'] = [
      '#title' => t('Network Analysis Type'),
      '#type' => t('select'),
      '#description' => t("Choose type of network described in the GraphML file."),
      '#required' => TRUE,
      '#options' => [
        '' => '',
        $csgcn->cvterm_id => $csgcn->name . ' (csGCN)',
        $gcn->cvterm_id => $gcn->name . ' (GCN)',
        $grn->cvterm_id => $grn->name . ' (GRN)',
        $mcn->cvterm_id => $mcn->name . ' (MCN)',
        $ppi->cvterm_id => $ppi->name . ' (PPI)',
        $wgcna->cvterm_id => $wgcna->name . ' (WGCNA)'
      ],
    ];


    // get the sequence ontology CV ID
    $values = ['name' => 'sequence'];
    $cv = chado_select_record('cv', ['cv_id'], $values);
    $cv_id = $cv[0]->cv_id;

    $form['node_type'] = [
      '#title' => t('Default Node Type'),
      '#required' => TRUE,
      '#type' => 'textfield',
      '#description' => t("The network nodes must be genomic features that are already in the database.
        But, the loader needs to know what type they are (e.g. mRNA, protein, etc.) Please specify the type.
        The value must be a term in the Sequence Ontololgy (SO). This is only used if the loader cannot
        determine the node type from the GraphML file itself"),
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/$cv_id",
    ];
    $form['match_type'] = [
      '#type' => 'radios',
      '#title' => 'Name Match Type',
      '#required' => TRUE,
      '#options' => [
        t('Name'),
        t('Unique name'),
      ],
      '#description' => t('Feature data is stored in Chado with both a human-readable
        name and a unique name. If the nodes in the GraphML file are uniquely identified using
        a human readable name then select the "Name" button. If the nodes are
        uniquely identified using the feature unique names then select the "Unique name" button.'),
      '#default_value' => 1,
    ];

    $form['next_step'] = array(
      '#type' => 'submit',
      '#name' => 'step2',
      '#value' => 'Next Step',
      '#weight' => 100,
    );
    return $form;
  }

  /**
   * Retreive files.
   *
   * Because we override the TripalImporter file management we must get the
   * file ourselves.
   */
  private function extractGraphMLpertinents($form_state) {

    $file_local = NULL;
    $file_upload = NULL;
    $file_remote = NULL;

    // Get the form values for the file.
    $file_local = trim($form_state['storage']['file_local']);
    $file_upload = trim($form_state['storage']['file_upload']);
    $file_existing = trim($form_state['storage']['file_upload_existing']);

    if ($file_existing) {
      $fid = $file_existing;
      $file = file_load($fid);
      $this->graphML_file = drupal_realpath($file->uri);
    }
    elseif ($file_local) {
      $this->graphML_file = $file_local;
    }
    elseif ($file_upload) {
      $fid = $file_upload;
      $file = file_load($fid);
      $this->graphML_file = drupal_realpath($file->uri);
    }

    // Open the GraphML file for a quick look at the header keys.
    $this->graphml = new XMLReader();
    $this->graphml->open($this->graphML_file);
    $this->quick_scan = TRUE;
    $this->parseGraphML();
    $this->graphml->close();

  }

  /**
   *
   */
  private function formStep2($form, &$form_state) {

    $this->extractGraphMLpertinents($form_state);

    $ranking_terms = theme_item_list([
      'items' => [
        'Similarity Score (SWO:0000425)',
        'Importance Weight (NCIT:C48192)',
        'Rank (NCIT:C48904)'
      ],
      'title' => NULL,
      'type' => 'ul',
      'attributes' => [],
    ]);

    $layout_terms = theme_item_list([
      'items' => [
        'x cartesian coordinate (SIO:000400)',
        'y cartesian coordinate (SIO:000401)'
      ],
      'title' => NULL,
      'type' => 'ul',
      'attributes' => [],
    ]);

    $form['gml_attributes'] = [
      '#type' => 'item',
      '#title' => 'Instructions',
      '#markup' => t('All attributes, that you want loaded, for the graph, nodes and edges in the
         GraphML file must be associated with a controlled
         vocabulary term. Use the form below to make the assocaition with
         attributes that were found in the file provided.
         If an appropriate term is not present you can add one
         !here.  You can find terms at the !EBI. If you cannot
         find an appropriate term, you can add one to the "local" vocabulary.
         However, best practice is to use existing terms if possible.
         If you leave any attribute without an association, then the property
         will be excluded and will not be visible to end-users.',
         [
           '!here' => l('here', 'http://localhost/admin/tripal/loaders/chado_vocabs/chado_cv/cvterm/add', ['attributes' => ['target' => '_blank']]),
           '!EBI' => l('EBI Ontology Lookup Service' ,'https://www.ebi.ac.uk/ols/index', ['attributes' => ['target' => '_blank']])
         ]) .
         '<p><b>Visualizing the Network</b><br>' .
         t('This loader does not create a 2D layout for the nodes in the network viewer.
            This x and y positions must be provided as node attributes in the GraphML file. Please use the
            following terms to indicate which node attributes below
            provide the x and y coordinates. If these terms are not used or the attributes are not in
            the GraphML file then node layout will be random. ') . $layout_terms . '</p>' .
            '<p><b>Ranking Edges in the Network</br></b>' .
         t('The netework viewer works best with at least one term that end-users can use for ranking edges. The following
            are examples of possible terms that could be used for ranking: ') . $ranking_terms . '</p>',
      '#weight' => -100,
    ];

    // For Ajax triggered form elements we don't want the fieldset to close
    // so we need to know the delta value of the triggering parent to
    // keep the fieldset open.
    $triggered_parent = NULL;
    if ($form_state['triggering_element']['#type'] == 'button' and
        $form_state['triggering_element']['#value'] == 'Lookup Term') {
      $triggered_parent = $form_state['triggering_element']['#parents'][0];
      $triggered_parent = preg_replace('/select_button/', '', $triggered_parent);
    }

    // Get the list of terms that have known default mappings.
    $default_terms = variable_get('tripal_network_graphml_mapper');

    // Iterate through the GraphML attributes and add a form element for each.
    $delta = 0;
    foreach ($this->attr_keys as $key => $attr_info) {

      $default_term_name = '';
      $default_data_type = 0;
      $default_missing = '';

      // Get the value from the form state
      if (array_key_exists('values', $form_state) and array_key_exists('data_class' . $delta, $form_state['values'])) {
        $default_data_type = $form_state['values']['term_match' . $delta]['data_class' . $delta];
        $default_missing = $form_state['values']['term_match' . $delta]['missing' . $delta];
      }
      // Get the default value for this attribute if the the form is being
      // reloaded due to an error.
      else if ($form_state['triggering_element']['#type'] == 'submit' and
          $form_state['triggering_element']['#name'] == 'step3' and
          $form_state['triggering_element']['#value'] == 'Next Step') {
        // Do nothing, we dcan't get the $form_state['input'] from the API.
      }
      // Get the default value for this attribute if the form is being
      // reloaded by returning from step 3.
      elseif ($form_state['triggering_element']['#type'] == 'submit' and
              $form_state['triggering_element']['#name'] == 'step2' and
              $form_state['triggering_element']['#value'] == 'Previous Step') {
        if (array_key_exists('graphml_key.' . $key . '.id', $form_state['storage'])) {
          $default_missing = $form_state['storage']['graphml_key.' . $key . '.missing'];
          $default_data_type = $form_state['storage']['graphml_key.' . $key . '.class'];
          $default_term_id = $form_state['storage']['graphml_key.' . $key . '.id'];
          $default_term = chado_get_cvterm(['cvterm_id' => $default_term_id]);
          $default_term_name = $default_term->name;
        }
      }
      // Get the default value for the attribute if this is being reloaded
      // from an AJAX submit for the lookup form.
      elseif ($form_state['triggering_element']['#type'] == 'button' and
              $form_state['triggering_element']['#value'] == 'Lookup Term') {
        // Do nothing, let the API handle it.
      }
      // If we can't set the default value from a form resubmit then
      // we'll try the global drupal variable that has some defaults.
      else {
        if (array_key_exists($key, $default_terms)) {
          $default_data_type = $default_terms[$key]['type'];
          $term_id = $default_terms[$key]['term'];
          $term = chado_get_cvterm(['id' => $term_id]);
          if ($term) {
            $default_term_name = $term->name;
          }
        }
      }

      // Build a title for the field set.
      $title = $attr_info['for'] . ' ' . $key;

      // Add the lookup fieldset to the form.
      tripal_get_term_lookup_form($form, $form_state, $default_term_name,
          $title, '', FALSE,  '', $delta,
          'tripal_network_graphml_form_ajax_callback');

      $form['term_match' . $delta]['data_class' . $delta] = [
        '#type' => 'select',
        '#options' => [
          0 => 'Use defaults',
          'quantitative' => 'Numerical',
          'ordinal' => 'Numeric ordinal (coded categorical)',
          'annotation' => 'Text: annotation text (not filterable)',
          'categorical' => 'Text: categorical variable name',
        ],
        '#description' => t('When viewing network relationships, the end-user
           can perform layering and filtering of nodes and edges using the
           attributes assigned to them. To support filtering, we need to know the type of
           data for each attribute because elements with numerical or categorical
           attributes can be filtered differently.  The GraphML loader will
           try to guess the data type, however, you can can specify the value
           here to prevent the loader from guessing incorrectly or to refine
           the type.  For example, the loader does not know if a numerical
           attribute is also ordinal or if a text value is categorical. Text
           attributes that are not specified as categorical will not be
           filterable.'),
        '#default_value' => $default_data_type,
      ];
      $form['term_match' . $delta]['missing' . $delta] = [
        '#type' => 'textfield',
        '#description' => t('If this attribute contains missing values please indicate
           the text for a missing value. Attributes for nodes or edges
           with a missing value will be excluded.  This loader automatically
           recognizes the following as indicators of a missing value:  NA, NaN, null, or None.
           Override this default by setting your own. You can specify multiple values by
           separating them with the bar ("|") character:  e.g., NA|NaN|null|None.'),
        '#default_value' => $default_missing,
      ];

      // Make the form element collapsed.
      $form['term_match' . $delta]['#collapsible'] = TRUE;
      if ($triggered_parent !== "$delta") {
        $form['term_match' . $delta]['#collapsed'] = TRUE;
      }
      $delta++;
    }

    $form['next_step'] = array(
      '#type' => 'submit',
      '#name' => 'step3',
      '#value' => 'Next Step',
      '#weight' => 100,
    );
    return $form;

  }

  /**
   *
   */
  private function formStep3($form, &$form_state) {

    // First get the mapping of CVterms to each attribute.
    $this->extractGraphMLpertinents($form_state);
    $attr_terms = [];
    foreach ($this->attr_keys as $key => $attr_info) {
      if (array_key_exists('graphml_key.' . $key . '.id', $form_state['storage'])) {
        $cvterm_id = $form_state['storage']['graphml_key.' . $key . '.id'];
        $for = $attr_info['for'];
        $attr_terms[$for][$cvterm_id] = $key;
      }
    }

    // If an attribute uses the schema:name for the graph then use
    // that for the name.
    $graph_name = $this->graph['id'];
    $namet = chado_get_cvterm(['id' => 'schema:name']);
    if (array_key_exists($namet->cvterm_id, $attr_terms['graph'])){
      $graph_name = $this->graph[$attr_terms['graph'][$namet->cvterm_id]];
      $graph_uname = $graph_name;
    }

    $graph_uname = $this->graph['id'];
    $unamet = chado_get_cvterm(['id' => 'data:0842']);
    if (array_key_exists($unamet->cvterm_id, $attr_terms['graph'])){
      $graph_uname = $this->graph[$attr_terms['graph'][$unamet->cvterm_id]];
      if (!$graph_name or $graph_name == 'G') {
        $graph_name = $graph_uname;
      }
    }


    $form['graph_name'] = [
      '#title' => t('Graph Name'),
      '#required' => TRUE,
      '#type' => 'textfield',
      '#default_value' => $graph_name,
      '#description' => t("The human readable name for this graph. This name was extracted from the GraphML file provided."),
    ];
    $form['graph_uname'] = [
      '#title' => t('Graph Unique Name'),
      '#required' => TRUE,
      '#type' => 'textfield',
      '#maxlength' => 255,
      '#default_value' => $graph_uname,
      '#description' => t("A unique name name for this graph. It need not be human
         readable, but it must be unique from any other existing graph. This name was extracted from the GraphML file provided."),
    ];

    // We need to let the user know how the load will perform given the
    // inputs provided. First get the list of required terms.
    $xcoord = chado_get_cvterm(['id' => 'SIO:000400']);
    if (!array_key_exists($xcoord->cvterm_id, $attr_terms['node'])) {
      drupal_set_message('The GraphML file provided needs a node attribute
        assigned the term "x cartesian coordinate" (SIO:000400). This term
        indicates that the attribute provides the x-axis coordinate for
        the node for the network visualization. If you do
        not have an attribute with this term then the node will be randomly
        positioned in the graphical viewers.',  'warning');
    }

    $ycoord = chado_get_cvterm(['id' => 'SIO:000401']);
    if (!array_key_exists($ycoord->cvterm_id, $attr_terms['node'])) {
      drupal_set_message('The GraphML file provided needs a node attribute
        assigned the term "y cartesian coordinate" (SIO:000401). This termy
        indicates that the attribute provides the x-axis coordinate for
        the node for the network visualization. If you do
        not have an attribute with this term then the node will be randomly
        positioned in the graphical viewers.',  'warning');
    }

    $simscore = chado_get_cvterm(['id' => 'SWO:0000425']);
    if (!array_key_exists($simscore->cvterm_id, $attr_terms['edge'])) {

    }


    $form['prev_step'] = array(
      '#type' => 'submit',
      '#name' => 'step2',
      '#value' => 'Previous Step',
      '#weight' => 100,
    );
    $form['next_step'] = array(
      '#type' => 'submit',
      '#name' => 'done',
      '#value' => 'Submit',
      '#weight' => 100,
    );
    return $form;
  }

  /**
   * See TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state) {
    $forward = TRUE;

    // If the submit was clicked, iterate the stage.
    $next_step = 'step1';
    if ($form_state['triggering_element']['#type'] == 'submit') {
      $next_step = $form_state['triggering_element']['#name'];
    }

    if (array_key_exists('triggering_element', $form_state) and
        $form_state['triggering_element']['#type'] == 'submit') {
      if ($form_state['triggering_element']['#value'] == 'Previous Step') {
        $forward = FALSE;
      }
      $next_step = $form_state['triggering_element']['#name'];
    }

    if ($next_step == 'step2') {
      $form_state['rebuild'] = TRUE;
      if ($forward) {
        $this->validateStep1($form, $form_state);
      }
    }
    if ($next_step == 'step3') {
      $form_state['rebuild'] = TRUE;
      if ($forward) {
        $this->validateStep2($form, $form_state);
      }
    }
    if ($next_step == 'done') {
      $this->validateStep3($form, $form_state);
    }
  }

  /**
   *
   */
  private function validateStep1($form, &$form_state) {

    $file_local = NULL;
    $file_upload = NULL;
    $file_existing = NULL;
    $organism_id = $form_state['values']['organism_id'];
    $network_type = $form_state['values']['network_type'];
    $analysis_id = $form_state['values']['analysis_id'];
    $node_type = $form_state['values']['node_type'];
    $match_type = $form_state['values']['match_type'];

    // Get the form values for the file.
    $file_local = trim($form_state['values']['file_local']);
    if ($file_local) {
      // check to see if the file is located local to Drupal
      $file_local = trim($file_local);
      $dfile = $_SERVER['DOCUMENT_ROOT'] . base_path() . $file_local;
      if (!file_exists($dfile)) {
        // if not local to Drupal, the file must be someplace else, just use
        // the full path provided
        $dfile = $file_local;
      }
      if (!file_exists($dfile)) {
        form_set_error('file_local', t("Cannot find the file on the system. Check that the file exists or that the web server has permissions to read the file."));
      }
    }

    $file_upload = trim($form_state['values']['file_upload']);
    $file_existing = trim($form_state['values']['file_upload_existing']);

    // The user must provide at least an uploaded file or a local file path.
    if (!$file_upload and !$file_local and !$file_existing) {
      form_set_error('file_local', t("You must provide a file."));
    }

    if (!$network_type) {
      form_set_error('network_type', t("You must provide a network type."));
    }

    // Put the values from this step of the form into storage.
    $form_state['storage']['file_local'] = $file_local;
    $form_state['storage']['file_upload'] = $file_upload;
    $form_state['storage']['file_upload_existing'] = $file_existing;
    $form_state['storage']['organism_id'] = $organism_id;
    $form_state['storage']['analysis_id'] = $analysis_id;
    $form_state['storage']['network_type'] = $network_type;
    $form_state['storage']['node_type'] = $node_type;
    $form_state['storage']['match_type'] = $match_type;
  }

  /**
   *
   */
  private function validateStep2($form, &$form_state) {
    $this->extractGraphMLpertinents($form_state);
    $delta = 0;
    foreach ($this->attr_keys as $key => $attr_info) {
      $selected_term = tripal_get_term_lookup_form_result($form, $form_state,'',$delta);
      if ($selected_term) {
        $cvterm_id = $selected_term[0]->cvterm_id;
        $form_state['storage']['graphml_key.' . $key . '.id'] = $cvterm_id;
      }
      else {
        $form_state['storage']['graphml_key.' . $key . '.id'] = NULL;
      }
      $form_state['storage']['graphml_key.' . $key . '.class'] = $form_state['values']['data_class' . $delta];
      $form_state['storage']['graphml_key.' . $key . '.missing'] = $form_state['values']['missing' . $delta];
      $delta++;
    }
  }
  /**
   *
   */
  private function validateStep3($form, &$form_state) {

    // Put all of the values in storage back into the values array for
    // job submission.
    $this->extractGraphMLpertinents($form_state);
    $delta = 0;
    foreach ($this->attr_keys as $key => $attr_info) {
      if (array_key_exists('graphml_key.' . $key . '.id', $form_state['storage'])) {
        $form_state['values']['graphml_key.' . $key . '.id'] = $form_state['storage']['graphml_key.' . $key . '.id'];
      }
      if (array_key_exists('graphml_key.' . $key . '.class', $form_state['storage'])) {
        $form_state['values']['graphml_key.' . $key . '.class'] = $form_state['storage']['graphml_key.' . $key . '.class'];
      }
      if (array_key_exists('graphml_key.' . $key . '.missing', $form_state['storage'])) {
        $form_state['values']['graphml_key.' . $key . '.missing'] = $form_state['storage']['graphml_key.' . $key . '.missing'];
      }
      $delta++;
    }
    $form_state['values']['graphml_file_local'] = $form_state['storage']['file_local'];
    $form_state['values']['graphml_file_upload'] = $form_state['storage']['file_upload'];
    $form_state['values']['graphml_file_upload_existing'] = $form_state['storage']['file_upload_existing'];
    $form_state['values']['organism_id'] = $form_state['storage']['organism_id'];
    $form_state['values']['analysis_id'] = $form_state['storage']['analysis_id'];
    $form_state['values']['network_type'] = $form_state['storage']['network_type'];
    $form_state['values']['node_type'] = $form_state['storage']['node_type'];
    $form_state['values']['match_type'] = $form_state['storage']['match_type'];
  }

  /**
   * Returns the path of the input file.
   *
   * @param $arguments
   *   The importer input arguments.
   */
  private function getFilePath($arguments) {

    if (array_key_exists('graphml_file_upload_existing', $arguments) and
        $arguments['graphml_file_upload_existing']) {
      $file = file_load($arguments['graphml_file_upload_existing']);
      return drupal_realpath($file->uri);
    }
    if (array_key_exists('grapml_file_upload', $arguments) and
        $arguments['grapml_file_upload']) {
      $file = file_load($arguments['grapml_file_upload']);
      return  drupal_realpath($file->uri);
    }
    if (array_key_exists('graphml_file_local', $arguments) and
        $arguments['graphml_file_local']) {
      return $arguments['graphml_file_local'];
    }
  }

  /**
   * Sets the terms array for the attributes terms.
   */
  private function setTerms(){
    $options = [
      'include_fk' => [
        'cv_id' => TRUE,
        'dbxref_id' => [
          'db_id' => TRUE
        ],
      ],
    ];

    foreach ($this->arguments['run_args'] as $key => $value) {
      $matches = [];
      if (preg_match('/^graphml_key\.(.*)\.id$/', $key, $matches)) {
        $term = chado_get_cvterm(['cvterm_id' => $value], $options);
        $this->terms[$matches[1]] = $term;

        // Store the  term with it's full accession so we can look
        // it up either way.
        $term_name = $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession;
        $this->terms[$term_name] = $term;
      }
    }

    // Add some terms added by this loader
    $term = chado_get_cvterm(['id' => 'local:node_count']);
    $term_name = $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession;
    $this->terms[$term_name] = $term;

    $term = chado_get_cvterm(['id' => 'local:edge_count']);
    $term_name = $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession;
    $this->terms[$term_name] = $term;
  }

  /**
   * Swaps the keys of an array for  the CV terms.
   */
  private function swapTerms($item) {
    $new_item = [];
    foreach (array_keys($item) as $key) {
      $term = $this->terms[$key];
      if ($term) {
        $new_key = $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession;
        $new_item[$new_key] = $item[$key];
      }
      else {
        $new_item[$key] = $item[$key];
      }
    }
    return $new_item;
  }

  /**
   * see TripalImporter::run()
   */
  public function run() {
    $arguments = $this->arguments['run_args'];
    $file_path = $this->getFilePath($arguments);

    $this->graphML_file = $file_path;
    $this->organism_id = $arguments['organism_id'];
    $this->analysis_id = $arguments['analysis_id'];
    $this->network_type_id = $arguments['network_type'];

    $type_term = chado_get_cvterm(['cv_id' => ['name' => "sequence"], 'name' => $arguments['node_type']]);
    $this->node_type_id = $type_term->cvterm_id;

    $this->match_type = 'name';
    if ($arguments['match_type'] == 1) {
      $this->match_type = 'uniquename';
    }

    // Set the CV terms used by attributes.
    $this->setTerms();

    // Get the organism object.
    $this->organism = new ChadoRecord('organism');
    $this->organism->setValues(['organism_id' => $this->organism_id]);
    $num_found = $this->organism->find();
    if ($num_found == 0) {
      throw new Exception(t("Cannot find the specified organism for this GFF3 file."));
    }

    try {
      // Create the cache file for storing parsed GFF entries.
      $this->openCacheFile();

      $this->logMessage("Step  1 of 11: Finding network size...                             ");
      $this->graphml = new XMLReader();
      $this->graphml->open($this->graphML_file);
      $this->parseNumElements();
      $this->graphml->close();
      $this->logMessage("               Found !num_elements elements                        ",
          ['!num_elements' => number_format($this->num_elements)]);

      $this->graphml = new XMLReader();
      $this->graphml->open($this->graphML_file);

      $this->logMessage("Step  2 of 11: Caching GraphML file...                             ");
      $this->parseGraphML();
      $this->graphml->close();

      $this->logMessage("Step  3 of 11: Insert network ...                                  ");
      $this->insertNetwork();

      $this->logMessage("Step  4 of 11: Find existing node features...                      ");
      $this->findFeatures();

      $this->logMessage("Step  5 of 11: Insert network nodes...                             ");
      $this->logMessage("               Adding !num_nodes nodes                             ",
          ['!num_nodes' => number_format(count($this->nodes))]);
      $this->insertNodes();

      $this->logMessage("Step  6 of 11: Find network nodes IDs...                           ");
      $this->findNodes();

      $this->logMessage("Step  7 of 11: Insert node properties...                           ");
      $this->insertNodeProps();

      $this->logMessage("Step  8 of 11: Insert network edges...                             ");
      $this->logMessage("               Adding !num_edges edges                             ",
          ['!num_edges' => number_format(count($this->edges))]);
      $this->insertEdges();

      $this->logMessage("Step  9 of 11: Find edge IDs...                                    ");
      $this->findEdges();

      $this->logMessage("Step 10 of 11: Insert edge properties...                           ");
      $this->insertEdgeProps();

      $this->logMessage("Step 11 of 11: Collecting stats...                                 ");
      $this->collectStats();

    }
    // On exception, catch the error, clean up the cache file and rethrow
    catch (Exception $e) {
      $this->closeCacheFile();
      $this->graphml->close();
      throw $e;
    }
    $this->closeCacheFile();
  }

  /**
   *
   */
  private function collectStats(){

    // Iterate through the attributes for this graph and insert information
    // about them.
    foreach ($this->attr_keys as $key => $attr_info) {
      if ($attr_info['for'] == 'edge') {
        $domain_term = chado_get_cvterm(['id' => 'NCIT:C75923']);
        if (!array_key_exists($key, $this->terms) or !$this->terms[$key]) {
          continue;
        }
        $attr_term = $this->terms[$key];
        $attr_term_name = $attr_term->dbxref_id->db_id->name . ':' . $attr_term->dbxref_id->accession;

        $edge_ids = [];
        foreach ($this->edges as $info) {
          $edge_ids[] = $info['network_edge_id'];
        }
        $props = tripal_network_get_edgeprops($edge_ids, $attr_term_name);

        $num_numeric = 0;
        $num_floats = 0;
        $num_ints = 0;
        $num_strings = 0;
        $prop_values = [];

        // Count the number of types of values.
        foreach ($props as $network_edge_id => $value) {
          $value = trim($value);

          if (is_numeric($value)) {
            $num_numeric++;
            if (preg_match('/^[01234567889]+$/', $value)) {
              $num_ints++;
            }
            else {
              $num_floats++;
            }
          }
          else {
            // Exclude missing values.
            if (preg_match('/^(NA|NaN|null|None)$/i', $value)) {
              $props[$network_edge_id] = '';
              continue;
            }
            $num_strings++;
          }
          $prop_values[$value] = 1;
        }


        // Determine if the property is quantitative.
        $data_class = 'annotation';
        $data_type = 'string';
        if ($num_strings == 0) {
          $data_class = 'quantitative';
          if ($num_ints > 0 and $num_floats == 0) {
            $data_type = 'integer';
          }
          else {
            $data_type = 'float';
          }
        }

        // Make sure the user didn't request an override in the data class.
        if (array_key_exists('graphml_key.' . $key . '.class', $this->arguments['run_args'])) {

          $data_class = $this->arguments['run_args']['graphml_key.' . $key . '.class'];
          if ($data_class == 'annotation') {
            $data_type = 'string';
          }
          else if ($data_class ==  'categorical') {
            $data_type = 'string';
          }
          else if ($data_class ==  'quantitative' and $num_strings == 0) {
            // No need to make changes.
          }
          else if ($data_class ==  'ordinal' and $num_strings == 0) {
            // No need to make changes.
          }
        }

        // Get the list of categories.
        $categories = [];
        if ($data_class == 'categorical') {
          $categories = array_keys($prop_values);
          asort($categories);
        }

        // Handle numerical data.
        $max = -INF;
        $min = INF;
        if ($data_class == 'quantitative' or $data_class == 'ordinal') {

          // Get the min and max value and take the abs value if requested.
          foreach ($props as $network_edge_id => $value) {
            if (!$value) {
              continue;
            }
            if ($value > $max) {
              $max = $value;
            }
            if ($value < $min) {
              $min = $value;
            }
          }
        }

        // Add the network attribute
        $network_attr = new ChadoRecord('network_attr');
        $network_attr->setValues([
          'network_id' => $this->network->getID(),
          'domain_id' => $domain_term->cvterm_id,
          'attr_id' => $attr_term->cvterm_id,
        ]);
        $network_attr->insert();

        // Indicate the class of variable:  Quantitative or Categorical
        if ($data_class == 'categorical') {
          $var_class = chado_get_cvterm(['id' => 'NCIT:C142657']);
        }
        if ($data_class == 'quantitative') {
          $var_class = chado_get_cvterm(['id' => 'NCIT:C142658']);
        }
        if ($data_class == 'annotation') {
          $var_class = chado_get_cvterm(['id' => 'NCIT:C44272']);
        }
        if ($data_class == 'ordinal') {
          $var_class = chado_get_cvterm(['id' => 'NCIT:C47797']);
        }

        // Get the classification term.
        $class_term = chado_get_cvterm(['id' => 'NCIT:C25161']);

        // Add the network attribute.
        $network_attrprop = new ChadoRecord('network_attrprop');
        $network_attrprop->setValues([
          'network_attr_id' => $network_attr->getID(),
          'type_id' => $class_term->cvterm_id,
          'value_id' => $var_class->cvterm_id,
          'rank' => 0,
        ]);
        $network_attrprop->insert();


        // Indicate the type of the variable
        $var_type = NULL;
        if ($data_type == 'float') {
          $var_type = chado_get_cvterm(['id' => 'NCIT:C48150']);
        }
        if ($data_type == 'integer') {
          $var_type = chado_get_cvterm(['id' => 'NCIT:C45255']);
        }
        if ($data_type == 'string') {
          $var_type = chado_get_cvterm(['id' => 'NCIT:C45253']);
        }
        $data_type_term = chado_get_cvterm(['id' => 'NCIT:C42645']);
        $network_attrprop = new ChadoRecord('network_attrprop');
        $network_attrprop->setValues([
          'network_attr_id' => $network_attr->getID(),
          'type_id' => $data_type_term->cvterm_id,
          'value_id' => $var_type->cvterm_id,
        ]);
        $network_attrprop->insert();

        // If this is categorical, save all the categories.
        $rank = 0;
        foreach ($categories as $category) {
          $catname_term = chado_get_cvterm(['id' => 'NCIT:C114464']);

          $network_attrprop = new ChadoRecord('network_attrprop');
          $network_attrprop->setValues([
            'network_attr_id' => $network_attr->getID(),
            'type_id' => $catname_term->cvterm_id,
            'value' => $category,
            'rank' => $rank,
          ]);
          $network_attrprop->insert();
          $rank++;
        }

        // If this is numerical save the min and max
        if ($is_quantitative or $is_ordinal) {
          $min_term = chado_get_cvterm(['id' => 'NCIT:C25570']);
          $network_attrprop = new ChadoRecord('network_attrprop');
          $network_attrprop->setValues([
            'network_attr_id' => $network_attr->getID(),
            'type_id' => $min_term->cvterm_id,
            'value_num' => $min,
          ]);
          $network_attrprop->insert();

          $max_term = chado_get_cvterm(['id' => 'NCIT:C25564']);
          $network_attrprop = new ChadoRecord('network_attrprop');
          $network_attrprop->setValues([
            'network_attr_id' => $network_attr->getID(),
            'type_id' => $max_term->cvterm_id,
            'value_num' => $max,
          ]);
          $network_attrprop->insert();
        }
      }
    }
  }

  /**
   * Check if the features exist in the database.
   */
  private function findFeatures() {
    $batch_size = 1000;
    $num_nodes = count(array_keys($this->nodes));
    $num_batches = (int) ($num_nodes / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $name_column = 'name';
    if ($this->match_type == 1) {
      $name_column = 'uniquename';
    }

    $sql = "
      SELECT uniquename, name, type_id, organism_id, feature_id
      FROM {feature}
      WHERE $name_column in (:names)";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $names = [];
    foreach ($this->nodes as $uniquename => $info) {
      $feature_id = $info['feature_id'];
      $total++;

      if (!$feature_id) {
        $i++;
        $names[] = $uniquename;
      }

      // If we've reached the size of the batch then let's do the select.
      if ($i == $batch_size or $total == $num_nodes) {
        if (count($names) > 0) {
          $args = [':names' => $names];
          $results = chado_query($sql, $args);
          while ($f = $results->fetchObject()) {
            if (array_key_exists($f->$name_column, $this->nodes)) {
              $matched_findex = $this->nodes[$f->$name_column]['findex'];
              $matched_node = $this->getCachedElement($matched_findex);
              if ($matched_node['type_id'] == $f->type_id and $matched_node['organism_id'] == $f->organism_id) {
                $this->nodes[$f->$name_column]['feature_id'] = $f->feature_id;
              }
            }
          }
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $i = 0;
        $names = [];
      }
    }
  }
  /**
   * Check if the features exist in the database.
   */
  private function findNodes() {
    $batch_size = 1000;
    $num_nodes = count(array_keys($this->nodes));
    $num_batches = (int) ($num_nodes / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $name_column = 'name';
    if ($this->match_type == 1) {
      $name_column = 'uniquename';
    }

    $sql = "
      SELECT NN.network_node_id, F.name, F.uniquename
      FROM {network_node} NN
        INNER JOIN {feature} F on F.feature_id = NN.feature_id
      WHERE NN.network_id = :network_id and F.feature_id IN (:ids)";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $ids = [];
    $network_id = $this->network->getID();
    foreach ($this->nodes as $info) {
      $feature_id = $info['feature_id'];
      $total++;
      $i++;
      $ids[] = $feature_id;

      // If we've reached the size of the batch then let's do the select.
      if ($i == $batch_size or $total == $num_nodes) {
        if (count($ids) > 0) {
          $args = [':network_id' => $network_id, ':ids' => $ids];
          $results = chado_query($sql, $args);
          while ($n = $results->fetchObject()) {
            if (array_key_exists($n->$name_column, $this->nodes)) {
              $this->nodes[$n->$name_column]['network_node_id'] = $n->network_node_id;
            }
          }
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $i = 0;
        $ids = [];
      }
    }
  }
  /**
   * Check if the features exist in the database.
   */
  private function findEdges() {
    $batch_size = 1000;
    $num_edges = count(array_keys($this->edges));
    $num_batches = (int) ($num_edges / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $source_name_column = 'source_name';
    $target_name_column = 'target_name';
    if ($this->match_type == 1) {
      $source_name_column = 'source_uniquename';
      $target_name_column = 'target_uniquename';
    }

    $init_sql = "
      SELECT
        NE.network_edge_id,
        FS.name as source_name, FS.uniquename as source_uniquename,
        FT.name as target_name, FT.uniquename as target_uniquename
      FROM {network_edge} NE
        INNER JOIN {network_node} NNS ON NNS.network_node_id = NE.source_id
        INNER JOIN {network_node} NNT ON NNT.network_node_id = NE.target_id
        INNER JOIN {feature} FS on NNS.feature_id = FS.feature_id
        INNER JOIN {feature} FT on NNT.feature_id = FT.feature_id
      WHERE NE.network_id = :network_id
      ORDER BY network_edge_id\n";
    $network_id = $this->network->getID();

    $edge_ids = array_keys($this->edges);

    for ($i = 0; $i < $num_batches; $i++) {
      $offset = $i * $batch_size;
      $limit = min($batch_size, $num_edges - $offset);
      $sql = $init_sql . " LIMIT $limit OFFSET $offset\n";
      $args = [':network_id' => $network_id];
      $results = chado_query($sql, $args);

      // Iterate through the edges. They should be in the same order
      // as the edge records that were inserted.
      foreach (array_slice($edge_ids, $offset, $limit) as $edge_id) {
        $e = $results->fetchObject();
        $findex = $this->edges[$edge_id]['findex'];
        $edge = $this->getCachedElement($findex);

        // Do a simple check just to make sure the edge names match. There
        // can be multiple edges between the same nodes
        $edge_name = $e->$source_name_column . ' (-) ' . $e->$target_name_column;
        if ($edge['schema:name'] != $edge_name) {
          throw new Exception(t('Something went wrong and edges are out of order', []));
        }
        $this->edges[$edge_id]['network_edge_id'] = $e->network_edge_id;
      }
      $this->setItemsHandled($i);
    }
  }

  /**
   * Determines the number of elements in the file.
   */
  private function parseNumElements() {

    while ($this->graphml->read()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {
        $this->num_elements++;
      }
    }
  }

  /**
   * A wrapper for the XMLReader read fucntion.
   *
   * This function ensures taht the items handled gets incremented correctly.
   */
  private function readGraphML() {
    $retval = $this->graphml->read();
    if ($this->graphml->nodeType == XMLReader::ELEMENT) {
      if (!$this->quick_scan) {
        $this->addItemsHandled(1);
      }
    }
    return $retval;
  }

  /**
   * Begins parsing of the GraphML XML file.
   */
  private function parseGraphML() {

    $this->setTotalItems($this->num_elements);
    while ($this->readGraphML()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

        // If this is a 'key' tag then we are in the header section
        // that maps ids to attribute names.
        if (strcmp($this->graphml->name, 'key') == 0) {
          $id = $this->graphml->getAttribute('id');
          $this->attr_keys[$id] = $this->parseAttributes();
        }
        elseif (strcmp($this->graphml->name, 'graph') == 0) {
          $this->parseGraph();
          // If this is a quick scan then we don't need to parse the nodes
          // and edges.
          if ($this->quick_scan) {
            return;
          }
        }
      }
    }
  }

  /**
   * Retrieves a list of attributes for the element
   */
  private function parseAttributes(){

    $attrs = [];
    if ($this->graphml->attributeCount > 0) {
      $this->graphml->moveToFirstAttribute();
      do {
        $attr_name = $this->graphml->name;
        $attr_value = $this->graphml->value;

        // When running as a job, skip attributes without terms.
        if (array_key_exists('run_args', $this->arguments) and
            array_key_exists('graphml_key.' . $attr_name . '.id', $this->arguments['run_args']) and
            !$this->arguments['run_args']['graphml_key.' . $attr_name . '.id']) {
          continue;
        }
        $attrs[$attr_name] = $attr_value;

      } while($this->graphml->moveToNextAttribute());
    }
    return $attrs;
  }


  /**
   * Imports the contents of the 'graph' section of the GraphML file.
   */
  private function parseGraph() {

    $this->graph = $this->parseAttributes();

    // Check that the defaul;t edge default directionaolity is set.
    if (!array_key_exists('edgedefault', $this->graph)) {
      $this->graph['edgedefault'] = 'undirected';
    }
    if ($this->graph['edgedefault'] != 'directed' and $this->graph['edgedefault'] != 'undirected') {
      $this->logMessage("The edge type for this network should be 'directed' or 'undirected'. Setting as undirected. ",
          [],TRIPAL_WARNING);
      $this->graph['edgedefault'] = 'undirected';
    }


    // Now iterate through the nodes and edges and add them to the database.
    while ($this->readGraphML()) {
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

        if (strcmp($this->graphml->name, 'data') == 0) {
          $data = $this->parseData();
          if ($data) {
            $this->graph[$data['key']] = $data['value'];
          }
        }
        elseif (strcmp($this->graphml->name, 'node') == 0) {
          if ($this->quick_scan) {
            $this->graphml->next();
          }
          else {
            $this->parseNode();
          }
        }
        elseif (strcmp($this->graphml->name, 'edge') == 0) {
          if ($this->quick_scan) {
            $this->graphml->next();
          }
          else {
            $this->parseEdge();
          }
        }
      }
    }

    if (!$this->quick_scan) {
      $this->graph = $this->swapTerms($this->graph);
    }
    $this->graph['local:node_count'] = count($this->nodes);
    $this->graph['local:edge_count'] = count($this->edges);
  }

  /**
   * Parses data information the graph.
   */
  private function parseData(){
    $key = $this->graphml->getAttribute('key');

    // When running as a job, skip attributes without terms.
    if (array_key_exists('run_args', $this->arguments) and
        array_key_exists('graphml_key.' . $key . '.id', $this->arguments['run_args']) and
        !$this->arguments['run_args']['graphml_key.' . $key . '.id']) {
      return FALSE;
    }
    $this->graphml->read();
    $value = $this->graphml->value;
    $value = trim($value);

    if (array_key_exists('run_args', $this->arguments) and
        array_key_exists('graphml.' . $key . 'missing', $this->arguments['run_args'])) {
      $missing_str = filter_xss($this->arguments['run_args']['graphml.' . $key . 'missing']);
      if (preg_match('/^(' . $missing_str . ')$/i', $value)) {
        return FALSE;
      }
    }
    // If this is a missing value then skip it.
    else if (preg_match('/^(NA|NaN|null|None)$/i', $value)) {
      return FALSE;
    }

    // Return the data.
    return ['key' => $key, 'value' => $value];
  }

  /**
   * Parses information about a node in the graph.
   */
  private function parseNode() {

    // Create the node array for caching.
    $node = $this->parseAttributes();

    // Now iterate through data tags.
    while ($this->readGraphML()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

         if (strcmp($this->graphml->name, 'data') == 0) {
           $data = $this->parseData();
           if ($data) {
             $node[$data['key']] = $data['value'];
           }
         }
      }
      // If we're at the </node> element then we're done.
      elseif ($this->graphml->nodeType == XMLReader::END_ELEMENT) {
         if (strcmp($this->graphml->name, 'node') == 0) {
           break;
         }
      }
    }
    $node = $this->swapTerms($node);
    $node['organism_id'] = $this->organism_id;
    $node['type_id'] = $this->node_type_id;
    $this->nodeMapping[$node['id']] = $node['schema:name'];
    $this->cacheNode($node);
  }

  /**
   * Parses information about an an edge
   */
  private function parseEdge() {

    // First get the graph attributes.
    $edge = $this->parseAttributes();

    // If an ID is not present then assign one.
    if (!array_key_exists('id', $edge)) {
      $edge['id'] = 'e' . count(array_keys($this->edges));
    }

    // If a source and target attribute are not prseent then this
    // file is corrupted
    if (!array_key_exists('source', $edge) and !array_key_exists('target', $edge)) {
      throw new Exception(t('The edge %id does not have both a "source" and "target" attribute.',
          ['%id' => $edge['id']]));
    }

    while ($this->readGraphML()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

        if (strcmp($this->graphml->name, 'data')==0) {
          $data = $this->parseData();
          if ($data) {
            $edge[$data['key']] = $data['value'];
          }
        }
      }
      // If we're at the </edge> element then we're done.
      elseif ($this->graphml->nodeType == XMLReader::END_ELEMENT) {

        if (strcmp($this->graphml->name, 'edge')==0) {
          break;
        }
      }
    }
    $edge = $this->swapTerms($edge);
    $source = $this->nodeMapping[$edge['source']];
    $target = $this->nodeMapping[$edge['target']];
    $edge['source_name'] = $source;
    $edge['target_name'] = $target;
    $edge['schema:name'] = $source . ' (-) ' . $target;
    $this->cacheEdge($edge);
  }

  /**
   * Opens the cache file for read/write access.
   */
  private function openCacheFile() {
    $temp_file = drupal_tempnam('temporary://', "TripalGraphMLImport_");
    $this->cache_file_name = drupal_realpath($temp_file);
    $this->logMessage("Opening temporary cache file: !cfile",
        ['!cfile' => $this->cache_file_name]);
    $this->cache_file = fopen($this->cache_file_name, "r+");
  }

  /**
   * Closes and cleans up the cache file.
   */
  private function closeCacheFile() {
    fclose($this->cache_file);
    $this->logMessage("Removing temporary cache file: !cfile",
        ['!cfile' => $this->cache_file_name]);
    unlink($this->cache_file_name);
  }


  /**
   * Caches the processed feature from a GFF3 file
   */
  private function cacheNode($node) {
    // Make sure we're at the end of the file.
    fseek($this->cache_file, 0, SEEK_END);

    // Get the index of this location
    $findex = ftell($this->cache_file);

    // Write the serialied array for this feature to the cache file
    // and save the index into the member variable.
    fwrite($this->cache_file, serialize($node) . "\n");
    $this->nodes[$node['schema:name']]['findex'] = $findex;
    $this->nodes[$node['schema:name']]['node_id'] = $node['id'];
  }
  /**
   * Caches the processed feature from a GFF3 file
   */
  private function cacheEdge($edge) {
    // Make sure we're at the end of the file.
    fseek($this->cache_file, 0, SEEK_END);

    // Get the index of this location
    $findex = ftell($this->cache_file);

    // Write the serialied array for this feature to the cache file
    // and save the index into the member variable.
    fwrite($this->cache_file, serialize($edge) . "\n");
    $this->edges[$edge['id']]['findex'] = $findex;
    $this->edges[$edge['id']]['edge_id'] = NULL;
  }

  /**
   * Retrieves a feature using its index from the cache file.
   */
  private function getCachedElement($findex) {
    $retval = fseek($this->cache_file, $findex);
    if ($retval == -1) {
      throw new Exception(t('Cannot seek to file location, !findex, in cache file !file.',
          ['!findex' => $findex, '!file' -> $this->cache_file]));
    }
    $feature = fgets($this->cache_file);
    $feature = unserialize($feature);
    return $feature;
  }

  /**
   * Deletes all anciallary data about a feature so we can re-insert it.
   */
  private function deleteNetworkData($network) {

    $args = [':network_id' => $network->getID()];

    $sql = "DELETE from {networkprop} WHERE network_id = :network_id";
    chado_query($sql, $args);
    $sql = "DELETE from {network_edge} WHERE network_id = :network_id";
    chado_query($sql, $args);
    $sql = "DELETE from {network_attr} WHERE network_id = :network_id";
    chado_query($sql, $args);
    $sql = "DELETE from {network_node} WHERE network_id = :network_id";
    chado_query($sql, $args);
    $sql = "DELETE from {network_analysis} WHERE network_id = :network_id";
    chado_query($sql, $args);
    $sql = "DELETE from {network_cvterm} WHERE network_id = :network_id";
    chado_query($sql, $args);
  }

  /**
   * Inserts the network record.
   */
  private function insertNetwork() {
    $uname = $this->arguments['run_args']['graph_uname'];
    $name = $this->arguments['run_args']['graph_name'];

    if (!$uname) {
      $uname = $this->graph['id'];
    }
    if (!$name) {
      if (array_key_exists('schema:name', $this->graph)) {
        $name = $this->graph['schema:name'];
      }
      elseif (array_key_exists('name', $this->graph)) {
        $name = $this->graph['name'];
      }
      else {
        $name = $this->graph['id'];
      }
    }

    // If the network already exists then udpate it. Otehrwise insert.
    $this->network = new ChadoRecord('network');
    $this->network->setValues(['uniquename' => $uname]);
    if ($this->network->find()) {
      $this->network->setValues([
        'uniquename' => $uname,
        'name' => $name,
        'organism_id' => $this->organism_id,
        'type_id' => $this->network_type_id,
      ]);
      $this->network->update();

      // If this is an update then let's clear out the network data.
      $this->deleteNetworkData($this->network);
    }
    else {
      $this->network->setValues([
        'uniquename' => $uname,
        'name' => $name,
        'organism_id' => $this->organism_id,
        'type_id' => $this->network_type_id,
      ]);
      $this->network->insert();
    }

    // Add the network analysis link.
    $network_analysis = new ChadoRecord('network_analysis');
    $network_analysis->setValues([
      'network_id' => $this->network->getID(),
      'analysis_id' => $this->analysis_id,
    ]);
    $network_analysis->insert();

    // Add the network properties
    $init_sql = "INSERT INTO {networkprop} (network_id, type_id, value, rank) VALUES\n";
    $j = 0;
    $args = [];
    foreach ($this->graph as $key => $value) {
      if (array_key_exists($key, $this->terms) and preg_match('/:/', $key)) {
        // Donj't save the name as a property because the name is
        // wikth the feature recrod.
        if ($key == 'schema:name') {
          continue;
        }
        $j++;

        // Build the SQL/args.
        $term = $this->terms[$key];
        if ($term) {
          $sql .= "(:network_id_$j, :type_id_$j, :value_$j, :rank_$j),\n";
          $args[":network_id_$j"] = $this->network->getID();
          $args[":type_id_$j"] = $term->cvterm_id;
          $args[":value_$j"] = $value;
          $args[":rank_$j"] = 0;
        }
      }
    }
    if (count($args) > 0) {
      $sql = rtrim($sql, ",\n");
      $sql = $init_sql . $sql;
      chado_query($sql, $args);
    }

    // Add the network cvterms.
    $j = 0;
    $args = [];
    $init_sql = "INSERT INTO {network_cvterm} (network_id, cvterm_id) VALUES ";
    $sql = '';

    // We'll always add the direction:
    $term = chado_get_cvterm(['id' => 'OBCS:0000198']);
    if ($this->graph['edgedefault'] == 'directed') {
      $term = chado_get_cvterm(['id' => 'OBCS:0000212']);
    }
    $sql .= "(:network_id_$j, :type_id_$j),\n";
    $args[":network_id_$j"] = $this->network->getID();
    $args[":type_id_$j"] = $term->cvterm_id;
    $sql = rtrim($sql, ",\n");
    $sql = $init_sql . $sql;
    chado_query($sql, $args);

  }


  /**
   * Inserts the node records.
   */
  private function insertNodes() {
    $batch_size = 1000;
    $num_nodes = count(array_keys($this->nodes));
    $num_batches = (int) ($num_nodes / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $init_sql = "INSERT INTO {network_node} (network_id, feature_id) VALUES\n";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $sql = '';
    $args = [];
    $network_id = $this->network->getID();
    foreach ($this->nodes as $uniquename => $info) {
      $findex = $info['findex'];
      $feature_id = $info['feature_id'];
      $node = $this->getCachedElement($findex);

      $total++;
      $i++;

      $sql .= "(:network_id_$i, :feature_id_$i),\n";
      $args[":network_id_$i"] = $network_id;
      $args[":feature_id_$i"] = $feature_id;

      // If we've reached the size of the batch then let's do the insert.
      if ($i == $batch_size or $total == $num_nodes) {
        if (count($args) > 0) {
          $sql = rtrim($sql, ",\n");
          $sql = $init_sql . $sql;
          chado_query($sql, $args);
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $sql = '';
        $i = 0;
        $args = [];
      }
    }
  }

  /**
   * Inserts the node records.
   */
  private function insertEdges() {
    $batch_size = 1000;
    $num_edges = count(array_keys($this->edges));
    $num_batches = (int) ($num_edges / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $init_sql = "INSERT INTO {network_edge} (network_id, source_id, target_id) VALUES\n";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $sql = '';
    $args = [];
    $network_id = $this->network->getID();
    foreach ($this->edges as $info) {
      $findex = $info['findex'];
      $edge = $this->getCachedElement($findex);
      $source_id = $this->nodes[$edge['source_name']]['network_node_id'];
      $target_id = $this->nodes[$edge['target_name']]['network_node_id'];

      $total++;
      $i++;

      $sql .= "(:network_id_$i, :source_id_$i, :target_id_$i),\n";
      $args[":network_id_$i"] = $network_id;
      $args[":source_id_$i"] = $source_id;
      $args[":target_id_$i"] = $target_id;

      // If we've reached the size of the batch then let's do the insert.
      if ($i == $batch_size or $total == $num_edges) {
        if (count($args) > 0) {
          $sql = rtrim($sql, ",\n");
          $sql = $init_sql . $sql;
          chado_query($sql, $args);
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $sql = '';
        $i = 0;
        $args = [];
      }
    }
  }

  /**
   *
   */
  private function insertNodeProps(){
    $batch_size = 100;
    $num_nodes = count(array_keys($this->nodes));
    $num_batches = (int) ($num_nodes / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $init_sql = "INSERT INTO {network_nodeprop} (network_node_id, type_id, value, rank) VALUES\n";
    $i = 0;
    $j = 0;
    $total = 0;
    $batch_num = 1;
    $sql = '';
    $args = [];
    foreach ($this->nodes as $info) {
      $findex = $info['findex'];
      $node = $this->getCachedElement($findex);
      $total++;
      $i++;

      // Iterate through all of the properties of this feature.
      foreach ($node as $key => $value) {
        if (array_key_exists($key, $this->terms) and preg_match('/:/', $key)) {


          // Donj't save the name as a property because the name is
          // wikth the feature recrod.
          if ($key == 'schema:name') {
            continue;
          }
          $j++;

          // Build the SQL/args.
          $term = $this->terms[$key];
          $sql .= "(:network_node_id_$j, :type_id_$j, :value_$j, :rank_$j),\n";
          $args[":network_node_id_$j"] = $info['network_node_id'];
          $args[":type_id_$j"] = $term->cvterm_id;;
          $args[":value_$j"] = $value;
          $args[":rank_$j"] = 0;
        }
      }

      // If we've reached the size of the batch then let's do the insert.
      if ($i == $batch_size or $total == $num_nodes) {
        if (count($args) > 0) {
          $sql = rtrim($sql, ",\n");
          $sql = $init_sql . $sql;
          chado_query($sql, $args);
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $sql = '';
        $i = 0;
        $args = [];
      }
    }
  }

  /**
   *
   */
  private function insertEdgeProps(){
    $batch_size = 100;
    $num_edges = count(array_keys($this->edges));
    $num_batches = (int) ($num_edges / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $init_sql = "INSERT INTO {network_edgeprop} (network_edge_id, type_id, value, rank) VALUES\n";
    $i = 0;
    $j = 0;
    $total = 0;
    $batch_num = 1;
    $sql = '';
    $args = [];
    foreach ($this->edges as $info) {
      $findex = $info['findex'];
      $edge = $this->getCachedElement($findex);
      $total++;
      $i++;

      // Iterate through all of the properties of this feature.
      foreach ($edge as $key => $value) {
        if (array_key_exists($key, $this->terms) and preg_match('/:/', $key)) {
          $j++;

          // Build the SQL/args.
          $term = $this->terms[$key];
          $sql .= "(:network_edge_id_$j, :type_id_$j, :value_$j, :rank_$j),\n";
          $args[":network_edge_id_$j"] = $info['network_edge_id'];
          $args[":type_id_$j"] = $term->cvterm_id;
          $args[":value_$j"] = $value;
          $args[":rank_$j"] = 0;
        }
      }

      // If we've reached the size of the batch then let's do the insert.
      if ($i == $batch_size or $total == $num_edges) {
        if (count($args) > 0) {
          $sql = rtrim($sql, ",\n");
          $sql = $init_sql . $sql;
          chado_query($sql, $args);
        }
        $this->setItemsHandled($batch_num);
        $batch_num++;

        // Now reset all of the varables for the next batch.
        $sql = '';
        $i = 0;
        $args = [];
      }
    }
  }
}

/**
 * Implements an AJAX callback for the tripal_chado_vocab_select_term_form.
 *
 * @ingroup tripal_terms_api
 */
function tripal_network_graphml_form_ajax_callback($form, $form_state) {
  $ajax_wrapper_id = $form_state['triggering_element']['#ajax']['wrapper'];

  $delta = $form_state['storage'][$ajax_wrapper_id]['term_match_delta'];
  return $form['term_match' . $delta];
}
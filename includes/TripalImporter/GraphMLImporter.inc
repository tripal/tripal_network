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
   * See TripalImporter::form)
   */
  public function form($form, &$form_state) {

    if (array_key_exists('triggering_element', $form_state) and
        $form_state['triggering_element']['#type'] == 'submit' and
        $form_state['triggering_element']['#value'] == 'Next Step') {
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

    $edam = new ChadoRecord('db');
    $edam->setValues(['name' => 'EDAM_operation']);
    $edam->find();

    $opcv = new ChadoRecord('cv');
    $opcv->setValues(['name' => 'EDAM']);
    $opcv->find();

    $regulatory = new ChadoRecord('cvterm');
    $regulatory->setValues([
      'name' => 'Gene regulatory network analysis',
      'cv_id' => $opcv->getValue('cv_id'),
    ]);
    $regulatory->find();

    $ppi = new ChadoRecord('cvterm');
    $ppi->setValues([
      'name' => 'Protein interaction network analysis',
      'cv_id' => $opcv->getValue('cv_id'),
    ]);
    $ppi->find();

    $coexp = new ChadoRecord('cvterm');
    $coexp->setValues([
      'name' => 'Weighted correlation network analysis',
      'cv_id' => $opcv->getValue('cv_id'),
    ]);
    $coexp->find();

    $form['network_type'] = [
      '#title' => t('Network Analysis Type'),
      '#type' => t('select'),
      '#description' => t("Choose type of network described in the GraphML file."),
      '#required' => TRUE,
      '#options' => [
        '' => '',
        $coexp->getID() => $coexp->getValue('name'),
        $regulatory->getID() => $regulatory->getValue('name'),
        $ppi->getID() => $ppi->getValue('name'),
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

    $this->extractGraphMLpertinents($form_state);
    $form['gml_attributes'] = [
      '#type' => 'fieldset',
      '#title' => 'Match Properties to Terms',
      '#collapsible' => TRUE,
      '#description' => t('Below is a list of properties that were found in the GraphML file. Tripal requires that
         all properties are associated with controlled vocabulary terms. Please take a moment to map each of
         these properties to appropriate terms.  If an appropriate term is not present you can add one
         !here.  You can find appropriate terms at the !EBI. To follow best practice, only add a term to the
         "local" vocabulary if you cannot find one in an existing controlled vocabulary. If you leave
         the fields blank then the property will be automatically added to the "local" vocabulary.
         however, this is not best practice.',
          [
            '!here' => l('here', 'http://localhost/admin/tripal/loaders/chado_vocabs/chado_cv/cvterm/add', ['attributes' => ['target' => '_blank']]),
            '!EBI' => l('EBI Ontology Lookup Service' ,'https://www.ebi.ac.uk/ols/index', ['attributes' => ['target' => '_blank']])
          ]),
    ];

    $default_terms = variable_get('tripal_network_graphml_mapper');
    $delta = 0;
    foreach ($this->attr_keys as $key => $attr_info) {
      // Skip any properties that are IDs.
      if ($attr_info['name'] == 'id' or $key == 'id') {
        continue;
      }

      // Set a reasonable default of the term mapping.
      $default_term = tripal_get_term_lookup_form_result($form, $form_state,'',$delta);
      if (!$default_term and array_key_exists($attr_info['name'], $default_terms)) {
        $term = $default_terms[$attr_info['name']];
        if (preg_match('/:/', $term)) {
          list($vocabulary, $accession) = explode(':', $term);
          $term = tripal_get_term_details($vocabulary, $accession);
          if ($term) {
            $default_term = $term['name'];
          }
        }
      }
      tripal_get_term_lookup_form($form['gml_attributes'], $form_state, $default_term,
          $attr_info['name'], '', FALSE,  '', $delta,
          'tripal_network_graphml_form_ajax_callback');


      // Add a skip checkbox
      $skip_property = FALSE;
      if (array_key_exists('skip' . $delta, $form_state['input'])) {
        $skip_property = $form_state['input']['skip' . $delta];
      }
      elseif (array_key_exists('skip' . $delta, $form_state['values'])) {
        $skip_property = $form_state['values']['skip' . $delta];
      }
      elseif (array_key_exists($attr_info['name'], $default_terms) and $default_terms[$attr_info['name']] == 'SKIP') {
        $skip_property = TRUE;
      }
      $form['gml_attributes']['term_match' . $delta]['skip' . $delta] = [
        '#type' => 'checkbox',
        '#title' => 'Skip this property during import',
        '#default_value' => $skip_property,
      ];
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
   * See TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state) {
    $form_state['rebuild'] = TRUE;

    // If the submit was clicked, iterate the stage.
    $next_step = 'step1';
    if ($form_state['triggering_element']['#type'] == 'submit') {
      $next_step = $form_state['triggering_element']['#name'];
    }

    if ($next_step == 'step2') {
      $this->validateStep1($form, $form_state);
    }

    if ($next_step == 'step3') {
      $this->validateStep2($form, $form_state);
    }
  }

  /**
   *
   */
  private function validateStep1($form, &$form_state) {

    $file_local = NULL;
    $file_upload = NULL;
    $file_existing = NULL;

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

    // Put the values from this step of the form into storage.
    $form_state['storage']['file_local'] = $file_local;
    $form_state['storage']['file_upload'] = $file_upload;
    $form_state['storage']['file_upload_existing'] = $file_existing;
  }

  /**
   *
   */
  private function validateStep2($form, &$form_state) {
    $organism_id = $form_state['values']['organism_id'];
    $network_type = $form_state['values']['network_type'];

    if (!$network_type) {
      form_set_error('network_type', t("You must provide a network type."));
    }
  }

  /**
   * see TripalImporter::run()
   */
  public function run() {
    $arguments = $this->arguments['run_args'];
    $file_path = $this->arguments['files'][0]['file_path'];

    $this->graphML_file = $file_path;
    $this->organism_id = $arguments['organism_id'];
    $this->analysis_id = $arguments['analysis_id'];
    $this->network_type_id = $arguments['network_type'];

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

      $this->logMessage("Step  1 of x: Finding network size...                             ");
      $this->graphml = new XMLReader();
      $this->graphml->open($this->graphML_file);
      $this->parseNumElements();
      $this->graphml->close();
      $this->logMessage("  Found !num_elements elements                                     ",
          ['!num_elements' => number_format($this->num_elements)]);

      $this->graphml = new XMLReader();
      $this->graphml->open($this->graphML_file);

      $this->logMessage("Step  2 of x: Caching GraphML file...                             ");
      $this->parseGraphML();
      $this->graphml->close();

      $this->logMessage("Step  3 of x: Insert network ...                                  ");
      $this->insertNetwork();

      $this->logMessage("Step  6 of 26: Find existing features...                          ");
      $this->findFeatures();

      $this->logMessage("Step  4 of x: Insert network nodes...                             ");
      $this->insertNodes();
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
   * A wrapper for getting a value within an element.
   *
   * Call this only after all attributes of the element have been retrieved.
   */
  private function getGraphMLValue() {
    $this->graphml->read();
    return $this->graphml->value;
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
          $this->parseAttrKey();
        }
        elseif (strcmp($this->graphml->name, 'graph') == 0) {
          $this->parseGraph();
          // If this is a quick scan then we don't need to parse the nodes and edges.
          if ($this->quick_scan) {
            return;
          }
        }
      }
    }
  }

  /**
   * Parses a single attribute key from the GraphML header section.
   */
  private function parseAttrKey() {
    $id = $this->graphml->getAttribute('id');
    $this->attr_keys[$id] = [
      'for' => $this->graphml->getAttribute('for'),
      'name' => $this->graphml->getAttribute('attr.name'),
      'type' => $this->graphml->getAttribute('attr.type'),
    ];
  }


  /**
   * Imports the contents of the 'graph' section of the GraphML file.
   */
  private function parseGraph() {

    // First get the graph attributes and ase the id as the default network name.
    $id = $this->graphml->getAttribute('id');
    $edgedefault = $this->graphml->getAttribute('edgedefault');
    if (!$edgedefault) {
      $edgedefault = 'undirected';
    }

    // Store the information about this graph.
    $this->graph = [
      'uniquename' => $id,
      'name' => $id,
      'organism_id' => $this->organism_id,
      'type_id' => $this->network_type_id,
    ];

    // If this is a quick scan then we don't need to parse the nodes and edges.
    if ($this->quick_scan) {
      return;
    }

    // Now iterate through the nodes and edges and add them to the database.
    while ($this->readGraphML()) {
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

        if (strcmp($this->graphml->name, 'node') == 0) {
          $this->parseNode();
        }
        elseif (strcmp($this->graphml->name, 'edge') == 0) {
          //$this->parseEdge();
        }
      }
    }
  }
  /**
   * Parses information about a node in the graph.
   */
  private function parseNode() {

    // Create the node array for caching.
    $node = [
      'id' => $this->graphml->getAttribute('id'),
    ];

    // Now iterate through data tags.
    while ($this->readGraphML()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

         if (strcmp($this->graphml->name, 'data')==0) {
           $key = $this->graphml->getAttribute('key');
           $node[$key] = $this->getGraphMLValue();
         }
      }
      // If we're at the </node> element then we're done.
      elseif ($this->graphml->nodeType == XMLReader::END_ELEMENT) {
         if (strcmp($this->graphml->name, 'node')==0) {
           break;
         }
      }
    }
    $this->cacheNode($node);
  }

  /**
   * Parses information about an an edge
   */
  private function parseEdge() {

    // First get the graph attributes.
    $edge = [
      'id' => 'e' . count(array_keys($this->edges)),
      'source' => $this->graphml->getAttribute('source'),
      'target' => $this->graphml->getAttribute('target'),
    ];

    while ($this->readGraphML()){
      if ($this->graphml->nodeType == XMLReader::ELEMENT) {

        if (strcmp($this->graphml->name, 'data')==0) {
          $key = $this->graphml->getAttribute('key');
          $edge[$key] = $this->getGraphMLValue();
        }
      }
      // If we're at the </edge> element then we're done.
      elseif ($this->graphml->nodeType == XMLReader::END_ELEMENT) {

        if (strcmp($this->graphml->name, 'edge')==0) {
          break;
        }
      }
    }
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
    $this->nodes[$node['id']]['findex'] = $findex;
    $this->nodes[$node['id']]['node_id'] = NULL;
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
   * Inserts the network record.
   */
  private function insertNetwork() {
    $this->network = new ChadoRecord('network');
    $this->network->setValues([
      'uniquename' => $this->graph['uniquename'],
      'name' => $this->graph['name'],
      'organism_id' => $this->graph['organism_id'],
      'type_id' => $this->graph['type_id'],
    ]);
    $this->network->insert();
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

    $sql = "SELECT uniquename, type_id, organism_id, feature_id FROM {feature} WHERE uniquename in (:uniquenames)";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $names = [];
    foreach ($this->nodes as $id => $info) {
      $total++;

      if (!$feature_id) {
        $i++;
        $names[] = $uniquename;
      }

      // If we've reached the size of the batch then let's do the select.
      if ($i == $batch_size or $total == $num_features) {
        if (count($names) > 0) {
          $args = [':uniquenames' => $names];
          $results = chado_query($sql, $args);
          while ($f = $results->fetchObject()) {
            if (array_key_exists($f->uniquename, $this->features)) {
              $matched_findex = $this->features[$f->uniquename]['findex'];
              $matched_feature = $this->getCachedFeature($matched_findex);
              $matched_type_id = $this->feature_cvterm_lookup[$matched_feature['type']];
              $matched_organism_id = $this->organism->getID();
              if ($matched_feature['organism']) {
                $matched_organism_id = $matched_feature['organism'];
              }
              if ($matched_type_id == $f->type_id and $matched_organism_id == $f->organism_id) {
                $this->features[$f->uniquename]['feature_id'] = $f->feature_id;
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
   * Inserts the node records.
   */
  private function insertNodes() {
    $batch_size = 1000;
    $num_nodes = count(array_keys($this->nodes));
    $num_batches = (int) ($num_nodes / $batch_size) + 1;

    $this->setItemsHandled(0);
    $this->setTotalItems($num_batches);

    $init_sql = "
      INSERT INTO {networknode}
        (uniquename, name, type_id, organism_id, residues, md5checksum,
         seqlen, is_analysis, is_obsolete)
      VALUES\n";
    $i = 0;
    $total = 0;
    $batch_num = 1;
    $sql = '';
    $args = [];
    foreach ($this->features as $uniquename => $info) {
      $findex = $info['findex'];
      $feature_id = $info['feature_id'];
      $feature = $this->getCachedFeature($findex);

      $total++;
      $i++;

      // Only do an insert if this feature doesn't already exist in the databse.
      if (!$feature_id and !$feature['skipped']) {
        $residues = '';
        $type_id = $this->feature_cvterm_lookup[$feature['type']];
        $sql .= "(:uniquename_$i, :name_$i, :type_id_$i, :organism_id_$i, :residues_$i, " .
        " :md5checksum_$i, :seqlen_$i, FALSE, FALSE),\n";
        $args[":uniquename_$i"] = $uniquename;
        $args[":name_$i"] = $feature['name'];
        $args[":type_id_$i"] = $type_id;
        $args[":organism_id_$i"] = $feature['organism'] ? $feature['organism'] : $this->organism->getID();
        $args[":residues_$i"] = $residues;
        $args[":md5checksum_$i"] = $residues ? md5($residues) : '';
        $args[":seqlen_$i"] = strlen($residues);
      }

      // If we've reached the size of the batch then let's do the insert.
      if ($i == $batch_size or $total == $num_features) {
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
  return $form['gml_attributes']['term_match' . $delta];
}
<?php
class sio__network_diagram extends ChadoField {

  // The default lable for this field.
  public static $default_label = 'Local Network Viewer';

  // The default description for this field.
  public static $description = 'A network diagram consists of a set of vertices connected by edges.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'NCIT',
    // The name of the term.
    'term_name' => 'Network',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'C61377',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );
  // The default widget for this field.
  public static $default_widget = 'sio__network_diagram_widget';
  // The default formatter for this field.
  public static $default_formatter = 'sio__network_diagram_formatter';

  /**
   * @see TripalField::load()
   */
  public function load($entity) {
    $record = $entity->chado_record;
    $settings = $this->instance['settings'];
    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    // Get the term used for the URL
    $url_type = tripal_get_chado_semweb_term('db', 'url');

    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0]['value'] = [];

    $i = 0;
    if ($field_table == 'network' ) {
      $entity->{$field_name}['und'][$i]['value'][$url_type] = url('fix_me');
      $entity->{$field_name}['und'][$i]['network_id'] = $record->network_id;
    }
    if ($field_table == 'feature') {
      $sql = "
         SELECT NN.network_id
         FROM {network_node} NN
           INNER JOIN {feature} F ON F.feature_id = NN.feature_id
         WHERE F.feature_id = :feature_id
      ";
      $results = chado_query($sql, [':feature_id' => $record->feature_id]);
      while($result = $results->fetchObject()) {
        $entity->{$field_name}['und'][$i]['value'][$url_type] = url('fix_me');
        $entity->{$field_name}['und'][$i]['network_id'] = $result->network_id;
        $i++;
      }
    }
  }

  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    // Get the terms for each of the keys for the 'values' property.
    $uname_term = tripal_get_chado_semweb_term('network', 'uniquename');
    $name_term = tripal_get_chado_semweb_term('network', 'name');
    $organism_term = tripal_get_chado_semweb_term('network', 'organism_id');
    $type_term = tripal_get_chado_semweb_term('network', 'type_id');

    return array(
      $field_term => array(
        'operations' => array('eq', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
        'readonly' => TRUE,
        'type' => 'xs:complexType',
        'elements' => [],
      ),
    );
  }
}

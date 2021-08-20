<?php
class sio__attribute extends ChadoField {

  // The default lable for this field.
  public static $default_label = 'Network Attribute';

  // The default description for this field.
  public static $description = 'A node, edge or grpah attribute for the network.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'SIO',
    // The name of the term.
    'term_name' => 'attribute',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '000614',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );
  // The default widget for this field.
  public static $default_widget = 'sio__attribute_widget';
  // The default formatter for this field.
  public static $default_formatter = 'sio__attribute_formatter';

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
    $base_table = $this->instance['settings']['base_table'];

    $vocabulary = $this->instance['settings']['term_vocabulary'];
    $accession = $this->instance['settings']['term_accession'];
    $cvterm = chado_get_cvterm(['id' => $vocabulary . ':' . $accession]);
    $cvterm_id = $cvterm->cvterm_id;


    $chado_record = $entity->chado_record;

    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = [
      'value' => '',
    ];

    // Get all the attributes and all properties for them.
    $sql = "
      SELECT
        NA.network_attr_id, CVTA.name as attr_name,
        CVTD.name as domain,
        CVTP.name as prop_name, DBXP.accession, DB.name as db_name,
        NAP.value, NAP.value_num, CVTV.name as value_name, NAP.rank
      FROM {network_attr} NA
        INNER JOIN {network} N on N.network_id = NA.network_id
        INNER JOIN {cvterm} CVTD on CVTD.cvterm_id = NA.domain_id
        INNER JOIN {cvterm} CVTA on CVTA.cvterm_id = NA.attr_id
        INNER JOIN {network_attrprop} NAP on NAP.network_attr_id = NA.network_attr_id
        INNER JOIN {cvterm} CVTP on CVTP.cvterm_id = NAP.type_id
        INNER JOIN {dbxref} DBXP on DBXP.dbxref_id = CVTP.dbxref_id
        INNER JOIN {db} DB on DB.db_id = DBXP.db_id
        LEFT JOIN {cvterm} CVTV on CVTV.cvterm_id = NAP.value_id
      WHERE NA.network_id = :network_id
    ";

    // Store the attributes in a handy array keyed by attribute.
    $properties = chado_query($sql, [':network_id' => $chado_record->network_id]);
    $attributes = [];
    while ($property = $properties->fetchObject()) {
      if (!array_key_exists($property->network_attr_id, $attributes)) {
        $attributes[$property->network_attr_id] = [
          'name' => $property->attr_name,
          'domain' => $property->domain,
          'properties' => [],
        ];
      }
      $attributes[$property->network_attr_id]['properties'][] = [
        'prop_name'=> $property->prop_name,
        'value' => $property->value,
        'value_num' => $property->value_num,
        'value_name' => $property->value_name,
        'term' => $property->db_name . ':' . $property->accession,
      ];
    }

    $domain_term = chado_get_semweb_term('network_attr', 'domain_id');
    $attr_id = chado_get_semweb_term('network_attr', 'attr_id');


    // Now add the attributes to the value array.
    $i = 0;
    foreach ($attributes as $attribute) {
      $entity->{$field_name}['und'][$i] = [
        'value' => [
          $domain_term => $attribute['domain'],
          $attr_id => $attribute['name'],
        ]
      ];
      foreach ($attribute['properties'] as $property) {
        $value = $property['value'];
        if (!$value) {
          $value = $property['value_num'];
        }
        if (!$value) {
          $value = $property['value_name'];
        }
        // If only one value is prsent then leave it as a scalar, otherwise,
        // expand it to an array.
        $prop_term = $property['term'];
        if (!array_key_exists($prop_term, $entity->{$field_name}['und'][$i]['value'])) {
          $entity->{$field_name}['und'][$i]['value'][$prop_term] = $value;
        }
        else if (is_array($entity->{$field_name}['und'][$i]['value'][$prop_term])) {
          $entity->{$field_name}['und'][$i]['value'][$prop_term][] = $value;
        }
        else {
          $prev_value = $entity->{$field_name}['und'][$i]['value'][$prop_term];
          $entity->{$field_name}['und'][$i]['value'][$prop_term] = [$prev_value, $value];
        }
      }
      $i++;
    }

    // Ensure there are no values if there are no properties.
    // This is necessary to make sure the field is not rendered when there are no properies.
    // @todo: We should revisit this in the future as none of the other fields do this.
    //        It was added here to make it easier to detect when the field was empty
    //        but in hindsight, we would just check $entity->{$field_name}['und'][$i]['value']
    //        in the formatter.
    if (empty($properties)) {
      unset($entity->{$field_name});
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

<?php

// $Id$

/**
 * Copyright 2011 ILRI
 *
 * This file is part of DataBridge.
 * 
 * DataBridge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * DataBridge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DataBridge.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *   Form functions used by the DataBridge module
*/

/**
 * Creates a drupal for for inputing analysis information.
 *
 * This form is used to put analysis information into the internal database as
 * part of the configuration process.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
 *
 * @return array $form
 *   A drupal form array
*/
function DataBridge_analysis_form($form, &$form_state) {
  $analysis = DataBridge_getAnalysis();
  
  if (!$analysis) {
    $analysis = array('new' => array());
  } else {
    $analysis['new'] = array();
  }
  
  foreach ( array_keys($analysis) as $name ) {
    $form += DataBridge_analysis_input($name, $analysis[$name]);
  }
  $form['submit'] = array('#type' => 'submit',
                          '#value' => 'Save Analysis',);
  return $form;
}

/**
 * Validation functions for the analysis form
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_analysis_form_validate($form, &$form_state) {
  // TODO: make sure that there are no # (hashes) in the names
}

/*
 * Submit function for the analysis form.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_analysis_form_submit($form, &$form_state) {
  $analysis = array();
  foreach ( $form_state['values'] as $key => $value ) {
    preg_match('/(.*)_name/', $key, $match);
    if ($match) {
      $analysis[$match[1]] = array('id'         => ($match[1] == 'new') ? NULL : $match[1],
                                  'name'        => $form_state['values'][$match[1] . '_name'],
                                  'protocol'    => $form_state['values'][$match[1] . '_protocol'],
                                  'nr_of_vars'  => $form_state['values'][$match[1] . '_nr_of_vars']
                                                 ? $form_state['values'][$match[1] . '_nr_of_vars']
                                                 : 0,
                                  'description' => $form_state['values'][$match[1] . '_description'],
                                  'species'     => strtolower(
                                                      implode(',', 
                                                      $form_state['values'][$match[1] . '_species'])),
                                  );
    }
  }
  foreach ( $analysis as $key => $value ) {
    if ($key == 'new') {
      if ($analysis[$key]['name']) {
        DataBridge_insertAnalysis($analysis[$key]);
      }
    } else {
      DataBridge_updateAnalysis($analysis[$key]);
    }
  }
}

/**
 * A small drupal form for inputing information about a type of sample analysis
 *
 * This function is used to create an input field for information about a 
 * single analysis type. Use DataBridge_analysis_form to get the entire form.
 *
 * @param string $id
 *   Id of the analysis form elements.
 * @param array $default_values = NULL;
 *   An array of default values for the fields 
 * @param bool $table = true
 *   If true the form will be inside a table
 * 
 * @return array $form
 *   A drupal form.
*/
function DataBridge_analysis_input($id, $default_values = NULL, $table = true) {
  $form = array();
  $species = array();
  foreach (DataBridge_getSpecies() as $animal) {
    $species[$animal['id']] = $animal['common_name'];
  }
  
  $prefix = $table ? '<table><tr><td rowspan="2">' : '';
  $suffix = $table ? '</td></tr></table>' : '';
  
  $form[$id . '_name']       = array('#prefix'        => $prefix,
                                     '#suffix'        => $table ? '</td>' : '',
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Name', 
                                     '#default_value' => $default_values ? $default_values['name'] : '', 
                                     '#size'          => 40,
                                     );
  $form[$id . '_protocol']   = array('#prefix'        => $table ? '<td>' : '',
                                     '#suffix'        => $table ? '</td>' : '',
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Protocol', 
                                     '#default_value' => $default_values ? $default_values['protocol'] : '',
                                     '#size'          => 40,
                                      );
  $form[$id . '_description'] = array('#prefix'       => $table ? '<td rowspan="2">' : '',
                                     '#suffix'        => $table ? '</td>' : '',
                                     '#type'          => 'textarea', 
                                     '#title'         => 'Description', 
                                     '#default_value' => $default_values ? $default_values['description'] : '',
                                     '#size'          => 40,
                                      );
  $form[$id . '_species']    = array('#prefix'        => $table ? '<td rowspan="2">' : '',
                                     '#suffix'        => $table ? '</td></tr>' : '',
                                     '#type'          => 'select',
                                     '#multiple'      => TRUE,
                                     '#options'       => $species, 
                                     '#title'         => 'Species', 
                                     '#default_value' => $default_values ? explode(',', $default_values['species']) : '',
                                     '#size'          => 6,
                                      );
  $form[$id . '_nr_of_vars'] = array('#prefix'   => $table ? '<td>' : '',
                                     '#suffix'        => $suffix,
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Nr. of variables', 
                                     '#default_value' => $default_values ? $default_values['nr_of_vars'] : '',
                                     '#size'          => 40,);
  return $form;
}

/**
* Creates a drupal for for inputing animal information.
*
* This form is used to put animal information into the internal database as
* part of the configuration process.
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*
* @return array $form
*   A drupal form array
*/
function DataBridge_animals_form($form, &$form_state) {
  $values = DataBridge_getAnimals();
  
  // animal list query
  $form['animal'] = array('#type'        => 'fieldset', 
                          '#title'       => 'Animal extraction queries', 
                          '#description' => t('Defines how to get animal information from the database.'), 
                          '#collapsible' => TRUE, 
                          );
  $form['animal']['select'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'SELECT fields', 
             '#description' => t('Defines the fields to select from the database.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['animal']['tables'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'FROM fields', 
             '#description' => t('Defines from where in the database the information should be selected.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['animal']['where'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'WHERE fields', 
             '#description' => t('Defines how the query should be filtered.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['animal']['details'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'DETAIL fields', 
             '#description' => t('Defines the fields to select from the database in the detail view.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['animal']['select']['id'] = 
       array('#type'        => 'hidden',
             '#value'       => isset($values['id']) ? $values['id'] : NULL);
  $form['animal']['select']['select_id'] = 
       array('#prefix'      => '<h3>SELECT</h3>',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'id', 
             '#default_value' => isset($values['select_id']) ? $values['select_id'] : NULL, 
             '#size'          => 40,);
  $form['animal']['select']['select_species'] = 
       array('#prefix' => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'species', 
             '#default_value' => isset($values['select_species']) ? $values['select_species'] : NULL, 
             '#size'          => 40,);
  $form['animal']['select']['select_sex'] = 
       array('#prefix'     => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'sex', 
             '#default_value' => isset($values['select_sex']) ? $values['select_sex'] : NULL, 
             '#size'          => 40,);
  foreach ( DataBridge_getAnalysis() as $analysis ) {
    $form['animal']['select']['select_' . $analysis['name']] = 
      array('#prefix'        => '',
            '#suffix'        => '',
            '#type'          => 'textfield', 
            '#title'         => $analysis['name'], 
            '#default_value' => isset($values['select_' . $analysis['name']]) ? $values['select_' . $analysis['name']] : NULL, 
            '#size'          => 40,);
    if ( $analysis['nr_of_vars'] ) {
      for ( $i = 0; $i < $analysis['nr_of_vars']; $i++ ) {
        $form['animal']['select']['select_' . $analysis['name'] . '_var_' . $i] =
          array('#prefix'        => '',
                '#suffix'        => '',
                '#type'          => 'textfield', 
                '#title'         => $analysis['name'] . ' variable ' . ($i + 1), 
                '#default_value' => isset($values['select_' . $analysis['name'] . '_var_' . $i]) ? $values['select_' . $analysis['name'] . '_var_' . $i] : NULL, 
                '#size'          => 40,);
      }
    }
  }
  $form['animal']['tables']['from'] = 
       array('#prefix'        => '<h3>FROM</h3>',
             '#suffix'        => '',
             '#type'          => 'textarea', 
             '#title'         => '', 
             '#default_value' => isset($values['from']) ? $values['from'] : NULL,);
  $form['animal']['where']['where_date'] = 
       array('#prefix'        => '<h3>WHERE</h3>',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => '= sampling date', 
             '#default_value' => isset($values['where_date']) ? $values['where_date'] : NULL,);
  $form['animal']['where']['where_origin'] = 
       array('#prefix'  => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => '= sampling origin', 
             '#default_value' => isset($values['where_origin']) ? $values['where_origin'] : NULL,);
  
  $form['animal']['details']['detail_query'] = 
       array('#prefix'        => '<h3>SELECT DETAILS</h3>',
             '#suffix'        => '',
             '#type'          => 'textarea', 
             '#title'         => '', 
             '#default_value' => isset($values['detail_query']) ? $values['detail_query'] : NULL,);
  
  
  $form['submit'] = array('#type' => 'submit',
                          '#value' => 'Save Queries',);
  return $form;
}

/**
* Validation of animal query input form
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*/
function DataBridge_animals_form_validate($form, &$form_state) {
}

/**
* Submits the animal query input form
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*/
function DataBridge_animals_form_submit($form, &$form_state) {
  $fields = array('id'         => NULL,
                  'name'       => 'animals',
                  'fields'     => '',
                  'tables'     => '',
                  'conditions' => '',);
  $details = '';
  foreach ( $form_state['values'] as $key => $value ) {
    if ( $key == 'id' ) {
      $fields['id'] = $value;
    }
    elseif ( $key == 'from' ) {
      $fields['tables'] .= $value;
    }
    elseif ( preg_match('/select_(.*)/', $key, $match) ) {
      if ( !$fields['fields'] ) {
        $fields['fields'] = $value . ' AS ' . $match[1];
      } else {
        $fields['fields'] .= '|' . $value . ' AS ' . $match[1];
      }
    }
    elseif ( preg_match('/where_(.*)/', $key, $match) ) {
      if ( !$fields['conditions'] ) {
        $fields['conditions'] = $value . ':' . $match[1];
      } else {
        $fields['conditions'] .= '|' . $value . ':' . $match[1];
      }
    }
    elseif ( preg_match('/detail_(.*)/', $key, $match) ) {
      $details .= ':' . $value;
    }
  }
  $fields['fields'] .= $details;
  if ( !isset($fields['id']) ) {
    DataBridge_insertAnimals($fields);
  } else {
    DataBridge_updateAnimals($fields);
  }
}

/**
* Creates a drupal for for inputing sample information.
*
* This form is used to put sample information into the internal database as
* part of the configuration process.
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*
* @return array $form
*   A drupal form array
*/
function DataBridge_samples_form($form, &$form_state) {
  $values = DataBridge_getSamples();
  
  // animal list query
  $form['sample'] = array('#type'        => 'fieldset', 
                          '#title'       => 'Sample extraction queries', 
                          '#description' => t('Defines how to get sample information from the database.'), 
                          '#collapsible' => TRUE, 
                          );
  $form['sample']['select'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'SELECT fields', 
             '#description' => t('Defines the fields to select from the database.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['sample']['tables'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'FROM fields', 
             '#description' => t('Defines from where in the database the information should be selected.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['sample']['where'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'WHERE fields', 
             '#description' => t('Defines how the query should be filtered.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['sample']['details'] = 
       array('#type'        => 'fieldset', 
             '#title'       => 'DETAIL fields', 
             '#description' => t('Defines the fields to select from the database in the detail view.'), 
             '#collapsible' => TRUE, 
             '#collapsed'   => TRUE,
            );
  $form['sample']['select']['id'] = 
       array('#type'        => 'hidden',
             '#value'       => isset($values['id']) ? $values['id'] : NULL);
  $form['sample']['select']['select_id'] = 
       array('#prefix'      => '<h3>SELECT</h3>',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'id', 
             '#default_value' => isset($values['select_id']) ? $values['select_id'] : NULL, 
             '#size'          => 40,
             '#maxlength'     => 400,);
  $form['sample']['select']['select_type'] = 
       array('#prefix' => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'sample type', 
             '#default_value' => isset($values['select_type']) ? $values['select_type'] : NULL, 
             '#size'          => 40,
             '#maxlength'     => 400,);
  $form['sample']['select']['select_date'] = 
       array('#prefix'     => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'sampling date', 
             '#default_value' => isset($values['select_date']) ? $values['select_date'] : NULL, 
             '#size'          => 40,
             '#maxlength'     => 400,);
  $form['sample']['select']['select_animal'] = 
       array('#prefix'     => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'source animal', 
             '#default_value' => isset($values['select_animal']) ? $values['select_animal'] : NULL, 
             '#size'          => 40,
             '#maxlength'     => 400,);
  $form['sample']['select']['select_storage'] = 
       array('#prefix'     => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'sample storage', 
             '#default_value' => isset($values['select_storage']) ? $values['select_storage'] : NULL, 
             '#size'          => 40,
             '#maxlength'     => 400,);
   $form['sample']['select']['select_latitude'] = 
        array('#prefix'     => '',
              '#suffix'        => '',
              '#type'          => 'textfield', 
              '#title'         => 'sampling latitude', 
              '#default_value' => isset($values['select_latitude']) ? $values['select_latitude'] : NULL, 
              '#size'          => 40,
              '#maxlength'     => 400,);
    $form['sample']['select']['select_longitude'] = 
         array('#prefix'     => '',
               '#suffix'        => '',
               '#type'          => 'textfield', 
               '#title'         => 'sampling longitude', 
               '#default_value' => isset($values['select_longitude']) ? $values['select_longitude'] : NULL, 
               '#size'          => 40,
               '#maxlength'     => 400,);
  $form['sample']['select']['select_parent'] = 
       array('#prefix'        => '',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => 'parent sample', 
             '#size'          => 40,
             '#maxlength'     => 400,
             '#default_value' => isset($values['select_parent']) ? $values['select_parent'] : NULL,);
  foreach ( DataBridge_getAnalysis() as $analysis ) {
    $form['sample']['select']['select_' . $analysis['name']] = 
      array('#prefix'        => '',
            '#suffix'        => '',
            '#type'          => 'textfield', 
            '#title'         => $analysis['name'], 
            '#default_value' => isset($values['select_' . $analysis['name']]) ? $values['select_' . $analysis['name']] : NULL, 
            '#size'          => 40,
            '#maxlength'     => 400,);
  }
  $form['sample']['tables']['from'] = 
       array('#prefix'        => '<h3>FROM</h3>',
             '#suffix'        => '',
             '#type'          => 'textarea', 
             '#title'         => '', 
             '#default_value' => isset($values['from']) ? $values['from'] : NULL,);
  $form['sample']['where']['where_animal'] = 
       array('#prefix'        => '<h3>WHERE</h3>',
             '#suffix'        => '',
             '#type'          => 'textfield', 
             '#title'         => '= animal id', 
             '#size'          => 40,
             '#maxlength'     => 400,
             '#default_value' => isset($values['where_animal']) ? $values['where_animal'] : NULL,);
  $form['sample']['details']['detail_query'] = 
       array('#prefix'        => '<h3>SELECT DETAILS</h3>',
             '#suffix'        => '',
             '#type'          => 'textarea', 
             '#title'         => '', 
             '#default_value' => isset($values['detail_query']) ? $values['detail_query'] : NULL,);
  
  
  $form['submit'] = array('#type' => 'submit',
                          '#value' => 'Save Queries',);
  return $form;
}

/**
* Validation of sample query input form
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*/
function DataBridge_samples_form_validate($form, &$form_state) {
}

/**
* Submits the sample query input form
*
* @param array $form
*   The form that is to be generated, supplied by drupal
* @param array &$form_state
*   The current state of the form, supplied by drupal
*/
function DataBridge_samples_form_submit($form, &$form_state) {
  $fields = array('id'         => NULL,
                  'name'       => 'samples',
                  'fields'     => '',
                  'tables'     => '',
                  'conditions' => '',);
  $details = '';
  foreach ( $form_state['values'] as $key => $value ) {
    if ( $key == 'id' ) {
      $fields['id'] = $value;
    }
    elseif ( $key == 'from' ) {
      $fields['tables'] .= $value;
    }
    elseif ( preg_match('/select_(.*)/', $key, $match) ) {
      if ( $value ) {
        if ( $fields['fields'] ) {
          $fields['fields'] .= '|';
        }
        $fields['fields'] .= $value . ' AS ' . $match[1];
      }
    }
    elseif ( preg_match('/where_(.*)/', $key, $match) ) {
      if ( !$fields['conditions'] ) {
        $fields['conditions'] = $value . ':' . $match[1];
      } else {
        $fields['conditions'] .= '|' . $value . ':' . $match[1];
      }
    }
    elseif ( preg_match('/detail_(.*)/', $key, $match) ) {
      $details .= ':' . $value;
    }
  }
  $fields['fields'] .= $details;
  if ( !isset($fields['id']) ) {
    DataBridge_insertSamples($fields);
  } else {
    DataBridge_updateSamples($fields);
  }
}

/**
 * Creates a drupal for for inputing sampling information.
 *
 * This form is used to put sampling information into the internal database as
 * part of the configuration process.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
 *
 * @return array $form
 *   A drupal form array
*/
function DataBridge_samplings_form($form, &$form_state) {
  
  foreach ( DataBridge_getClasses() as $class ) {
    $fields = DataBridge_getSamplings($class, true);
    $form[$class] = DataBridge_query_form($class, $fields);
  }
  
  $form['submit'] = array('#type'  => 'submit', 
                          '#value' => t('Save Values'),);
  
  return $form;
}

/**
 * Validates the sampling form.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_samplings_form_validate($form, &$form_state) {
}

/**
 * Submits the sampling form.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_samplings_form_submit($form, &$form_state) {
  $queries = array();
  foreach ( DataBridge_getClasses() as $class ) {  
    $queries[$class] = array('name'   => $class);
    $fields = '';
    foreach ( $form_state['values'] as $field => $value) {
      if ( preg_match('/' . $class . '_(.*)/', $field, $match) and $value) {
        $fields .= $fields ? '|' . $value : $value;
        $fields .= ' AS `' . $match[1] . '`';
      }
      elseif ( preg_match('/from_' . $class . '/', $field) ) {
        $queries[$class]['tables'] = $value;
      }
    }
    $queries[$class]['fields'] = $fields;
  }
  
  foreach ($queries as $name => $query) {
    if ( $values = DataBridge_getQuery($name) ) {
      $query['id'] = $values['id'];
      DataBridge_updateSampling($query);
    } else {
      DataBridge_insertSampling($query);
    }
  }
}

/**
 * Creates the query input form.
 *
 * @param string $class
 *   Title of the fieldset
 * @param array $data 
 *   Array of fields to put in the fieldset.
 *
 * @return array $form
 *   A query config form with three fieldsets. 
*/
function DataBridge_query_form($class = '', $data) {
  $form = array();
  $title = strtolower(str_replace('_', ' ', $class));
  
  $form[$class] = array(
    '#type'        => 'fieldset', 
    '#title'       => t(ucfirst($title) . ' extraction queries'), 
    '#description' => t('Defines how to access information about ' . $title . ' from the database.'), 
    '#collapsible' => TRUE, 
  );
  $form[$class]['fields'] = array(
    '#type'        => 'fieldset', 
    '#attributes'  => array('class' => array('databridge_field')), 
    '#title'       => t('Database Fields'), 
    '#description' => t('Database query strings to get field data from the remote database.'), 
    '#collapsed'   => TRUE, 
    '#collapsible' => TRUE, 
  );
  if ( is_array($data) and in_array('fields', array_keys($data)) ) {
    $form[$class]['fields'] += DataBridge_sampling_input($class . '_', $data['fields']);
  }
  $form[$class]['tables'] = array(
    '#type'        => 'fieldset', 
    '#attributes'  => array('class' => array('databridge_field')), 
    '#title'       => t('Database Tables'), 
    '#description' => t('Database query strings to get the tables to extract data from.'), 
    '#collapsed'   => TRUE, 
    '#collapsible' => TRUE, 
  );
  $form[$class]['tables']['from_' . $class] = 
             array('#type'          => 'textarea', 
                   '#title'         => 'SQL', 
                   '#cols'          => 60, 
                   '#default_value' => (is_array($data) and in_array('tables', array_keys($data))) ? $data['tables'] : '', 
                   '#required'      => TRUE, 
                  );
    
  return $form;
}

/**
 * Generates a table of textfields for the query input form
 *
 * @param string prefix
 *   A string prefix given to all fields
 *
 * @param array $fields
 *   An array of field names and values, each get a textfield in the form. 
 *   The array should be formatted like array ('field_name' => 'value').
 *
 * @return array $form
 *   A drupal form array
*/
function DataBridge_sampling_input($prefix = '', $fields = array()) {
  $form = array();

  foreach (array_keys($fields) as $field) {
    $form[$prefix . $field] =
      array('#type'    => 'textfield',
      '#title'         => $field,
  	  '#default_value' => $fields[$field],
  	  '#size'          => 90,
  	  '#maxlength'     => 200,
  	  );
  }

  return $form;
}

/**
 * Creates a drupal form for inputing species information.
 *
 * This form is used to put species information into the internal database as
 * part of the configuration process.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
 *
 * @return array $form
 *   A drupal form array
*/
function DataBridge_species_form($form, &$form_state) {
  $species = DataBridge_getSpecies();
  
  if (!$species) {
    $species = array('new' => array());
  } else {
    $species['new'] = array();
  }
  
  foreach ( array_keys($species) as $name ) {
    $form += DataBridge_species_input($name, $species[$name]);
  }
  $form['submit'] = array('#type' => 'submit',
                          '#value' => 'Save Species',);
  return $form;
}

/**
 * Validation for the species form.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_species_form_validate($form, &$form_state) {
}

/*
 * Submit function for the species form.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function DataBridge_species_form_submit($form, &$form_state) {
  $species = array();
  foreach ( $form_state['values'] as $key => $value ) {
    preg_match('/(.*)_common/', $key, $match);
    if ($match) {
      $species[$match[1]] = array('id' => ($match[1] == 'new') ? NULL : $match[1],
                                  'common_name' => $form_state['values'][$match[1] . '_common'],
                                  'scientific_name' => $form_state['values'][$match[1] . '_scientific'],
                                  'class' => $form_state['values'][$match[1] . '_class']);
    }
  }
  foreach ( $species as $key => $value ) {
    if ($key == 'new') {
      if ($species[$key]['common_name']) {
        DataBridge_insertSpecies($species[$key]);
      }
    } else {
      DataBridge_updateSpecies($species[$key]);
    }
  }
}

/**
 * A small drupal form for inputing information about a species of animal.
 *
 * This function is used to create an input field for information about a 
 * single species. Use DataBridge_species_form to get the entire form.
 *
 * @param string $id
 *   Id of the species form elements.
 * @param array $default_values = NULL;
 *   An array of default values for the fields 
 * @param bool $table = true
 *   If true the form will be inside a table
 * 
 * @return array $form
 *   A drupal form.
*/
function DataBridge_species_input($id, $default_values = NULL, $table = true) {
  $form = array();
  
  $prefix = $table ? '<table><tr><td>' : '';
  $suffix = $table ? '</td></tr></table>' : '';
  
  $form[$id . '_common']     = array('#prefix'        => $prefix,
                                     '#suffix'        => $table ? '</td>' : '',
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Common Name', 
                                     '#default_value' => $default_values ? $default_values['common_name'] : '', 
                                     '#size'          => 40,);
  $form[$id . '_scientific'] = array('#prefix'        => $table ? '<td>' : '',
                                     '#suffix'        => $table ? '</td>' : '',
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Scientific Name', 
                                     '#default_value' => $default_values ? $default_values['scientific_name'] : '',
                                     '#size'          => 40,
                                      );
  $form[$id . '_class']      = array('#prefix'        => $table ? '<td>' : '',
                                     '#suffix'        => $suffix,
                                     '#type'          => 'textfield', 
                                     '#title'         => 'Class', 
                                     '#default_value' => $default_values ? $default_values['class'] : '',
                                     '#size'          => 40,);
  return $form;
}

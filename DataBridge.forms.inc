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
  
  $form['id']        = array('#type'        => 'hidden',
                             '#value'       => isset($values['id']) ? $values['id'] : NULL);
  $form['select_id'] = array('#prefix'      => '<h3>SELECT</h3>',
                           '#suffix'        => '',
                           '#type'          => 'textfield', 
                           '#title'         => 'id', 
                           '#default_value' => isset($values['id']) ? $values['select_id'] : NULL, 
                           '#size'          => 40,);
  $form['select_species'] = array('#prefix' => '',
                           '#suffix'        => '',
                           '#type'          => 'textfield', 
                           '#title'         => 'species', 
                           '#default_value' => isset($values['id']) ? $values['select_species'] : NULL, 
                           '#size'          => 40,);
  $form['select_sex'] = array('#prefix'     => '',
                           '#suffix'        => '',
                           '#type'          => 'textfield', 
                           '#title'         => 'sex', 
                           '#default_value' => isset($values['id']) ? $values['select_sex'] : NULL, 
                           '#size'          => 40,);
  foreach ( DataBridge_getAnalysis() as $analysis ) {
    $form['select_' . $analysis['name']] = 
      array('#prefix'        => '',
            '#suffix'        => '',
            '#type'          => 'textfield', 
            '#title'         => $analysis['name'], 
            '#default_value' => isset($values['id']) ? $values['select_' . $analysis['name']] : NULL, 
            '#size'          => 40,);
  }
  $form['from']    = array('#prefix'        => '<h3>FROM</h3>',
                           '#suffix'        => '',
                           '#type'          => 'textarea', 
                           '#title'         => '', 
                           '#default_value' => isset($values['id']) ? $values['from'] : NULL,);
  $form['where_date']    = array('#prefix'        => '<h3>WHERE</h3>',
                           '#suffix'        => '',
                           '#type'          => 'textfield', 
                           '#title'         => '= sampling date', 
                           '#default_value' => isset($values['id']) ? $values['where_date'] : NULL,);
  $form['where_origin']  = array('#prefix'  => '',
                           '#suffix'        => '',
                           '#type'          => 'textfield', 
                           '#title'         => '= sampling origin', 
                           '#default_value' => isset($values['id']) ? $values['where_origin'] : NULL,);
  
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
  }
  if ( !isset($fields['id']) ) {
    DataBridge_insertAnimals($fields);
  } else {
    DataBridge_updateAnimals($fields);
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
  
  drupal_set_message('<pre>' . print_r($queries, true) . '</pre>');
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

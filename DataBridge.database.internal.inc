<?php
// $Id$

/**
 * Copyright 2011 ILRI
 *
 * This file is part of Databridge.
 * 
 * Databridge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Databridge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Databridge. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *   Functions calling the internal database of the Databridge module
 *
 *   This include file implements the functions used by the internal database
 *   interface of the Databridge package.
*/

// TODO: replace i/u/g-Sampling/Animals w. iug-Query since they use the same table.

/**
 * Gets analysis information from the internal database.
 *
 * @param int $species = NULL
 *   If set, only analysis for the given species id are returned.
 *
 * @return array $analysis
 *   An array of analysis information.
*/
function DataBridge_getAnalysis($species = NULL) {
  $query = db_select('databridge_analysis', 'da')
                    ->fields('da', array('id', 'name', 'protocol', 'nr_of_vars', 'description', 'species'));
  if ($species) {
    $query -> condition('species', '%' . $species . '%', 'LIKE');
  }
  $db_data = $query->execute();
  
  $analysis = array();
  while ( $data = $db_data->fetchAssoc() ) {
    $analysis[$data['id']] = $data;
  }
  return $analysis;
}

/**
 * Gets animal query information from the internal database.
 *
 * @return string $query
 *   An animal information extraction query
*/
function DataBridge_getAnimals() {
  $query = db_select('databridge_queries', 'dq')
                   ->fields('dq', array('id', 'fields', 'tables', 'conditions',))
                   ->condition('name', 'animals', '=')
                   ->execute();
  $raw = $query->fetchAssoc();
  $animals = array('id' => $raw['id'],);
  foreach ( explode('|', $raw['fields']) as $field ) {
    if ( preg_match('/(.*) AS (.*)/', $field, $match) ) {
      $animals['select_' . $match[2]] = $match[1];
    }
  }
  if ( $raw['tables'] ) {
    $animals['from'] = $raw['tables'];
  }
  foreach ( explode('|', $raw['conditions']) as $field ) {
    if ( preg_match('/(.*):(.*)/', $field, $match) ) {
      $animals['where_' . $match[2]] = $match[1];
    }
  }
  return $animals;
}

/**
 * Gets all unique species classes from the database
 *
 * @return array $classes
 *   An array of unique species classes.
*/
function DataBridge_getClasses() {
  $db_data = db_select('databridge_species', 'ds')
                      ->fields('ds', array('class'))
                      ->distinct()
                      ->execute();
  $classes = array();
  while ( $data = $db_data->fetchAssoc() ) {
    foreach (explode(',', $data['class']) as $class) {
      if ( !in_array(trim($class), $classes) ) {
        array_push($classes, trim($class));
      }
    }
  }
  foreach ( array_keys($classes) as $class ) {
    if ( !$classes[$class] ) {
      unset($classes[$class]);
    }
  }
  return array_filter($classes);
}

/**
 * Gets sampling information from the internal database.
 *
 * By default returns sampling queries for all sampling types, when given
 * the optional class parameter, only the class (or classes if given an array)
 * matching the parameter is returned.
 *
 * @param string $class
 *   Name of the query in the database
 * @param bool $include_blanks = False
 *   If true, inserts blank fields for all legal values of this query. If false
 *   returns only fields with known values.
 *
 * @return array $samplings
 *   An array of query information
*/
function DataBridge_getSamplings($class = NULL, $include_blanks = False) {
  $default = array('date' => '', 
                   'origin' => '', 
                   'latitude' => '', 
                   'longitude' => '');
  $fields = array();
  $table = array();
  if ($include_blanks) {
    $blanks = array();
  
    // Add blanks for species 
    foreach ( DataBridge_getSpecies($class) as $species ) {
      $name = strtolower($species['common_name']);
      $blanks[$name . '/sampled'] = '';
      $blanks[$name . '/female'] = '';
  
      // Add blanks for species analysis
      foreach ( DataBridge_getAnalysis($species['id']) as $analysis ) {
        $blanks[$name . '/' . $analysis['name']] = '';
      }
    }
    $fields = array_merge($default, $blanks);
  }
  $values = DataBridge_getQuery($class);
  
  foreach (explode('|', $values['fields']) as $field) {
    if (preg_match('/(.*) AS (.*)/',$field, $match)) {
      $fields[$match[2]] = $match[1];
    }
  }
  $values['fields'] = $fields;
  $values['table']  = $table;
  return $values;
}

/**
 * Gets species information from the internal database.
 *
 * @param string $class = NULL
 *   If set, only species that belong to $class is returned.
 *
 * @return array $species
 *   An array of species information.
*/
function DataBridge_getSpecies($class = NULL) {
  $query = db_select('databridge_species', 'ds')
                      ->fields('ds', array('id', 'common_name', 'scientific_name', 'class'));
  if ($class) {
    $query -> condition('class', '%' . $class . '%', 'LIKE');
  }
  $db_data = $query->execute();
  
  $species = array();
  while ( $data = $db_data->fetchAssoc() ) {
    $species[$data['id']] = $data;
  }
  return $species;
}

/**
 * Gets query information from the database about a single query
 *
 * @param string $query
 *   Name of the query in the database
 *
 * @return array $values
 *   Associative array containing query values. 
*/
function DataBridge_getQuery($query) {
  return db_select('databridge_queries', 'd')
                   ->fields('d', array('id', 'fields', 'tables', 'conditions'))
                   ->condition('name', $query, '=')
                   ->execute()
                   ->fetchAssoc();
}

/**
 * insert analysis information to the internal database.
 *
 * @param array $analysis
 *   An array of analysis information arranged as 
 *   ['name' => '', 'protocol' => '', 'nr_of_vars' => '', 'description' => '',
 *   'species' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_insertAnalysis($analysis) {
  $trn = db_transaction();
  try {
    db_insert('databridge_analysis')
              ->fields($analysis)
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * insert animal extraction query information to the internal database.
 *
 * @param array $fields
 *   An array of animal query information arranged as 
 *   ['name' => '', 'fields' => '', 'tables' => '', 'conditions' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_insertAnimals($fields) {
  $trn = db_transaction();
  try {
    db_insert('databridge_queries')
              ->fields($fields)
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * Insert sampling information to the internal database.
 *
 * @param array $fields
 *   An array of sampling information arranged as 
 *   ['name' => '', 'fields' => '', 'tables' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_insertSampling($fields) {
  $trn = db_transaction();
  
  try {
    db_insert('databridge_queries')
              ->fields($fields)
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * insert species information to the internal database.
 *
 * @param array $species
 *   An array of species information arranged as 
 *   ['common_name' => '', 'scientific_name' => '', 'class' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_insertSpecies($species) {
  $trn = db_transaction();
  try {
    db_insert('databridge_species')
              ->fields($species)
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * Update analysis information in the internal database.
 *
 * @param array $analysis
 *   An array of analysis information arranged as 
 *   ['id' => '', 'name' => '', 'protocol' => '', 'nr_of_vars' => '', 
 *   'description' => '', 'species' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_updateAnalysis($analysis) {
  $trn = db_transaction();
  try {
    db_update('databridge_analysis')
              ->fields($analysis)
              ->condition('id', $analysis['id'], '=')
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * Updates animal extraction query information to the internal database.
 *
 * @param array $fields
 *   An array of animal query information arranged as 
 *   ['id' => '', name' => '', 'fields' => '', 'tables' => '', 
 *   'conditions' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_updateAnimals($fields) {
  $trn = db_transaction();
  try {
    db_update('databridge_queries')
              ->fields($fields)
              ->condition('id', $fields['id'], '=')
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * Update sampling information in the internal database.
 *
 * @param array $fields
 *   An array of sampling information arranged as 
 *   ['name' => '', 'fields' => '', 'tables' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_updateSampling($fields) {
  $trn = db_transaction();
  try {
    db_update('databridge_queries')
              ->fields($fields)
              ->condition('id', $fields['id'], '=')
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * update species information in the internal database.
 *
 * @param array $species
 *   An array of species information arranged as 
 *   ['id' => '', common_name' => '', 'scientific_name' => '', 'class' => '']
 *
 * @return bool $success
 *   Returns True on success, False otherwise
*/
function DataBridge_updateSpecies($species) {
  $trn = db_transaction();
  try {
    db_update('databridge_species')
              ->fields($species)
              ->condition('id', $species['id'], '=')
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

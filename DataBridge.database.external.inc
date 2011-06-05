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
 *   Functions used in the database interface of the Databridge module
 *
 *   This include file implements the functions used by the database interface 
 *   of the Databridge package.
*/

/**
 * Class that handles the database connection to the DataBridge database
 *
 * This class depends on having set the connection variables in the module 
 * configuration page.
*/
Class _DataBridgeInterface {
  var $user;
  var $pass;
  var $name;
  var $host;
  var $db;
  var $error;
  
  /**
   * Constructor for the _DataBridgeInterface.
   *
   * The constructor assigns the variables set in the drupal configuration page
   * to local variables.
  */
  public function __construct() {
    $this->user = variable_get('DataBridge_dbuser', 'databridge');
    $this->pass = variable_get('DataBridge_dbpass', '');
    $this->name = variable_get('DataBridge_dbname', 'databridge');
    $this->host = variable_get('DataBridge_dbhost', 'localhost');
  }
  
  /**
   * Opens the database connection
  */
  public function open() {
    if ( !($this->db = mysql_connect($this->host, $this->user, $this->pass)) ) {
      $this->error = mysql_error();
      return False;
    } elseif ( !mysql_select_db($this->name, $this->db) ) {
      $this->error = mysql_error();
      return False;
    }
    return True;
  }
  
  /**
   * Closes the database connection
  */
  public function close() {
    if ($this->db) {
      mysql_close($this->db);
    }
  }
  
  /**
   * Performs a query in the database and returns the results.
   *
   * This function performs a query in the DataBridge database and returns 
   * the query results in an array.
   *
   * @param string $query
   *   The query to be executed in the database.
   *
   * @return array $results
   *   An array containing the query results.
  */
  public function query($query) {
    $results = array();
    if ($this->db) {
      $query_results = mysql_query($query, $this->db);
      while ($query_results and $result = mysql_fetch_assoc($query_results)) {
        array_push($results, $result);
      }
      
    } else {
      _DataBridge_Message('NotConnected');
    }
    return $results;
  }
  
}

/**
 * Gets animal information from the external database.
 *
 * By default returns animal information for a single animal. If the id
 * parameter is omitted the functions sets an error message and defaults to the
 * summary page.
 *
 * @param $id = NULL
 *   Animal id value.
 *
 * @return array $data
 *   An array of animal information.
*/
function DataBridge_getAnimalInfo($id = NULL) {
  $raw = DataBridge_getQuery('animals');
  
  // Get the "id" from the normal fields
  $id_query = strstr($raw['fields'], '|', true) . ",\n";
  
  // Get the rest of the normal fields
  $data = substr(strstr(strstr($raw['fields'], ':', true), '|'), 1);
  $query = "SELECT " . $id_query . implode(",\n\t", explode('|', $data) );
  
  // Get the "details" part of the query
  if ( $data = substr(strstr($raw['fields'], ':'), 1) ) { 
    $query .= ",\n" . implode(",\n\t", explode('|', $data) );
  }
  
  // Add FROM part of the query
  $query .= "\nFROM " . $raw['tables'];

  // Add WHERE part of the query
  $where = "\nWHERE ";
  $where .= strstr($id_query, ' ', true) . ' = "' . $id . '"';
  $query .= $where;
  
  // Add a LIMIT 1 just in case
  $query .= "\n" . 'LIMIT 1';
  
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $data = $db->query($query);
    if ( $data ) {
      $data = $data[0];
    }
  }
  
  $data['samples'] = DataBridge_getSampleList($data['id']);
  
  return $data;
}

/**
 * Gets animal information from the external database.
 *
 * By default returns animal information for all animals. When given one or 
 * more of the optional parameters, only the animal/s fitting the filter will
 * be returned. All options can be given either as strings or arrays.
 *
 * @param $origin = NULL
 *   String or array of strings with animal origins.
 * @param $date = NULL
 *   String or array of strings with sampling dates.
 *
 * @return array $samplings
 *   An array of arrays of animal information.
*/
function DataBridge_getAnimalList($origin = NULL, $date = NULL) {
  $raw = DataBridge_getQuery('animals');
  $fields = strpos($raw['fields'], ':') ? strstr($raw['fields'], ':', true) : $raw['fields'];
  $query  = "SELECT " . implode(",\n\t", explode('|', $fields));
  $query .= "\nFROM " . $raw['tables'];
  $where = '';
  if ( ($origin or $date) and isset($raw['conditions']) ) {
    $conditions = array();
    foreach ( explode('|', $raw['conditions']) as $condition ) {
      if ( preg_match('/(.*):(.*)/', $condition, $match)) {
        $conditions[$match[2]] = $match[1];
      }
    }
    
    if ( $origin and isset($conditions['origin']) ) {
      if ( !$where ) {
        $where = "\nWHERE ";
      } else {
        $where .= "\nAND ";
      }
      if ( is_array($origin) ) {
        $where .= $conditions['origin'] . ' IN ("' . implode('", "', $origin) . '")';
      } else {
        $where .= $conditions['origin'] . ' = "' . $origin . '"';
      }
    }
    if ( $date and isset($conditions['origin']) ) {
      if ( !$where ) {
        $where = "\nWHERE ";
      } else {
        $where .= "\nAND ";
      }
      if ( is_array($date) ) {
        $where .= $conditions['date'] . ' IN ("' . implode('", "', $date) . '")';
      } else {
        $where .= $conditions['date'] . ' = "' . $date . '"';
      }
    }
  }
  $query .= $where;
  $query .= "\n" . 'GROUP BY id';
  
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $data = $db->query($query);
  }
  
  return $data;
}

/**
 * Gets sample information from the external database.
 *
 * By default returns sample information for a single sample. If the id
 * parameter is omitted the functions sets an error message and defaults to the
 * summary page.
 *
 * @param $id = NULL
 *   Sample id value.
 *
 * @return array $data
 *   An array of sample information.
*/
function DataBridge_getSampleInfo($id = NULL) {
  $raw = DataBridge_getQuery('samples');
  
  // Get the "id" from the normal fields
  $id_query = strstr($raw['fields'], '|', true) . ",\n";
  
  // Get the rest of the normal fields
  $normal = substr(strstr(strstr($raw['fields'], ':', true), '|'), 1);
  $query = "SELECT " . $id_query . implode(",\n\t", explode('|', $normal) );
  
  // Get the "details" part of the query
  if ( $details = substr(strstr($raw['fields'], ':'), 1) ) { 
    $query .= ",\n" . implode(",\n\t", explode('|', $details) );
  }
  
  // Add FROM part of the query
  $query .= "\nFROM " . $raw['tables'];

  // Add WHERE part of the query
  $where = "\nWHERE ";
  $where .= strstr($id_query, ' ', true) . ' = "' . $id . '"';
  $query .= $where;
  
  // Add a LIMIT 1 just in case
  $query .= "\n" . 'LIMIT 1';
  
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $data = $db->query($query);
    if ( $data ) {
      $data = $data[0];
      
      // Add sibling samples if we have a parent
      if ( preg_match('/\|([^|]*) AS parent/', $raw['fields'], $match) ) { 
        $query = 'SELECT ' . $id_query . implode(",\n\t", explode('|', $normal) ) . "\n" . 
                 'FROM ' . $raw['tables'] . "\n" . 
                 'WHERE ' . $match[1] . ' = "' . $data['parent'] . '"';
        $data['siblings'] = array();
        foreach ( $db->query($query) as $sibling ) {
          if ( $sibling['id'] != $id ) {
            array_push($data['siblings'], $sibling);
          }
        }
      }
    }
  }
  
  return $data;
}

/**
 * Gets sample information from the external database.
 *
 * By default returns sample information for all samples. When given the 
 * optional animal_id parameter, only the sample/s from this animal will be 
 * returned. animal_id can be given either as a string or an array.
 *
 * @param $animal_id = NULL
 *   String or array of strings with animal IDs.
 *
 * @return array $samplings
 *   An array of arrays of animal information.
*/
function DataBridge_getSampleList($animal_id = NULL) {
  $raw = DataBridge_getQuery('samples');
  
  $fields = strpos($raw['fields'], ':') ? strstr($raw['fields'], ':', true) : $raw['fields'];
  $query  = "SELECT " . implode(",\n\t", explode('|', $fields));
  $query .= "\nFROM " . $raw['tables'];
  $where = '';
  
  if ( $animal_id and isset($raw['conditions']) ) {
    $conditions = array();
    foreach ( explode('|', $raw['conditions']) as $condition ) {
      if ( preg_match('/(.*):(.*)/', $condition, $match)) {
        $conditions[$match[2]] = $match[1];
      }
    }
    if ( $animal_id and isset($conditions['animal']) ) {
      if ( !$where ) {
        $where = "\nWHERE ";
      } else {
        $where .= "\nAND ";
      }
      if ( is_array($animal_id) ) {
        $where .= $conditions['animal'] . ' IN ("' . implode('", "', $animal_id) . '")';
      } else {
        $where .= $conditions['animal'] . ' = "' . $animal_id . '"';
      }
    }
  }
  $query .= $where;
  
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $data = $db->query($query);
  }
  return $data;
}

/**
 * Gets sampling information from the external database.
 *
 * By default returns sampling information for all sampling types, when given
 * the optional class parameter, only the class (or classes if given an array)
 * matching the parameter is returned.
 *
 * @param $classes = NULL
 *   String or array of strings with classes to return.
 * @param $cluster = true
 *   If true, the samples will be clustered using geocoding and time.
 * @param $min_clustering_time = 24
 *   The minimum time in hours between samplings to count them as a single 
 *   sampling trip.
 *
 * @return array $samplings
 *   An array of arrays of sampling information.
*/
function DataBridge_getSamplingInfo($classes = NULL, $cluster = true, $min_clustering_time = 24) {
  
  // Get the classes sorted out
  if ($classes) {
    if ( !is_array($classes) ) {
      $classes = array($classes);
    }
  } else {
    $classes = DataBridge_getClasses();
  }
  
  // Extract the queries from the internal database
  $queries = array();
  foreach ( $classes as $class ) {
    $query = DataBridge_getQuery($class);
    $fields = implode(", \n\t", explode('|', $query['fields']));
    $from = $query['tables'];
    $queries[$class] = "SELECT " . $fields . "\nFROM " . $from . "\nGROUP BY TO_DAYS(date), origin ORDER BY date;";
  }
  
  // Execute the queries in the external database 
  $data = array();
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    foreach ( $queries as $name => $query ) {
      $data[$name] = $db->query($query);
    }
  }
  
  // Re-cluster the results with geocoding and sampling dates
  $sorted = array();
  foreach ( $data as $name => $type ) {
    $sorted[$name] = array();
    foreach ( $type as $trip ) {
      $geo_info = DataBridge_geocode($trip['latitude'], $trip['longitude']);
      $location = DataBridge_rfind($geo_info, 'SubAdministrativeAreaName');
      if ( !$location ) {
        $location = DataBridge_rfind($geo_info, 'AdministrativeAreaName');
      }
      
      $found = false;
      foreach ($sorted[$name] as $key => $sampling) {
        if ($location == $sampling['location'] 
            and $trip['date'] <= $sampling['end_date'] + $min_clustering_time*3600) {
              
          array_push($sorted[$name][$key]['sublocations'], $trip);
          
          $sorted[$name][$key]['end_date'] = $trip['date'];
          foreach ($trip as $index => $value) {
            if ( !in_array($index, array('origin', 'date',)) ) {
              $sorted[$name][$key][$index] += $value;
            }
          }
          
          $found = true;
          
        }
      } 
      if ( !$found ) {
        $new = array('start_date'   => $trip['date'],
                     'end_date'     => $trip['date'],
                     'location'     => $location,
                     'sublocations' => array($trip),);
        foreach ($trip as $key => $value) {
          if ( !in_array($key, array('origin', 'date',)) ) {
            $new[$key] = $value;
          }
        }
        array_push($sorted[$name], $new);
      }
    }  
    // Calculate gps mid-point
    foreach ($sorted[$name] as $key => $sampling) {
      $sorted[$name][$key]['latitude'] /= count($sorted[$name][$key]['sublocations']);
      $sorted[$name][$key]['longitude'] /= count($sorted[$name][$key]['sublocations']);
    }
  }
  
  return $sorted;
}

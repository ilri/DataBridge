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
      while ($query_results and $result = mysql_fetch_row($query_results)) {
        array_push($results, $result);
      }
      
    } else {
      _DataBridge_Message('NotConnected');
    }
    return $results;
  }
  
}

/**
 * Gets a list of animals from the external database.
 *
 * @param int $start = 0
 *   The first animal to be listed
 * @param int $nr_of_items = 2000000
 *   The number of items to return
 *
 * @return array $animals
 *   An array of animals
*/
function Databridge_getAnimals($start = 0, $nr_of_items = 2000000) {
  $db = new _DataBridgeInterface();
  $query = "SELECT a.animal_id, o.org_name, a.sex, a.location, a.owner, a.status 
            FROM custom_animals a 
            JOIN organisms o ON o.org_id = a.organism
            ORDER BY a.animal_id
            LIMIT $start, $nr_of_items;";
  if ($db->open()) {
    return $db->query($query);
  }
  return array();
}

/**
 * Returns information about an animal from an animal id.
 *
 * @param int $id
 *   The id of the animal to extract information about.
 *
 * @return array $info
 *   An associative array containing animal information
*/
function DataBridge_getAnimal($id) {
  $db = new _DataBridgeInterface();
  $info = array('id' => NULL,
                'previous_tag' => NULL,
                'organism' => NULL,
                'dob' => NULL,
                'sex' => NULL,
                'location' => NULL,
                'owner' => NULL,
                'status' => NULL,);
  
  $query = "SELECT ca.animal_id, 
                   ca.prev_tag, 
                   o.org_name, 
                   ca.approx_dob, 
                   ca.sex, 
                   ca.location, 
                   ca.owner, 
                   ca.status 
            FROM custom_animals ca 
            JOIN organisms o ON ca.organism = o.org_id 
            WHERE animal_id = '" . $id . "';";
  $sample_query = "SELECT s.label, UNIX_TIMESTAMP(s.date_created), et.STATUS, 
                          (100 - (et.OD1 - et.OD2)) / (es.meanControlOD*100) inhib, 
                          s.TrayID, s.box_details, s.Longitude, s.Latitude,
                          IF ( SUBSTR(s.TrayID, 5,1) = 'L' OR SUBSTR(TrayID, 5,1) = 'W', 'ILRI', 
                            IF ( SUBSTR(s.TrayID, 5,1) = 'R', 'KARI', 
                            IF ( SUBSTR(s.TrayID, 5,1) = 'A', 'DVS', 
                            IF ( SUBSTR(s.TrayID, 5,1) = 'C', 'ICIPE', 
                            IF (SUBSTR(s.TrayID, 5,1) = 'M', 'KEMRI', 'Unknown'))))) inst
                   FROM samples s 
                   LEFT OUTER JOIN elisaTest et ON et.sampleId = s.count 
                   LEFT OUTER JOIN elisaSetUp es ON et.testID = es.testID
                   WHERE s.AnimalID = '" . $id . "';";
  
  if ($db->open()) {
    if ( $result = $db->query($query) ) {
      $info = array('id' => $result[0][0],
                    'previous_tag' => $result[0][1],
                    'organism' => $result[0][2],
                    'dob' => $result[0][3],
                    'sex' => $result[0][4],
                    'location' => $result[0][5],
                    'owner' => $result[0][6],
                    'status' => $result[0][7],);
    }
    $samples = array();
    $locations = array();
    foreach ( $db->query($sample_query) as $sample) {
      array_push($samples, array('id' => $sample[0],
                                 'date' => strftime('%Y-%m-%d', $sample[1]),
                                 'status' => $sample[2],
                                 'var' => $sample[3],
                                 'locale' => $sample[4] . ':' . $sample[5],
                                 'location' => array($sample[7], $sample[6]), 
                                 ));
      array_push($locations, array($sample[7], $sample[6]));
    }
    $info['samples'] = $samples;
    $info['map'] = _GoogleStaticMap($locations, 348, 210);
  }
  return $info;
}

/**
 * Gets samples positive to a specific analysis from the external database.
 *
 * @param string $type
 *   The type of analysis results to return
 *
 * @return array $samples
 * 
*/
function DataBridge_getPositive($type = 'elisa') {
  switch ($type) {
    case 'elisa': $query = 'SELECT s.label,
                                   s.AnimalID, 
                                   s.origin, 
                                   UNIX_TIMESTAMP(s.VisitDate)
                            FROM samples s 
                            JOIN elisaTest et ON s.count = et.sampleID
                            WHERE s.project = 5
                              AND et.STATUS = "POSITIVE"
                            GROUP BY s.AnimalID;'; break;
    default: return array();
  }  
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    return $db->query($query);
  }
  return array();
}

/**
 * Gets sampling trips from the external database.
 *
 * Extracts query information from the internal database to query the external
 * database and return an array of sampling trip information.
 *
 * @return array $samplings
 *   An array of sampling trips.
*/
function Databridge_getSamplingTrips() {
  $values = db_select('databridge_queries', 'd')
                      ->fields('d', array('fields', 'tables'))
                      ->condition('name', 'sampling_trips', '=')
                      ->execute()
                      ->fetchAssoc();
  $temp = DataBridge_get('sampling_trips');
  $link = $temp['fields']['date'] . ', "." , ' . $temp['fields']['origin'];
  $query = 'SELECT ' . str_replace('|', ', ', $values['fields']) . 
           ' CONCAT(' . $link . ') AS link' . 
           ' FROM ' . $values['tables'] . 
           ' GROUP BY TO_DAYS(date), origin;';
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    return $db->query($query);
  }
  return array();
}

/**
 * Gets information about a sampling trip
 *
 * @param string $location
 *   Name of the sampling location
 * @param string $date
 *   Date of the sampling trip
 *
 * @return array $data
 *   An associative array of sampling trip data
*/
function DataBridge_getSampling($location, $date) {
  $db = new _DataBridgeInterface();
  $data = array();
  $query = "SELECT s.Longitude, 
                   s.Latitude, 
                   s.AnimalID, 
                   ca.sex, 
                   ca.status, 
                   o.org_name, 
                   et.STATUS
            FROM samples s 
            JOIN custom_animals ca ON s.AnimalID = ca.animal_id
            JOIN organisms o ON o.org_id = ca.organism
            LEFT JOIN elisaTest et ON s.count = et.sampleId
            WHERE s.origin = '$location' 
              AND s.VisitDate = '$date' 
            GROUP BY s.AnimalID;";
  if ($db->open()) {
    $data = $db->query($query);
  }
  return $data;
}

/**
 * Gets wildlife sampling trips from the external database.
 *
 * Extracts query information from the internal database to query the external
 * database and return an array of wildlife sampling trip information.
 *
 * @return array $samplings
 *   An array of wildlife sampling trips.
*/
function Databridge_getWildlifeTrips() {
  $values = db_select('databridge_queries', 'd')
                      ->fields('d', array('fields', 'tables'))
                      ->condition('name', 'wildlife_trips', '=')
                      ->execute()
                      ->fetchAssoc();
  $query = 'SELECT ' . rtrim(str_replace('|', ', ', $values['fields']), ', ') . 
           ' FROM ' . $values['tables'] . 
           ' GROUP BY TO_DAYS(date), origin;';
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    return $db->query($query);
  }
  return array();
}

/**
 * Gets a list of all samples from the database.
 *
 * @param int $first = 0
 *   First id to return.
 * @param int $nr_of_items = 50
 *   Number of units to return
 *   
 * @return array $animals
 *   A list of all samples in the database.
*/
function DataBridge_getSamples($first = 0, $nr_of_items = 50) {
  $samples = array();
  $db = new _DataBridgeInterface();
  $query = "SELECT s.label, UNIX_TIMESTAMP(s.date_created), et.STATUS, 
                   (100 - (et.OD1 - et.OD2)) / (es.meanControlOD*100) inhib, 
                   s.TrayID, s.box_details, s.Longitude, s.Latitude,
                   IF ( SUBSTR(s.TrayID, 5,1) = 'L' OR SUBSTR(TrayID, 5,1) = 'W', 'ILRI', 
                     IF ( SUBSTR(s.TrayID, 5,1) = 'R', 'KARI', 
                     IF ( SUBSTR(s.TrayID, 5,1) = 'A', 'DVS', 
                     IF ( SUBSTR(s.TrayID, 5,1) = 'C', 'ICIPE', 
                     IF (SUBSTR(s.TrayID, 5,1) = 'M', 'KEMRI', 'Unknown'))))) institute
            FROM samples s 
            LEFT OUTER JOIN elisaTest et ON et.sampleId = s.count 
            LEFT OUTER JOIN elisaSetUp es ON et.testID = es.testID
            WHERE s.AnimalID LIKE 'AVD%'
            ORDER BY s.VisitDate DESC
            LIMIT $first, $nr_of_items";

  if ($db->open()) {
    $samples = $db->query($query);
  }
  return $samples;
}

/**
 * Gets information about a single sample in the external database.
 * 
 * @param int $id
 *   Id of the sample in the database.
 *
 * @return array $data
 *   An array of information about the sample.
*/
function DataBridge_getSample($id = NULL) {
  $data = array();
  $db = new _DataBridgeInterface();
  $query = "SELECT s.label,  s.comments, s.date_created, s.date_updated, s.sample_type, 
                   s.origin, s.org,      s.volume,       s.volume_unit,  s.unique_code, 
                   s.main_operator,      s.box_id,       s.box_details,  s.wait, 
                   s.secret, s.Project,  s.SSID,         s.StoreLabel,   s.SampleID,
                   s.VisitID,            UNIX_TIMESTAMP(s.VisitDate),    s.AnimalID,     
                   s.Visit,  s.ShippedDate,              s.TrayID,       s.DateRecorded, 
                   s.Longitude,          s.Latitude,     et.STATUS
            FROM samples s
            LEFT JOIN elisaTest et ON et.sampleID = s.count
            WHERE s.label = '$id'; 
            ";
  if ($db->open()) {
    $data = $db->query($query);
    // Add map
    array_push($data, _GoogleStaticMap(array(array($data[0][27], $data[0][26])), 348, 210));
    // Add parent sampling trip
    array_push($data, array($data[0][5], strftime('%Y-%m-%d', $data[0][20])));
    // Add sibling samples
    preg_match('/SourceTube=[A-Z0-9]+/', $data[0][1], $parent);
    $query = "SELECT s.label, UNIX_TIMESTAMP(s.date_created), et.STATUS, 
                     (100 - (et.OD1 - et.OD2)) / (es.meanControlOD*100) inhib, 
                     s.TrayID, s.box_details, s.Longitude, s.Latitude,
                     IF ( SUBSTR(s.TrayID, 5,1) = 'L' OR SUBSTR(TrayID, 5,1) = 'W', 'ILRI', 
                       IF ( SUBSTR(s.TrayID, 5,1) = 'R', 'KARI', 
                       IF ( SUBSTR(s.TrayID, 5,1) = 'A', 'DVS', 
                       IF ( SUBSTR(s.TrayID, 5,1) = 'C', 'ICIPE', 
                       IF (SUBSTR(s.TrayID, 5,1) = 'M', 'KEMRI', 'Unknown'))))) inst
              FROM samples s 
              LEFT OUTER JOIN elisaTest et ON et.sampleId = s.count 
              LEFT OUTER JOIN elisaSetUp es ON et.testID = es.testID
              WHERE comments LIKE '%$parent[0]%'
                AND label != '" . $data[0][0] . "'
              ORDER BY s.date_created DESC;";
    $siblings = array();
    foreach ( $db->query($query) as $sibling) {
      array_push($siblings, array('id' => $sibling[0],
                                  'date' => strftime('%Y-%m-%d', $sibling[1]),
                                  'status' => $sibling[2],
                                  'var' => $sibling[3],
                                  'locale' => $sibling[4] . ':' . $sibling[5],
                                  'location' => array($sibling[7], $sibling[6]), 
                                  ));
    }
    array_push($data, $siblings);
  }
  return $data;
}

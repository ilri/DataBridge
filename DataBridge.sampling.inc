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
 *   Functions used in the sampling view by the Databridge module
 *
 *   This include file implements the functions used by the animal view of the 
 *   Databridge package.
*/

/**
 * Creates an array of events for each livestock sampling
 * 
 * @return array(array) $events
 *   An array of associative arrays containing the events
*/
function DataBridge_SamplingTrips() {
  $events = array();
  foreach (Databridge_getSamplingTrips() as $result) {
    $event = array();
    $event['name']       = $result[1];
    $event['start_date'] = $result[0];
    $event['end_date']   = NULL;
    $event['map']        = _GoogleStaticMap(array(array($result[2], $result[3])));
    $event['latitude']   = $result[2];
    $event['longitude']  = $result[3];
    $event['private']    = user_access('access DataBridge private') 
                           ? '<a href="' . url('databridge/private/sampling/' . str_replace('.', '/', $result[14]))  . '">&gt; Download Summary</a>' 
                           : '';
    $event['open']       = theme('table', 
                                 array('header' => array(array('data' => $result[4] . ' animals sampled', 'colspan' => 3),),
                                       'rows'   => array(array('Sheep',  $result[7] . ' F', ($result[4] - $result[5] - $result[7]) . ' M',),
                                                         array('Goats',  $result[6] . ' F', ($result[5] - $result[6]             ) . ' M',),)
                                 )) . 
                           theme('table', 
                                 array('header' => array(array('data' => 'Animal Status', 'colspan' => 4),),
                                       'rows'   => array(array('', 'Alive', 'Dead', 'Unacc.'),
                                                         array('Sheep', $result[10], $result[11], ($result[4] - $result[5]) - ($result[10] + $result[11])),
                                                         array('Goats', $result[8], $result[9], $result[5] - $result[8] - $result[9])))) . 
                           theme('table', 
                                 array('header' => array(array('data' => 'Elisa Results', 'colspan' => 3),),
                                       'rows'   => array(array('', 'Positive', 'Negative'),
                                                         array('Sheep', $result[12], ($result[4] - $result[5]) - $result[12]),
                                                         array('Goats', $result[13], $result[5] - $result[13]))));
    $event['link']       = user_access('access DataBridge private') ? l('View Details', '/databridge/sampling/' . $result[1] . '/' . strftime('%Y-%m-%d', $result[0])) : '';
    array_push($events, $event);
  }
  return $events;
}

/**
 * Returns a summary of a SamplingTrip defined by date and location.
 *
 * @param int $date
 *   The date in days
 * @param string $location
 *   The name of the location 
 *
 * @return array(array) $data
 *   An array of arrays representing a spreadsheet of information.
*/
function DataBridge_SamplingTrip($sampling_day, $location) {
  // Variables 
  $db = new _DataBridgeInterface();
  $header = array("Animal ID", "Location", "Species", "Status", "Sex", "Owner", );
  $last_date = NULL;
  $location = str_replace('_', ' ', $location);  
  
  // Queries, some don't make much sense 
  $get_earlier = "SELECT TO_DAYS(VisitDate) days, 
                         DATE_FORMAT(VisitDate, '%Y-%m-%d') VisitDate 
                  FROM samples 
                  WHERE AnimalID LIKE 'AVD%' 
                    AND project = 5 
                    AND VisitDate != '0000-00-00' 
                    AND TO_DAYS(VisitDate) <= " . $sampling_day . " 
                    AND origin = '" . $location . "' 
                  GROUP BY TO_DAYS(VisitDate)
                  ORDER BY TO_DAYS(VisitDate) ASC;";  
  
  if ($db->open()) {
    // Get earlier sampling trips to the same location.
    $earlier = array();
    foreach ($db->query($get_earlier) as $result) {
      array_push($earlier, $result[0]);
      array_push($header, $result[1]);
      $last_date = $result[1];
    }
    
    // Defining the rest of the queries and variables now that we know the 
    // history
    $sheet = array(0 => $header);
    $where     = "WHERE s.AnimalID LIKE 'AVD%' 
                    AND s.project = 5 
                    AND TO_DAYS(s.VisitDate) IN (".implode(', ', $earlier).") 
                    AND s.origin = '" . $location . "' 
                  GROUP BY s.AnimalID ORDER BY s.AnimalID;";
    $from      = "FROM samples s 
                       JOIN custom_animals ca ON s.AnimalID = ca.animal_id 
                       JOIN organisms o ON ca.organism = o.org_id 
                       LEFT JOIN elisaTest et ON et.sampleID = s.count"; 
    $base_info = "SELECT s.AnimalID, 
                         s.origin, 
                         o.org_name, 
                         ca.status, 
                         ca.sex, 
                         ca.owner " . $from . " " . $where;
    $get_elisa = "IF(SUM(IF(et.STATUS = 'POSITIVE', 1, 0)), '+', IF(SUM(IF(et.STATUS = 'NEGATIVE', 1, 0)), '-', '')) 'Elisa' ";
    
    // fill out the sheet with base information
    foreach ($db->query($base_info) as $result) {
      array_push($sheet, $result);
    }
    
    $length = count($sheet[2]);
    foreach($earlier as $date) {
      $query = "SELECT s.AnimalID, " . $get_elisa . " " . $from . " 
                WHERE s.AnimalID LIKE 'AVD%' 
                  AND s.project = 5 
                  AND TO_DAYS(s.VisitDate) = " . $date . "
                  GROUP BY s.AnimalID;";
      foreach ($db->query($query) as $result) {
        for ($i = 0; $i < count($sheet); $i++) {
          if ($sheet[$i][0] == $result[0]) {
            array_push($sheet[$i], $result[1]);
            break;
          }
        }
      }  
      _balance_sheet($sheet, ++$length);
    } 
    $length = count($sheet[2]);
  }
  return array(str_replace(' ', '_', $location) . '_' . $last_date . '.xls', $sheet);
}

/**
 * Creates a sheet summary of all livestock sampling.
 *
 * This function creates an array of arrays in the format rows[cols] containing
 * all livestock sampling information that is stored until now in the database.
 *
 * @return array $sheet
 *  An array of arrays containing the information. Format: rows[cols].
*/
function DataBridge_SamplingSummary($start_day = 0, $end_day = 20000000, $locations = 'all') {
  $sheet = array();
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $location_query = 'SELECT DISTINCT origin 
                       FROM samples 
                       WHERE project = 5 
                         AND AnimalID LIKE "AVD%" 
                         AND origin != "";';
    $locations = array();
    foreach ($db->query($location_query) as $result) {
      array_push($locations, "'" . $result[0] . "'");
    }
    $from = "samples s JOIN custom_animals ca ON s.AnimalID = ca.animal_id LEFT JOIN elisaTest et ON et.sampleID = s.count LEFT JOIN organisms o ON ca.organism = o.org_id "; // Organism added if requested later.
    $where = "s.project = 5 AND s.AnimalID LIKE 'AVD%' AND TO_DAYS(s.VisitDate) >= ".$start_day." AND TO_DAYS(s.VisitDate) <= ".$end_day." AND s.origin IN (".implode(', ', $locations).")";
    $elisa = "IF(SUM(IF(et.STATUS = 'POSITIVE', 1, 0)), '+', IF(SUM(IF(et.STATUS = 'NEGATIVE', 1, 0)), '-', '')) 'Elisa' ";
    $dates = "SELECT DATE_FORMAT(s.VisitDate, '%Y-%m-%d') VisitDate, TO_DAYS(s.VisitDate), s.origin days FROM ".$from." WHERE VisitDate != '0000-00-00' AND ".$where." GROUP BY TO_DAYS(s.VisitDate);";
    $animals = 'SELECT DISTINCT s.AnimalID, o.org_name FROM '.$from.' WHERE '.$where.';';
    
    $header = array('Animal ID', 'organism');
    $pre_header = array();
    foreach($header as $nothing) {
      array_push($pre_header, '');
    }
    array_pop($pre_header);
    array_push($pre_header, 'Elisa Results');
    $sheet = array($pre_header, $header);

    foreach ($db->query($animals) as $result) {
      array_push($sheet, $result);
    }
    
    $length = count($sheet[1]);
    foreach ($db->query($dates) as $result) {
      array_push($sheet[1], $result[0]);
      array_push($sheet[0], $result[2]);
      $elisa_query = 'SELECT s.AnimalID, '.$elisa.' FROM '.$from.' WHERE TO_DAYS(s.VisitDate) = '.$result[1].' AND '.$where.' GROUP BY s.AnimalID;';
      foreach ($db->query($elisa_query) as $elisa_result) {
        for ($i = 0; $i < count($sheet); $i++) {
          if ($sheet[$i][0] == $elisa_result[0]) {
            array_push($sheet[$i], $elisa_result[1]);
            break;
          }
        }
      }
      _balance_sheet($sheet, ++$length);
    }
  }
  return $sheet;
}

/**
 * Creates an array of events for each wildlife sampling
 * 
 * @return array(array) $events
 *   An array of associative arrays containing the events
*/
function DataBridge_WildlifeTrips() {
  $events = array();
  
  foreach (Databridge_getWildlifeTrips() as $result) {
    $event = array();
    $event['name']       = $result[1];
    $event['start_date'] = $result[0];
    $event['end_date']   = NULL;
    $event['map']        = _GoogleStaticMap(array(array($result[2], $result[3])));
    $event['latitude']   = $result[2];
    $event['longitude']  = $result[3];
    $event['private']    = user_access('access DataBridge private') ? '[download_link]' : '';
    $event['open']       = 'Wildlife sampling trip. ' . $result[4] . ' samples collected.';
    array_push($events, $event);
  }
  
  return $events;
}

/**
 * Lists sampling trips from the database.
 *
 * Creates a list of sampling trips from the database.
 *
 * @param int $start = 0
 *   The first trip to be listed
 * @param int $nr_of_items = 50
 *   The number of items to view on each page.
 *
 * @return string $html
 *   An html page with an sampling trip list.
*/
function DataBridge_samplings($start = 0, $nr_of_items = 50) {
  $page_content  = '';
  $trips = array();
  foreach (DataBridge_SamplingTrips($start, $nr_of_items) as $trip) {
    array_push($trips, array(l(strftime('%Y-%m-%d', $trip['start_date']), 'databridge/sampling/' . $trip['name'] . '/' . strftime('%Y-%m-%d', $trip['start_date'])), $trip['name'],));
  }
  
  $page_content .= DataBridge_listcontrol('sampling', $start, $nr_of_items);
  $page_content .= '<div id="databridge_samplinglist">';
  $page_content .= theme('table', array('header' => array('Date', 'Location',), 
                                        'rows' => $trips));
  $page_content .= '</div>';
  $page_content .= DataBridge_listcontrol('sampling', $start, $nr_of_items);
  return $page_content;
}

/**
 * Displays information about a sampling trip, it's animals and samples.
 *
 * Creates a simple information page about a single sampling trip. The page has
 * three parts, a general static information div, a static google map of sampling 
 * locations and lastly a dynamic list of collected samples.
 *
 * @param int $id = NULL
 *   Database id of the animal to be viewed.
 *
 * @return string $html
 *   An html page to be returned by drupal.
*/
function DataBridge_samplingview($location = NULL, $date = NULL) {
  if ($location == NULL or $date == NULL) {
    return DataBridge_samplings();
  }
  $data = DataBridge_getSampling($location, $date);
  $animals = array();
  foreach ($data as $animal) {
    switch ($animal[6]) {
      case "POSITIVE": $elisa = '<span class="databridge_warning">POSITIVE</span>'; break;
      case "NEGATIVE": $elisa = '<span class="databridge_ok">NEGATIVE</span>';      break;
      default:         $elisa = '';
    }
    array_push($animals, array(l($animal[2], 'databridge/animal/' . $animal[2]), 
                         $animal[5], 
                         $animal[3], 
                         $animal[4],
                         $elisa,));
  }
  $page_content  = '';
  $page_content .= '<h3>Sampling Trip to ' . $location . ' on ' . $date . '</h3>';
  $page_content .= theme('table', array('header' => array('Animal', 'Organism', 'Sex', 'Status', 'ELISA'),
                                        'rows' => $animals,
                                       ));
  return $page_content;
}

/**
 * Displays static animal information about an animal in the database.
 *
 * @param array $data
 *   An array contatining the data to be displayed.
 *
 * @return string $html
 *   The reurned html.
*/
function DataBridge_samplingview_static($data) {

}


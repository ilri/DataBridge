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
 * along with DataBridge. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *   Functions used in the summary view by the DataBridge module
 *
 *   This include file implements the functions used by the summary view of
 *   the DataBridge package.
*/

/**
 * Returns a summary overview of all information in the database.
 * 
 * Creates an overview with summary information about all sampling trips, all
 * samples, all animals, etc.
 *
 * @return string $html
 *   A summary page.
*/
function DataBridge_summary() {
  $page_content  = '';
  
  // Print summaries of all classes of sampling trips
  $page_content .= DataBridge_summary_sampling();
  
  
  //$page_content .= '<div class="databridge_analysis">' . 
  //                 '<div class="databridge_header">Elisa Positive Samples</div>';                 
  //$page_content .= '<div class="databridge_list">' . 
  //                 theme('table', array('header' => array('Sample', 'Animal', 'Location', 'Date',),
  //                                      'rows'   => DataBridge_positive_samples(),));
  //$page_content .= '</div>';                                     
  //$page_content .= '</div>';
  return $page_content;
}

/**
 * Displays detailed info about a sampling trip.
 *
 * By default this function returns a list of all sampling trips, when given 
 * the three optional parameters class, location and date it instead provides
 * detailed information about the given trip. Note that the date in question 
 * is the start date of the sampling trip.
 *
 * @param string $class = NULL
 *   Sampling trip class, as set in the config/species page.
 * @param string $location = NULL
 *   Sampling trip location, as returned by DataBridge_getSamplingInfo()
 * @param string $date = NULL
 *
 * @return string $html
 *   Html formatted sampling information
*/
function DataBridge_summary_sampling($class = NULL, $location = NULL, $date = NULL) {
  $raw = DataBridge_getSamplingInfo();
  $page_content = '';
  $data = NULL;
  
  // Check if we can get the data that we need from the database.
  if ( ($class != NULL) and ($location != NULL) and ($date != NULL) ) {
    foreach ( $raw[$class] as $trip ) {
      if ( $trip['location'] == $location and strftime('%Y-%m-%d', $trip['start_date']) == $date ) {
        $data = $trip;
        break;
      }
    }
  }
  
  // If the data was found, present it in a neat manner. 
  if ( $data ) {
    $page_content .= '<h3>' . ucfirst($class) . ' Sampling Trip to ' . $data['location'] . '</h3>';
    $page_content .= DataBridge_textbox('Sampling Information', $data);
    if ( isset($data['latitude']) and isset($data['longitude']) ) {
      $coords = array();
      foreach ($data['sublocations'] as $position) {
        array_push($coords, array($position['latitude'], $position['longitude']));
      }
      $page_content .= '<div class="databridge_map">' . 
                       '<img src="' . _GoogleStaticMap($coords, 348, 250) . '"></div>';
    }
    $page_content .= '<div class="databridge_divider databridge_header">' . 'Sublocations' . '</div>';
    foreach ($data['sublocations'] as $location) {
      $page_content .= '<div class="databridge_sampling">' . 
                       '<div class="databridge_header">' . ucfirst($location['origin']) . 
                       ' on ' . strftime('%Y-%m-%d', $location['date']) . '</div>';                 
      $page_content .= '<div class="databridge_list">';
      
      // Create a header with the proper fields
      $header = array('Animal ID', 'Sex', 'Species', );
      foreach ( DataBridge_getAnalysis() as $analysis ) {
        array_push($header, $analysis['name']);
      }
      
      $animals = DataBridge_getAnimalInfo($location['origin'], $location['date']);
      
      // add links
      foreach ( $animals as $key => $animal ) {
        $animals[$key]['id'] = l($animal['id'], 'databridge/animal/' . $animal['id']);
      }
      
      $page_content .= theme('table', array('header' => $header, 'rows' => $animals,) );
      $page_content .= '</div>';                                    
      $page_content .= '</div>';
    }
  }
  
  // If the data was NOT found, present a summary of samplings.
  else {
    foreach ( $raw as $name => $sampling ) {
      $species = DataBridge_getSpecies($name);
      $rows = array();
      foreach ($sampling as $trip) {
        $sampled = 0;
        foreach ($species as $animal) {
          if (array_key_exists(strtolower($animal['common_name']) . '/sampled', $trip)) {
            $sampled += $trip[strtolower($animal['common_name']). '/sampled'];
          }
        }
        $date = strftime('%Y-%m-%d', $trip['start_date']);
        $location = l($trip['location'], 'databridge/sampling/' . $name . '/' . $trip['location'] . '/' . $date);
        array_push($rows, array($location, $date, $sampled,));
      }
      $page_content .= '<div class="databridge_sampling">' . 
                       '<div class="databridge_header">' . ucfirst($name) . ' Sampling Trips</div>';                 
      $page_content .= '<div class="databridge_list">' . 
                       theme('table', array('header' => array('Location', 'Date', 'Animals Sampled'),
                                            'rows'   => $rows,));
      $page_content .= '</div>';                                    
      $page_content .= '</div>';
    }
  }
  
  
  return $page_content;
}

/**
 * Displays detailed info about an animal.
 *
 * By default this function returns detailed information about a single animal,
 * when given an invalid id or no id defaults to showing the summary page and an 
 * error message.
 *
 * @param $id = NULL
 *   The id of the animal to extract from the database.
 *
 * @return $page_content
 *   Returns some html to drupal.
*/
function DataBridge_summary_animal($id = NULL) {
  $page_content  = '';
  $info = DataBridge_getAnimalInfo(NULL, NULL, $id);
  $page_content .= '<pre>';
  $page_content .= '$info=' . print_r($info, true) . "\n";
  $page_content .= '</pre>';
  
  return $page_content;
}

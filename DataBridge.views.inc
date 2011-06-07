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
 *   the DataBridge package. All functions in this file should be rewritten to
 *   use some kind of standard instead of passing around huge, more or less 
 *   randomly formatted arrays. This would also make formatting a lot more 
 *   streamlined.
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
  $page_content .= DataBridge_view_sampling();
  
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
 *   Sampling trip date
 * @param string $format = NULL
 *   Output format, defaults to html.
 *
 * @return string $html
 *   Html formatted sampling information
*/
function DataBridge_view_sampling($class = NULL, $location = NULL, $date = NULL, $format = NULL) {
  
  // if we're returning an excel sheet...
  if ( $format == 'xls' ) {
    drupal_add_http_header('Content-Type', 'application/vnd.ms-excel'); 
    drupal_add_http_header('Content-Disposition:', 'attachment; filename=' . $class . '_sampling_' . $location . '_' . $date . '.' . $format . ';');
    print DataBridge_excel(DataBridge_getSamplingTable($class, $location, $date));
    exit(0);
  } 
  
  // If we're presenting HTML:
  elseif ( !$format or $format == 'html' ) {
    $raw = DataBridge_getSamplingInfo();
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
    // If we have data, add a download link
    if ( $data ) {
      $data['download'] = l('xls', 'databridge/sampling/' . $class . '/' . $location . '/' . $date . '/xls');
    }
    
    // If the data was found, present it in a neat manner, otherwise present a summary of samplings.
    return $data ? DataBridge_format_sampling($data, $class) : DataBridge_format_sampling_summary($raw);
  }  else {
    _DataBridge_Message('UnknownFormat', $format);
    return DataBridge_format_sampling_summary($raw);
  }
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
 * @param string $format = NULL
 *   Output format, defaults to html.
 *
 * @return $page_content
 *   Returns some html to drupal.
*/
function DataBridge_view_animal($id = NULL, $format = NULL) {
  
  if ( $format == 'xls' ) {
    drupal_add_http_header('Content-Type', 'application/vnd.ms-excel'); 
    drupal_add_http_header('Content-Disposition:', 'attachment; filename=' . 'Animal_' . $id . '.' . $format . ';');
    print DataBridge_excel(DataBridge_getAnimalTable($id));
    exit(0);
  } else {
    $page_content  = '';
    $data = DataBridge_getAnimalInfo($id);
    $data['download'] = l('xls', 'databridge/animal/' . $id . '/xls');
    $page_content .= '<h3>Animal ' . ucfirst($id) . '</h3>';
    $page_content .= DataBridge_textbox('Animal Information', $data);
    
    // Add a map if there are latitude and longitude.
    if ( isset($data['Latitude']) and isset($data['Longitude']) ) {
      $coords = array();
      foreach ( $data['samples'] as $sample ) {
        if ( isset($sample['latitude']) and isset($sample['longitude']) ) {
          array_push( $coords, array($sample['latitude'], $sample['longitude']) );
        }
      }
      $page_content .= '<div class="databridge_map">' . 
                       '<img src="' . _GoogleStaticMap($coords, 348, 250) . '"></div>';
    }
    $page_content .= '<div class="databridge_divider databridge_header">' . 'Samples' . '</div>';
    
    // Add a box with information about each sample
    foreach ( $data['samples'] as $sample ) {
      $page_content .= DataBridge_textbox_slim(l($sample['id'], 'databridge/sample/' . $sample['id']), $sample);
    }
    
    return $page_content;
  }
}

/**
 * Displays detailed info about an sample.
 *
 * By default this function returns detailed information about a single sample,
 * when given an invalid id or no id defaults to showing the summary page and an 
 * error message.
 *
 * @param $id = NULL
 *   The id of the sample to extract from the database.
 * @param string $format = NULL
 *   Output format, defaults to html.
 *
 * @return $page_content
 *   Returns some html to drupal.
*/
function DataBridge_view_sample($id = NULL, $format = NULL) {
  
  if ( $format == 'xls' ) {
    drupal_add_http_header('Content-Type', 'application/vnd.ms-excel'); 
    drupal_add_http_header('Content-Disposition:', 'attachment; filename=' . 'Sample_' . $id . '.' . $format . ';');
    print DataBridge_excel(DataBridge_getSampleTable($id));
    exit(0);
  } else {
    $page_content  = '';
    $data = DataBridge_getSampleInfo($id);
    $data['download'] = l('xls', 'databridge/sample/' . $id . '/xls');
    $page_content .= '<h3>Sample ' . ucfirst($id) . '</h3>';
    if ( isset($data['animal']) ) {
      $data['animal'] = l($data['animal'], 'databridge/animal/' . $data['animal']);
    }
    $page_content .= DataBridge_textbox('Sample Information', $data);
    
    if ( isset($data['latitude']) and isset($data['longitude']) ) {
      $coords = array(array($data['latitude'], $data['longitude']));
      $page_content .= '<div class="databridge_map">' . 
                       '<img src="' . _GoogleStaticMap($coords, 348, 250, 10) . '"></div>';
    }
    
    // Add a box with information about each sibling if available
    if ( isset($data['siblings']) ) {
      $page_content .= '<div class="databridge_divider databridge_header">' . 'Sibling Samples' . '</div>';
      foreach ( $data['siblings'] as $sample ) {
        $page_content .= DataBridge_textbox_slim(l($sample['id'], 'databridge/sample/' . $sample['id']), $sample);
      }
    }
    
    return $page_content;
    
  }
}

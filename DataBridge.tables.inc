<?php

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
 *   Functions used for creating information tables in the Databridge module.
 *
 *   This include file implements the functions used to create information 
 *   tables in the DataBridge module. 
*/

/**
 * Gets a table of sample information, mostly used to return file summaries.
 *
 * @param $id
 *   The id of the sample to return
 *
 * @return array $table
 *   A table of sample information
*/
function DataBridge_getSampleTable($id) {
  $table = array();
  $raw = DataBridge_getSampleInfo($id);
  
  $siblings = array();
  foreach ( $raw as $key => $value ) {
    if ( $key == 'siblings' ) {
      $siblings = $value;
    } elseif ( preg_match('/.*date.*/', strtolower($key)) ) {
      array_push($table, array($key, strftime('%Y-%m-%d', $value)));
    } else {
      array_push($table, array($key, $value));
    }
  }
  
  if ( $siblings ) {
    array_push($table, array(''));
    array_push($table, array('Siblings'));
    $header = array();
    
    foreach ( $siblings as $sibling ) {
      if ( !$header ) {
        $header = array_keys($sibling);
        array_push($table, $header);
      }
      $row = array();
      foreach ( $sibling as $key => $value ) {
        if ( preg_match('/.*date.*/', strtolower($key)) ) {
          array_push($row, strftime('%Y-%m-%d', $value));
        } else {
          array_push($row, $value);
        }
      }
      array_push($table, $row);
    }
  }
  
  return $table;
}

/**
 * Gets a table of animal information, mostly used to return file summaries.
 *
 * @param $id
 *   The id of the animal to return
 *
 * @return array $table
 *   A table of animal information
*/
function DataBridge_getAnimalTable($id) {
  $table = array();
  
  $raw = DataBridge_getAnimalInfo($id);
  
  $samples = array();
  foreach ( $raw as $key => $value ) {
    $row = array($key, $value);
    if ( $key == 'samples' ) {
      $samples = $value;
    } else {
      array_push($table, $row);
    }
  }
  
  if ( $samples ) {
    array_push($table, array(''));
    array_push($table, array('Samples'));
    $header = array();
    
    foreach ( $samples as $sample ) {
      if ( !$header ) {
        $header = array_keys($sample);
        array_push($table, $header);
      }
      $row = array();
      foreach ( $sample as $key => $value ) {
        if ( preg_match('/.*date.*/', strtolower($key)) ) {
          array_push($row, strftime('%Y-%m-%d', $value));
        } else {
          array_push($row, $value);
        }
      }
      array_push($table, $row);
    }
  }
  
  return $table;
}

/**
 * Gets a table of sampling information, mostly used to return file summaries.
 *
 * @param string $class = NULL
 *   Sampling trip class, as set in the config/species page.
 * @param string $location = NULL
 *   Sampling trip location, as returned by DataBridge_getSamplingInfo()
 * @param string $date = NULL
 *   Sampling trip date
 *
 * @return array $table
 *   A table of sampling information
*/
function DataBridge_getSamplingTable($class, $location, $date) {
  $table = array();
  $title = $class . ' sampling  to ' . $location;
  
  // This thing needs to be rewritten... _BADLY_ needs to be rewritten
  $raw = DataBridge_getSamplingInfo();
  $data = array();
  // Check if we can get the data that we need from the database.
  if ( ($class != NULL) and ($location != NULL) and ($date != NULL) ) {
    foreach ( $raw[$class] as $trip ) {
      if ( $trip['location'] == $location and strftime('%Y-%m-%d', $trip['start_date']) == $date ) {
        $data = $trip;
        break;
      }
    }
  }
  
  // Check if we have any data and finish the title
  if ( $data ) {
    if ( $data['end_date'] ) {
      $title .= ' from ' . strftime('%Y-%m-%d', $data['start_date']) . 
                ' to ' . strftime('%Y-%m-%d', $data['end_date']);
    } else {
      $title .= ' on ' . strftime('%Y-%m-%d', $data['start_date']);
    }
  } else {
    return array();
  }
  
  array_push($table, array($title));
  array_push($table, array(''));
  
  $sublocations = array();
  $animals = array();
  
  foreach ( $data as $key => $value ) {
    if ( $key == 'sublocations' ) {
      $sublocations = $value;
    } elseif ( preg_match('/.*date.*/', strtolower($key)) ) {
      array_push($table, array($key, strftime('%Y-%m-%d', $value)));
    } else {
      array_push($table, array($key, $value));
    }
  }
  
  // sort out sublocations
  if ( $sublocations ) {
    array_push($table, array(''));
    array_push($table, array('Sublocations'));
    $header = array();
    
    // Calculate days to get animals for
    $dates = array($data['start_date']);
    $date = $data['start_date'];
    while ($date < $data['end_date']) {
      $date += 24*3600;
      array_push($dates, $date);
    }
    
    $locations = array();
    
    foreach ( $sublocations as $location ) {
      if ( !$header ) {
        $header = array_keys($location);
        array_push($table, $header);
      }
      $row = array();
      foreach ( $location as $key => $value ) {
        if ( preg_match('/.*date.*/', strtolower($key)) ) {
          array_push($row, strftime('%Y-%m-%d', $value));
        } else {
          array_push($row, $value);
        }
      }
      array_push($table, $row);
      array_push($locations, $location['origin']);  
    }
    
    // Add detail information about every animal in the sampling
    array_push($table, array(''));
    array_push($table, array('Animals'));
    $header = array();
    
    foreach ( DataBridge_getAnimalList($locations, $dates) as $animal ) {
      if ( !$header ) {
        $header = array_keys($animal);
        array_push($table, $header);
      }
      $row = array();
      foreach ( $animal as $key => $value ) {
        if ( preg_match('/.*date.*/', strtolower($key)) ) {
          array_push($row, strftime('%Y-%m-%d', $value));
        } else {
          array_push($row, $value);
        }
      }
      array_push($table, $row);
    }
    
  }
  
  
  return $table; 
}

/**
 * Gets a summary table of the database, mostly used to return file summaries.
 *
 * @return array $table
 *   A table of information
*/
function DataBridge_getSummaryTable() {
  $table = array();
  return $table;  
}

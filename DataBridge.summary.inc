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
  $page_content .= '<div id="databridge_sampling_trips">' . 
                   '<div class="databridge_header">Sampling Trips</div>';                 
  $page_content .= '<div class="databridge_list">' . 
                   theme('table', array('header' => array('Location', 'Date', 'Animals Sampled'),
                                        'rows'   => DataBridge_summary_trips(),));
  $page_content .= '</div>';                                    
  $page_content .= '</div>';
  
  $page_content .= '<div class="databridge_analysis">' . 
                   '<div class="databridge_header">Elisa Positive Samples</div>';                 
  $page_content .= '<div class="databridge_list">' . 
                   theme('table', array('header' => array('Sample', 'Animal', 'Location', 'Date',),
                                        'rows'   => DataBridge_positive_samples(),));
  $page_content .= '</div>';                                     
  $page_content .= '</div>';
  return $page_content;
}

/**
 * Returns a list of sampling trips to show in the summary page.
 *
 * Gets sampling trip data from the database functions and format it to view
 * on the summary page.
 *
 * @return array $trips
 *   An array of sampling trip information.
*/
function DataBridge_summary_trips() {
  $trips = array();
  foreach (Databridge_getSamplingTrips() as $raw) {
    $trip = array(l($raw[1], 'databridge/sampling/' . $raw[1] . '/' . strftime('%Y-%m-%d', $raw[0])), 
                  strftime('%Y-%m-%d', $raw[0]), $raw[4], );
    array_push($trips, $trip);
  }
  
  return $trips;
}

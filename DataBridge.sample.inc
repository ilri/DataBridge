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
 *   Functions used in the sample view by the DataBridge module
 *
 *   This include file implements the functions used by the sample view of
 *   the DataBridge package.
*/

/**
 * Formats information about a single sample gotten from the database.
 * 
 * @param int $id
 *   Id of the sample in the database.
 *
 * @return array $data
 *   An array of information about the sample.
*/
function DataBridge_Sample($id) {
  $data = array();
  if ($id) {   
    $raw = DataBridge_getSample($id);
    $data['id']            = $raw[0][0];
    $data['comments']      = $raw[0][1];
    $data['date_created']  = $raw[0][2];
    $data['date_updated']  = $raw[0][3];
    $data['sample_type']   = $raw[0][4];
    $data['origin']        = $raw[0][5];
    $data['org']           = $raw[0][6];
    $data['volume']        = $raw[0][7];
    $data['volume_unit']   = $raw[0][8];
    $data['unique_code']   = $raw[0][9];
    $data['main_operator'] = $raw[0][10];
    $data['box_id']        = $raw[0][11];
    $data['box_details']   = $raw[0][12];
    $data['wait']          = $raw[0][13];
    $data['secret']        = $raw[0][14];
    $data['Project']       = $raw[0][15];
    $data['SSID']          = $raw[0][16];
    $data['StoreLabel']    = $raw[0][17];
    $data['SampleID']      = $raw[0][18];
    $data['VisitID']       = $raw[0][19];
    $data['VisitDate']     = strftime('%Y-%m-%d', $raw[0][20]);
    $data['AnimalID']      = $raw[0][21];
    $data['Visit']         = $raw[0][22];
    $data['ShippedDate']   = $raw[0][23];
    $data['TrayID']        = $raw[0][24];
    $data['DateRecorded']  = $raw[0][25];
    $data['Longitude']     = $raw[0][26];
    $data['Latitude']      = $raw[0][27];
    $data['Elisa_Results'] = $raw[0][28];
    $data['map']           = $raw[1];
    $data['trip']          = l($raw[2][0] . ', ' .  $raw[2][1], '/databridge/sampling/' . $raw[2][0] . '/' . $raw[2][1]);
    $data['siblings']      = $raw[3];
  } else {
    $data = array('id' => NULL);
  }
  return $data;
}

/**
 * Lists samples from the database.
 *
 * Creates a list of samples from the database.
 *
 * @param int $start = 0
 *   The first sample to be listed
 * @param int $nr_of_items = 50
 *   The number of items to view on each page.
 *
 * @return string $html
 *   An html page with an sampling trip list.
*/
function DataBridge_samples($start = 0, $nr_of_items = 50) {
  $page_content  = '';
  $samples = array();
  foreach (DataBridge_getSamples($start, $nr_of_items) as $sample) {
    switch ($sample[2]) {
      case "POSITIVE": $elisa = '<span class="databridge_warning">POSITIVE</span>'; break;
      case "NEGATIVE": $elisa = '<span class="databridge_ok">NEGATIVE</span>';      break;
      default:         $elisa = '';
    }
    array_push($samples, array(l($sample[0], 'databridge/sample/' . $sample[0]), strftime('%Y-%m-%d', $sample[1]),
                               $sample[8], $elisa));
  }
  
  $page_content .= DataBridge_listcontrol('sample', $start, $nr_of_items);
  $page_content .= '<div id="databridge_samplelist">';
  $page_content .= theme('table', array('header' => array('Sample ID', 'Date','Institution', 'ELISA'), 
                                        'rows' => $samples));
  $page_content .= '</div>';
  $page_content .= DataBridge_listcontrol('sample', $start, $nr_of_items);
  return $page_content;
}

/**
 * Displays information about a sample.
 *
 * Creates a simple information page about a single sample. 
 *
 * @param int $id = NULL
 *   Database id of the sample to be viewed.
 *
 * @return string $html
 *   An html page to be returned by drupal.
*/
function DataBridge_sampleview($id = NULL) {
  $page_content  = '';
  if ($id == NULL) {
    return DataBridge_samples();
  } else {
    $data = DataBridge_Sample($id);
    $page_content .= '<div id="databridge_sampleview">' . 
                     '<div class="databridge_header">Sample ' . $data['id'] . '</div>';
    $page_content .= '<div class="databridge_info databridge_large">' . 
                      Databridge_sampleview_static($data);
    $page_content .= '</div>';
    $page_content .= '</div>';
    if ($data['map']) {
      $page_content .= '<div id="databridge_sampleview_map">' . 
                       '<img src="' . $data['map'] . '"></div>';
    }
    $page_content .= '<div id="databridge_sampleview_comment">' . 
                     '<div class="databridge_title">' . 
                     'Comments' . 
                     '</div>' .
                     $data['comments'] . 
                     '</div>';
    $page_content .= '<div class="databridge_divider databridge_header">' . 'Sibling Samples' . '</div>';
    $page_content .= '<div id="databridge_animalview_samples">';
    foreach ($data['siblings'] as $sample) {
      $page_content .= Databridge_animalview_sample($sample);
    }
    $page_content .= '</div>';
  }
  return $page_content;
}

/**
 * Displays static sample information.
 *
 * @param array $data
 *   An array contatining the data to be displayed.
 *
 * @return string $html
 *   The returned html.
*/
function Databridge_sampleview_static($data) {
  $page_content  = '';
  $page_content .= '<div class="databridge_left">' . 
                   'Sample ID:'     . '<br>' . 
                   'Sampling Trip:' . '<br>' . 
                   'Date Created:'  . '<br>' . 
                   'Date updated:'  . '<br>' . 
                   'Sample type:'   . '<br>' . 
                   'Origin:'        . '<br>' . 
                   'Org:'           . '<br>' . 
                   'Main Operator:' . '<br>' . 
                   'Box Id:'        . '<br>' . 
                   'Box Details:'   . '<br>' . 
                   'StoreLabel:'    . '<br>' . 
                   'SampleID:'      . '<br>' . 
                   'VisitID:'       . '<br>' . 
                   'VisitDate:'     . '<br>' . 
                   'AnimalID:'      . '<br>' . 
                   'Visit:'         . '<br>' . 
                   'ShippedDate:'   . '<br>' . 
                   'TrayID:'        . '<br>' . 
                   'DateRecorded:'  . '<br>' . 
                   'Longitude:'     . '<br>' . 
                   'Latitude:'      . '<br>' . 
                   'Elisa Results:' . '<br>' .
                   '</div>';
  $page_content .= '<div class="databridge_left">' . 
                    $data['id']            . '<br>' .
                    $data['trip']          . '<br>' . 
                    $data['date_created']  . '<br>' . 
                    $data['date_updated']  . '<br>' . 
                    $data['sample_type']   . '<br>' . 
                    $data['origin']        . '<br>' . 
                    $data['org']           . '<br>' . 
                    $data['main_operator'] . '<br>' . 
                    $data['box_id']        . '<br>' . 
                    $data['box_details']   . '<br>' . 
                    $data['StoreLabel']    . '<br>' . 
                    $data['SampleID']      . '<br>' . 
                    $data['VisitID']       . '<br>' . 
                    $data['VisitDate']     . '<br>' . 
                    $data['AnimalID']      . '<br>' . 
                    $data['Visit']         . '<br>' . 
                    $data['ShippedDate']   . '<br>' . 
                    $data['TrayID']        . '<br>' . 
                    $data['DateRecorded']  . '<br>' . 
                    $data['Longitude']     . '<br>' . 
                    $data['Latitude']      . '<br>' . 
                    $data['Elisa_Results'] . '<br>' .
                   '</div>';
  
  return $page_content;
}

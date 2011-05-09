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
 *   Functions used in the animal view by the Databridge module
 *
 *   This include file implements the functions used by the animal view of the 
 *   Databridge package.
*/

/**
 * Lists animals from the database.
 *
 * Creates a list of animals from the database.
 *
 * @param int $start = 0
 *   The first animal to be listed
 * @param int $nr_of_items = 50
 *   The number of items to view on each page.
 *
 * @return string $html
 *   An html page with an animal list.
*/
function Databridge_animals($start = 0, $nr_of_items = 50) {
  $page_content  = '';
  $animals = array();
  foreach(Databridge_getAnimals($start, $nr_of_items) as $animal) {
    array_push($animals, array(l($animal[0], 'databridge/animal/' . $animal[0]), $animal[1], $animal[2], $animal[3], $animal[5]));
  }
  
  $page_content .= Databridge_listcontrol('animal', $start, $nr_of_items);
  $page_content .= '<div id="databridge_animallist">';
  $page_content .= theme('table', array('header' => array('Animal ID', 'Species', 'Sex', 'Location', 'Status'), 
                                        'rows' => $animals));
  $page_content .= '</div>';
  $page_content .= Databridge_listcontrol('animal', $start, $nr_of_items);
  
  return $page_content;
}

/**
 * Displays animal information about a single animal and it's samples
 *
 * Creates a simple information page about a single animal. The page has three
 * parts, a general static information div, a static google map of sampling 
 * locations and lastly a dynamic list of collected samples. If no id is given
 * the function will default to showing the animal list instead.
 *
 * @param int $id = NULL
 *   Database id of the animal to be viewed.
 *
 * @return string $html
 *   An html page to be returned by drupal.
*/
function Databridge_animalview($id = NULL) {
  $page_content  = '';
  if ($id) {
    $data = DataBridge_getAnimal($id);
    $page_content .= '<div id="databridge_animalview">';
    $page_content .= '<div id="databridge_animalview_static">' . 
                     '<div class="databridge_header">Animal Information</div>' . 
                     '<div class="databridge_info databridge_large">' . 
                     Databridge_animalview_static($data) . 
                     '</div>' . '</div>';
    if ($data['map']) {
      $page_content .= '<div id="databridge_animalview_map">' . 
                       '<img src="' . $data['map'] . '"></div>';
    }
    $page_content .= '<div class="databridge_divider databridge_header">' . 'Samples' . '</div>';
    $page_content .= '<div id="databridge_animalview_samples">';
    foreach ($data['samples'] as $sample) {
      $page_content .= Databridge_animalview_sample($sample);
    }
    $page_content .= '</div>';
    $page_content .= '</div>';
  }
  else {
    $page_content = Databridge_animals();
  }
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
function Databridge_animalview_static($data) {
  $page_content  = '';
  $page_content .= '<div class="databridge_left">' . 
                   'Animal ID:    <br>' . 
                   'Previous tag: <br>' . 
                   'Organism:     <br>' . 
                   'Date of Birth:<br>' . 
                   'Sex:          <br>' . 
                   'Location:     <br>' . 
                   'Owner:        <br>' . 
                   'Status:       ' . 
                   '</div>';
  $page_content .= '<div class="databridge_left">' . 
                    $data['id']           . '<br>' .
                    $data['previous_tag'] . '<br>' .
                    $data['organism']     . '<br>' .
                    $data['dob']          . '<br>' .
                    $data['sex']          . '<br>' .
                    $data['location']     . '<br>' .
                    $data['owner']        . '<br>' .
                    $data['status']       . 
                   '</div>';
  
  return $page_content;
}

/**
 * Renders a single sample information box.
 *
 * @param array $sample = NULL
 *   An array of of sample information to display.
 *
 * @return string $html
 *   An html div holding the sample information.
*/
function Databridge_animalview_sample($sample = NULL) {
  $page_content  = '';
  if ($sample) {
    $page_content .= '<div class="databridge_animalview_sample">';
    $page_content .= '<div class="databridge_title">' . l($sample['id'], 'databridge/sample/' . $sample['id']) . '</div>';
    $num = $sample['var'] ? number_format($sample['var'], 2) : '';
    $page_content .= '<div class="databridge_info databridge_small">' . 
                     'Date: ' . $sample['date'] . '<br>' .
                     'ELISA: ' . $sample['status'] . '<br>' .
                     '%inhib: ' . $num . '<br>' .
                     'Tray: ' . $sample['locale'] . '<br>';
    $page_content .= '</div>';            
    $page_content .= '</div>';
  }
  return $page_content; 
}

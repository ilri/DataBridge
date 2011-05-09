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
 *   Functions used in the analysis view by the Databridge module
 *
 *   This include file implements the functions used by the analysis view of the 
 *   Databridge package.
*/

/**
 * Gets samples positive to a specific analysis from the database
 *
 * @param string $type
 *   The type of analysis results to return
 *
 * @return array $samples
 * 
*/
function DataBridge_positive_samples($type = 'elisa') {
  $data = array();
  switch ($type) {
    case 'elisa': foreach (DataBridge_getPositive('elisa') as $raw) {
                    $time = strftime('%Y-%m-%d', $raw[3]);
                    array_push($data, array(l($raw[0], '/databridge/sample/' . $raw[0]), $raw[1], $raw[2], $time,));
                  } break;
    default: break;
  }
  return $data;
}
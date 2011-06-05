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
 * along with DataBridge.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *   Functions using external resources for the DataBridge module
*/

/**
 * Returns google geo info for a specific point.
 *
 * @param float $latitude
 *   Latitude for the point to check (decimal degrees)
 * @param float $longitude
 *   Longitude for the point to check (decimal degrees)
 *
 * @return array $data
*/
function DataBridge_geocode($latitude, $longitude) {
  $url = 'http://maps.google.com/maps/geo' . '?q=' . $latitude . ',' . $longitude;
  $info = array();
  
  $session = curl_init($url);
  if ($session) {
    curl_setopt($session, CURLOPT_RETURNTRANSFER, TRUE);
    $info = drupal_json_decode(curl_exec($session));
  }
  curl_close($session);
  return $info;
}


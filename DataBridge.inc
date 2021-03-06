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
 *   Functions and constants used by the DataBridge module
*/

/**
 * Sets default messages for the DataBridge module.
 *
 * @param string $type
 *   The type of message to be set, valid values are:
 *   - UnknownType
 *   - NotConnected
 * @param string $opt1
 *   Extra information to the message
*/
function _DataBridge_Message($type = '', $opt1 = '') {
  $msg = 'Something went wrong but something also went wrong with reporting what\'s wrong.';
  $msg_type = 'warning';
  switch ($type) {
  case 'UnknownType':  $msg = 'DataBridge asked for unknown type ' . $opt1 . '.';
                       $msg_type = 'warning';
                       break;
  case 'NotConnected': $msg = 'Not connected to the DataBridge database.';
                       $msg_type = 'error';
  case 'UnknownFormat':$msg = 'Unknown Format "' . $opt1 . '"';
                       $msg_type = 'warning';
                       break;
  }
  drupal_set_message($msg, $msg_type);
}

/**
 * Plots a position or a series of positions on a google static map.
 *
 * This function plots a number of markers on a google static map and returns
 * the url for this map. 
 *
 * @param array(array) coords
 *   An array of arrays containing coordinates as array([0] => lat, [1] => lon)
 * @param int $w
 *   Width of the returned image. default: 200
 * @param int $h
 *   Height of the returned image. default: 200
 * @param int $z
 *   Zoom of the google static map. default: 10
 * @param array $colors
 *   An array of colors that the markers will cycle through. 
 *   default: Blue, Red, Green, Yellow
 *
 * @return string $url
 *   The URL for the google static map.
*/
function _GoogleStaticMap($coords, $w = 200, $h = 200, $z = NULL, $colors = array('blue', 'red', 'green', 'yellow')) {
  $and = "&amp;";
	$url = "http://maps.google.com/maps/api/staticmap?size=".$w."x".$h.$and;
	if ($z) {
	  $url .= "zoom=" . $z;
	}
	foreach ($coords as $pos) {
		$url .= $and."markers=size:mid|color:blue|label:S|".$pos[0].",".$pos[1];
	}
	$url .= $and."sensor=false";
	return $url;
}

/**
 * Makes sure that an array of arrays is square.
 *
 * @param &array(array) $sheet
 *   The array to be balanced
 * @param int $length
 *   The required width of the sheet
*/
function _balance_sheet(&$sheet, $length) {
  for($i = 0; $i < count($sheet); $i++) {
    while (count($sheet[$i]) < $length) {
      array_push($sheet[$i], '');
    }
  }
  return True;
}

/**
 * Creates an excel spreadsheet from an array of arrays
 *
 * Note that this is a very simplified excel writer that only creates one very
 * specific type of spreadsheets.
 *
 * @param array(array) $data
 *   An array of arrays containing the spreadsheet data. Format: Row(Column).
 *
 * @return string $sheet
 *   A string contaning a complete excel spreadsheet
*/
function DataBridge_excel($data = array(array()), $date = NULL, $author = 'TimeView') {
  if (!$date) {
    $date = strftime('%Y-%m-%dT%H:%M:%SZ');
  }
  $header = <<<EOH
<html xmlns:o="urn:schemas-microsoft-com:office:office"
xmlns:x="urn:schemas-microsoft-com:office:excel"
xmlns="http://www.w3.org/TR/REC-html40">

<head>
<meta http-equiv=Content-Type content="text/html; charset=us-ascii">
<meta name=ProgId content=Excel.Sheet>
<!--[if gte mso 9]><xml>
 <o:DocumentProperties>
  <o:LastAuthor>{$author}</o:LastAuthor>
  <o:LastSaved>{$date}</o:LastSaved>
  <o:Version>10.2625</o:Version>
 </o:DocumentProperties>
 <o:OfficeDocumentSettings>
  <o:DownloadComponents/>
 </o:OfficeDocumentSettings>
</xml><![endif]-->
<style>
<!--table
	{mso-displayed-decimal-separator:"\.";
	mso-displayed-thousand-separator:"\,";}
@page
	{margin:1.0in .75in 1.0in .75in;
	mso-header-margin:.5in;
	mso-footer-margin:.5in;}
tr
	{mso-height-source:auto;}
col
	{mso-width-source:auto;}
br
	{mso-data-placement:same-cell;}
.style0
	{mso-number-format:General;
	text-align:general;
	vertical-align:bottom;
	white-space:nowrap;
	mso-rotate:0;
	mso-background-source:auto;
	mso-pattern:auto;
	color:windowtext;
	font-size:10.0pt;
	font-weight:400;
	font-style:normal;
	text-decoration:none;
	font-family:Arial;
	mso-generic-font-family:auto;
	mso-font-charset:0;
	border:none;
	mso-protection:locked visible;
	mso-style-name:Normal;
	mso-style-id:0;}
td
	{mso-style-parent:style0;
	padding-top:1px;
	padding-right:1px;
	padding-left:1px;
	mso-ignore:padding;
	color:windowtext;
	font-size:10.0pt;
	font-weight:400;
	font-style:normal;
	text-decoration:none;
	font-family:Arial;
	mso-generic-font-family:auto;
	mso-font-charset:0;
	mso-number-format:General;
	text-align:general;
	vertical-align:bottom;
	border:none;
	mso-background-source:auto;
	mso-pattern:auto;
	mso-protection:locked visible;
	white-space:nowrap;
	mso-rotate:0;}
.xl24
	{mso-style-parent:style0;
	white-space:normal;}
-->
</style>
<!--[if gte mso 9]><xml>
 <x:ExcelWorkbook>
  <x:ExcelWorksheets>
   <x:ExcelWorksheet>
	<x:Name>srirmam</x:Name>
	<x:WorksheetOptions>
	 <x:Selected/>
	 <x:ProtectContents>False</x:ProtectContents>
	 <x:ProtectObjects>False</x:ProtectObjects>
	 <x:ProtectScenarios>False</x:ProtectScenarios>
	</x:WorksheetOptions>
   </x:ExcelWorksheet>
  </x:ExcelWorksheets>
  <x:WindowHeight>10005</x:WindowHeight>
  <x:WindowWidth>10005</x:WindowWidth>
  <x:WindowTopX>120</x:WindowTopX>
  <x:WindowTopY>135</x:WindowTopY>
  <x:ProtectStructure>False</x:ProtectStructure>
  <x:ProtectWindows>False</x:ProtectWindows>
 </x:ExcelWorkbook>
</xml><![endif]-->
</head>

<body link=blue vlink=purple>
<table x:str border=0 cellpadding=0 cellspacing=0 style='border-collapse: collapse;table-layout:fixed;'>\n
EOH;
  $footer = '</table></body></html>';

  $sheet = $header;
  foreach ($data as $row) {
    $sheet .= "  <tr>";
  	foreach($row as $col) {
  	  $sheet .= "<td class=xl24 width=64 >$col</td>";
  	}
    $sheet .= "</tr>\n";
  }
  $sheet .= $footer;
  
  return $sheet;
}

/**
 * Recursively finds a key in a nested array and returns it's value
 *
 * @param array $data
 *   A nested array to search through
 * @param string $key
 *   The key to search for
 *
 * @return $value
 *   The value of the key or NULL if not found.
*/
function DataBridge_rfind($data, $key) {
  if ( !is_array($data) ) {
    return NULL;
  }
  if (array_key_exists($key, $data)) {
    return $data[$key];
  }
  foreach ( $data as $sub ) {
    if ( is_array($sub) ) {
      if ( $val = DataBridge_rfind($sub, $key) ) {
        return $val;
      }
    }
  }
  return NULL;
}

/**
 * Returns a div with formatted text.
 *
 * @param string $title
 *   Title text for the textbox
 * @param array $input
 *   An associative array of values to present
 * @param bool $include_table = true
 *   If true, the textbox will also include a summary table.
 * 
 * @return $html
 *   HTML div for the textbox
*/
function DataBridge_textbox($title, $input, $include_table = true) {
  $page_content  = '';
  $page_content .= '<div class="databridge_textbox">';
  $page_content .= '<div class="databridge_header">' . $title . '</div>';
  $page_content .= '<div class="databridge_large">';
  
  $page_content .= DataBridge_info($input, true);

  $page_content .= '</div>';
  $page_content .= '</div>';
  return $page_content;
}

/**
 * Returns a div with formatted text.
 *
 * This div is slimmer than the other type!
 *
 * @param string $title
 *   Title text for the textbox
 * @param array $input
 *   An associative array of values to present
 * @param bool $include_table = true
 *   If true, the textbox will also include a summary table.
 * 
 * @return $html
 *   HTML div for the textbox
*/
function DataBridge_textbox_slim($title, $input, $include_table = true) {
  $page_content  = '';
  $page_content .= '<div class="databridge_textbox_slim">';
  $page_content .= '<div class="databridge_title">' . $title . '</div>';
  $page_content .= '<div class="databridge_small">';
  
  $page_content .= DataBridge_info($input, true);

  $page_content .= '</div>';
  $page_content .= '</div>';
  return $page_content;
}

/**
 * Returns a set of float: left divs with formatted information.
 *
 * @param array $input
 *   An associative array of values to present
 * @param bool $include_table = true
 *   If true, the textbox will also include a summary table.
 *
 * @return string $html
 *   A html string including the formatted information.
*/
function DataBridge_info($input, $include_table = true) {
  $page_content = '<div class="databridge_info">';
  $keys = '';
  $values = '';
  $table = array();
  foreach ( $input as $key => $value ) {
    if ( !is_array($value) ) {
      if ( strpos($key, '/') ) {
        $table[$key] = $value;
      }
      else {
        if ( stristr($key, 'date') ) {
          try {
            if ( !$value ) {
              $values .= '-' . '<br>';
            } else {
              $values .= strftime('%Y-%m-%d', $value) . '<br>';
            }
          }
          catch (Exception $e)  {
            $values .= $value . '<br>';
          }
        } else {
          $values .= $value . '<br>';
        }
        $keys   .= DataBridge_format($key) . ': <br>';
      }
    }
  }
  $page_content .= '<div class="databridge_left">' . $keys . '</div>';
  $page_content .= '<div class="databridge_left">' . $values . '</div>';
  if ($table and $include_table) {
    $page_content .= '<div class="databridge_table">' . theme('table', DataBridge_table($table)) . '</div>';
  }
  $page_content .= '</div>';
  return $page_content;
}

/**
 * Makes a information table from databridge species/analysis info.
 *
 * @param array $data
 *   An array of data in the format [species/key => value]
 *
 * @return array $table
 *   An array formatted as a drupal table. [header => array, rows => array].
*/
function DataBridge_table($data) {
  $header = array('');
  $rows = array();
  foreach ( $data as $key => $value ) {
     if ( strpos($key, '/') ) {
      $split = explode('/', $key);
      if ( !in_array($split[0], $header) ) {
        array_push($header, $split[0]);
      }
      if ( !array_key_exists($split[1], $rows) ) {
        $rows[$split[1]] = array('title' => $split[1]);
      }
      $rows[$split[1]][$split[0]] = $value;
    }
  }
  
  // Make the text a bit neater
  foreach ($header as $key => $value) { $header[$key]  = ucfirst($value);    }
  foreach ($rows as $key => $value)   { $rows[$key]['title'] = ucfirst($value['title']); }
  
  return array('header' => $header, 'rows' => $rows);
}

/**
 * Format a string a bit neater when taken from the database.
 *
 * @param string $value
 *   The string to be formatted
 *
 * @return string $value
 *   The formatted string
*/
function DataBridge_format($value) {
  return ucfirst(str_replace('_', ' ',$value));
}

/**
 * Returns a formatted page describing a sampling trip.
 *
 * @param array $data
 *   An array of data to be presented
 * @param array $class
 *   Sampling class
 *
 * @return $page_content
 *   Page content to be displayed.
*/
function DataBridge_format_sampling($data, $class) {
  $page_content  = '';
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
    
    $animals = DataBridge_getAnimalList($location['origin'], $location['date']);
    
    // add links
    $header = array();
    foreach ( $animals as $key => $animal ) {
      $animals[$key]['id'] = l($animal['id'], 'databridge/animal/' . $animal['id']);
      if ( !$header ) {
        foreach ( $animal as $key => $value ) {
          array_push($header, DataBridge_format($key));
        }
      }
    }
    
    $page_content .= theme('table', array('header' => $header, 'rows' => $animals,) );
    $page_content .= '</div>';                                    
    $page_content .= '</div>';
  }  
  return $page_content;
}

/**
 * Returns a neat summary of all sampling trips from an array of data.
 *
 * @param array $raw
 *   An array of data to be presented
 *
 * @return $page_content
 *   Page content to be displayed.
*/
function DataBridge_format_sampling_summary($raw) {
  $page_content  = '';
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
  return $page_content;
}

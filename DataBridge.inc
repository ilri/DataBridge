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
function _GoogleStaticMap($coords, $w = 200, $h = 200, $z = 10, $colors = array('blue', 'red', 'green', 'yellow')) {
  $and = "&amp;";
	$url = "http://maps.google.com/maps/api/staticmap?size=".$w."x".$h.$and."zoom=".$z;
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
 * Creates list controls for a list
 *
 * @param string $list
 *   The list to be controlled
 * @param int $start
 *   Start of the current list
 * @param int $nr_of_items
 *   Number of items in the current list
 *
 * @return string $html
*/
function Databridge_listcontrol($list, $start = 0, $nr_of_items = 50) {
  $page_content  = '<div class="databridge_listcontrol">';
  if ($start > 0) {
    $min = ($start - $nr_of_items) < 0 ? 0 : ($start - $nr_of_items);
    $page_content .= l('<<', 'databridge/' . $list . '/list/' . $min . '/' . $nr_of_items);
  }
  $page_content .= ' [' . $start . ' to ' . ($start + $nr_of_items) . '] ';
  $page_content .= l('>>', 'databridge/' . $list . '/list/' . ($start + $nr_of_items) . '/' . $nr_of_items);
  $page_content .= '</div>';
  return $page_content;
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
function _Excel($data = array(array()), $date = NULL, $author = 'TimeView') {
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
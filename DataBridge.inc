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
 * Class that handles the database connection to the DataBridge database
 *
 * This class depends on having set the connection variables in the module 
 * configuration page.
*/
Class _DataBridgeInterface {
  var $user;
  var $pass;
  var $name;
  var $host;
  var $db;
  var $error;
  
  /**
   * Constructor for the _DataBridgeInterface.
   *
   * The constructor assigns the variables set in the drupal configuration page
   * to local variables.
  */
  public function __construct() {
    $this->user = variable_get('DataBridge_dbuser', 'databridge');
    $this->pass = variable_get('DataBridge_dbpass', '');
    $this->name = variable_get('DataBridge_dbname', 'databridge');
    $this->host = variable_get('DataBridge_dbhost', 'localhost');
  }
  
  /**
   * Opens the database connection
  */
  public function open() {
    if ( !($this->db = mysql_connect($this->host, $this->user, $this->pass)) ) {
      $this->error = mysql_error();
      return False;
    } elseif ( !mysql_select_db($this->name, $this->db) ) {
      $this->error = mysql_error();
      return False;
    }
    return True;
  }
  
  /**
   * Closes the database connection
  */
  public function close() {
    if ($this->db) {
      mysql_close($this->db);
    }
  }
  
  /**
   * Performs a query in the database and returns the results.
   *
   * This function performs a query in the DataBridge database and returns 
   * the query results in an array.
   *
   * @param string $query
   *   The query to be executed in the database.
   *
   * @return array $results
   *   An array containing the query results.
  */
  public function query($query) {
    $results = array();
    if ($this->db) {
      $query_results = mysql_query($query, $this->db);
      while ($query_results and $result = mysql_fetch_row($query_results)) {
        array_push($results, $result);
      }
      
    } else {
      _DataBridge_Message('NotConnected');
    }
    return $results;
  }
  
} 

/**
 * Creates an array of events for each livestock sampling
 * 
 * @return array(array) $events
 *   An array of associative arrays containing the events
*/
function DataBridge_SamplingTrips() {
  $query = "SELECT DATE_FORMAT(s.VisitDate, '%b %d %Y') AS date, 
                   s.origin,
                   s.Latitude,
                   s.Longitude,
                   SUM(IF(s.project = 5 AND s.AnimalID LIKE 'AVD%', 1, 0)) AS sampled_animals, 
                   SUM(IF(ca.organism = 2, 1, 0)) AS sampled_goats, 
                   SUM(IF(ca.organism = 2 AND ca.sex = 'Female', 1, 0)) AS female_goats, 
                   SUM(IF(ca.organism = 3 AND ca.sex = 'Female', 1, 0)) AS female_sheep, 
                   SUM(IF(ca.organism = 2 AND ca.Status = 'Alive', 1, 0)) AS goats_alive, 
                   SUM(IF(ca.organism = 2 AND ca.Status = 'Dead', 1, 0)) AS goats_dead, 
                   SUM(IF(ca.organism = 3 AND ca.Status = 'Alive', 1, 0)) AS sheep_alive, 
                   SUM(IF(ca.organism = 3 AND ca.Status = 'Dead', 1, 0)) AS sheep_dead, 
                   SUM(IF(ca.organism = 3 AND et.STATUS = 'POSITIVE', 1, 0)) AS sheep_positive, 
                   SUM(IF(ca.organism = 2 AND et.STATUS = 'POSITIVE', 1, 0)) AS goats_positive, 
                   CONCAT(TO_DAYS(s.VisitDate), '.', s.origin) AS link
            FROM (
                   SELECT AnimalID, 
                          VisitDate, 
                          origin, 
                          project, 
                          count, 
                          Latitude, 
                          Longitude
                    FROM samples
                    GROUP BY AnimalID
                  ) s
                  JOIN custom_animals ca ON s.AnimalID = ca.animal_id 
                  LEFT JOIN elisaTest et ON et.sampleID = s.count 
            WHERE VisitDate AND origin IS NOT NULL AND s.project = 5
            GROUP BY TO_DAYS(date), origin;";
  $events = array();
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    foreach ($db->query($query) as $result) {
      $event = array();
      $event['name']       = $result[1];
      $event['start_date'] = $result[0];
      $event['end_date']   = NULL;
      $event['map']        = _GoogleStaticMap(array(array($result[2], $result[3])));
      $event['latitude']   = $result[2];
      $event['longitude']  = $result[3];
      $event['private']    = user_access('access DataBridge private') 
                             ? '<a href="' . url('databridge/private/sampling/' . str_replace('.', '/', $result[14]))  . '">&gt; Download Summary</a>' 
                             : '';
      $event['open']       = theme('table', 
                                   array('header' => array(array('data' => $result[4] . ' animals sampled', 'colspan' => 3),),
                                         'rows'   => array(array('Sheep',  $result[7] . ' F', ($result[4] - $result[5] - $result[7]) . ' M',),
                                                           array('Goats',  $result[6] . ' F', ($result[5] - $result[6]             ) . ' M',),)
                                   )) . 
                             theme('table', 
                                   array('header' => array(array('data' => 'Animal Status', 'colspan' => 4),),
                                         'rows'   => array(array('', 'Alive', 'Dead', 'Unacc.'),
                                                           array('Sheep', $result[10], $result[11], ($result[4] - $result[5]) - ($result[10] + $result[11])),
                                                           array('Goats', $result[8], $result[9], $result[5] - $result[8] - $result[9])))) . 
                             theme('table', 
                                   array('header' => array(array('data' => 'Elisa Results', 'colspan' => 3),),
                                         'rows'   => array(array('', 'Positive', 'Negative'),
                                                           array('Sheep', $result[12], ($result[4] - $result[5]) - $result[12]),
                                                           array('Goats', $result[13], $result[5] - $result[13]))));
      array_push($events, $event);
    }
  }
  return $events;
}

/**
 * Creates a sheet summary of all livestock sampling.
 *
 * This function creates an array of arrays in the format rows[cols] containing
 * all livestock sampling information that is stored until now in the database.
 *
 * @return array $sheet
 *  An array of arrays containing the information. Format: rows[cols].
*/
function DataBridge_SamplingSummary($start_day = 0, $end_day = 20000000, $locations = 'all') {
  $sheet = array();
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    $location_query = 'SELECT DISTINCT origin 
                       FROM samples 
                       WHERE project = 5 
                         AND AnimalID LIKE "AVD%" 
                         AND origin != "";';
    $locations = array();
    foreach ($db->query($location_query) as $result) {
      array_push($locations, "'" . $result[0] . "'");
    }
    $from = "samples s JOIN custom_animals ca ON s.AnimalID = ca.animal_id LEFT JOIN elisaTest et ON et.sampleID = s.count LEFT JOIN organisms o ON ca.organism = o.org_id "; // Organism added if requested later.
    $where = "s.project = 5 AND s.AnimalID LIKE 'AVD%' AND TO_DAYS(s.VisitDate) >= ".$start_day." AND TO_DAYS(s.VisitDate) <= ".$end_day." AND s.origin IN (".implode(', ', $locations).")";
    $elisa = "IF(SUM(IF(et.STATUS = 'POSITIVE', 1, 0)), '+', IF(SUM(IF(et.STATUS = 'NEGATIVE', 1, 0)), '-', '')) 'Elisa' ";
    $dates = "SELECT DATE_FORMAT(s.VisitDate, '%Y-%m-%d') VisitDate, TO_DAYS(s.VisitDate), s.origin days FROM ".$from." WHERE VisitDate != '0000-00-00' AND ".$where." GROUP BY TO_DAYS(s.VisitDate);";
    $animals = 'SELECT DISTINCT s.AnimalID, o.org_name FROM '.$from.' WHERE '.$where.';';
    
    $header = array('Animal ID', 'organism');
    $pre_header = array();
    foreach($header as $nothing) {
      array_push($pre_header, '');
    }
    array_pop($pre_header);
    array_push($pre_header, 'Elisa Results');
    $sheet = array($pre_header, $header);

    foreach ($db->query($animals) as $result) {
      array_push($sheet, $result);
    }
    
    $length = count($sheet[1]);
    foreach ($db->query($dates) as $result) {
      array_push($sheet[1], $result[0]);
      array_push($sheet[0], $result[2]);
      $elisa_query = 'SELECT s.AnimalID, '.$elisa.' FROM '.$from.' WHERE TO_DAYS(s.VisitDate) = '.$result[1].' AND '.$where.' GROUP BY s.AnimalID;';
      foreach ($db->query($elisa_query) as $elisa_result) {
        for ($i = 0; $i < count($sheet); $i++) {
          if ($sheet[$i][0] == $elisa_result[0]) {
            array_push($sheet[$i], $elisa_result[1]);
            break;
          }
        }
      }
      _balance_sheet($sheet, ++$length);
    }
  }
  return $sheet;
}

/**
 * Creates an array of events for each wildlife sampling
 * 
 * @return array(array) $events
 *   An array of associative arrays containing the events
*/
function DataBridge_WildlifeTrips() {
  $query = "SELECT DATE_FORMAT(VisitDate, '%b %d %Y') AS date, 
                   origin,
                   Latitude,
                   Longitude,
                   SUM(IF(count, 1, 0)) AS samples
            FROM samples
            WHERE VisitDate AND origin IS NOT NULL AND project = 5
            AND AnimalID NOT IN (SELECT animal_id FROM custom_animals)
            GROUP BY TO_DAYS(date), origin;";
  $events = array();
  $db = new _DataBridgeInterface();
  if ($db->open()) {
    foreach ($db->query($query) as $result) {
      $event = array();
      $event['name']       = $result[1];
      $event['start_date'] = $result[0];
      $event['end_date']   = NULL;
      $event['map']        = _GoogleStaticMap(array(array($result[2], $result[3])));
      $event['latitude']   = $result[2];
      $event['longitude']  = $result[3];
      $event['private']    = user_access('access DataBridge private') ? '[download_link]' : '';
      $event['open']       = 'Wildlife sampling trip. ' . $result[4] . ' samples collected.';
      array_push($events, $event);
    }
  } 
  return $events;
}

/**
 * Returns a summary of a SamplingTrip defined by date and location.
 *
 * @param int $date
 *   The date in days
 * @param string $location
 *   The name of the location 
 *
 * @return array(array) $data
 *   An array of arrays representing a spreadsheet of information.
*/
function DataBridge_SamplingTrip($sampling_day, $location) {
  // Variables 
  $db = new _DataBridgeInterface();
  $header = array("Animal ID", "Location", "Species", "Status", "Sex", "Owner", );
  $last_date = NULL;
  $location = str_replace('_', ' ', $location);  
  
  // Queries, some don't make much sense 
  $get_earlier = "SELECT TO_DAYS(VisitDate) days, 
                         DATE_FORMAT(VisitDate, '%Y-%m-%d') VisitDate 
                  FROM samples 
                  WHERE AnimalID LIKE 'AVD%' 
                    AND project = 5 
                    AND VisitDate != '0000-00-00' 
                    AND TO_DAYS(VisitDate) <= " . $sampling_day . " 
                    AND origin = '" . $location . "' 
                  GROUP BY TO_DAYS(VisitDate)
                  ORDER BY TO_DAYS(VisitDate) ASC;";  
  
  if ($db->open()) {
    // Get earlier sampling trips to the same location.
    $earlier = array();
    foreach ($db->query($get_earlier) as $result) {
      array_push($earlier, $result[0]);
      array_push($header, $result[1]);
      $last_date = $result[1];
    }
    
    // Defining the rest of the queries and variables now that we know the 
    // history
    $sheet = array(0 => $header);
    $where     = "WHERE s.AnimalID LIKE 'AVD%' 
                    AND s.project = 5 
                    AND TO_DAYS(s.VisitDate) IN (".implode(', ', $earlier).") 
                    AND s.origin = '" . $location . "' 
                  GROUP BY s.AnimalID ORDER BY s.AnimalID;";
    $from      = "FROM samples s 
                       JOIN custom_animals ca ON s.AnimalID = ca.animal_id 
                       JOIN organisms o ON ca.organism = o.org_id 
                       LEFT JOIN elisaTest et ON et.sampleID = s.count"; 
    $base_info = "SELECT s.AnimalID, 
                         s.origin, 
                         o.org_name, 
                         ca.status, 
                         ca.sex, 
                         ca.owner " . $from . " " . $where;
    $get_elisa = "IF(SUM(IF(et.STATUS = 'POSITIVE', 1, 0)), '+', IF(SUM(IF(et.STATUS = 'NEGATIVE', 1, 0)), '-', '')) 'Elisa' ";
    
    // fill out the sheet with base information
    foreach ($db->query($base_info) as $result) {
      array_push($sheet, $result);
    }
    
    $length = count($sheet[2]);
    foreach($earlier as $date) {
      $query = "SELECT s.AnimalID, " . $get_elisa . " " . $from . " 
                WHERE s.AnimalID LIKE 'AVD%' 
                  AND s.project = 5 
                  AND TO_DAYS(s.VisitDate) = " . $date . "
                  GROUP BY s.AnimalID;";
      foreach ($db->query($query) as $result) {
        for ($i = 0; $i < count($sheet); $i++) {
          if ($sheet[$i][0] == $result[0]) {
            array_push($sheet[$i], $result[1]);
            break;
          }
        }
      }  
      _balance_sheet($sheet, ++$length);
    } 
    $length = count($sheet[2]);
  }
  return array(str_replace(' ', '_', $location) . '_' . $last_date . '.xls', $sheet);
}

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



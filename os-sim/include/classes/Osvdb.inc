<?php
/*****************************************************************************
*
*    License:
*
*   Copyright (c) 2003-2006 ossim.net
*   Copyright (c) 2007-2009 AlienVault
*   All rights reserved.
*
*   This package is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; version 2 dated June, 1991.
*   You may not use, modify or distribute this program under any other version
*   of the GNU General Public License.
*
*   This package is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this package; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
*   MA  02110-1301  USA
*
*
* On Debian GNU/Linux systems, the complete text of the GNU General
* Public License can be found in `/usr/share/common-licenses/GPL-2'.
*
* Otherwise you can read it here: http://www.gnu.org/licenses/gpl-2.0.txt
****************************************************************************/
/**
* Class and Function List:
* Function list:
* - Osvdb()
* - get_id()
* - get_title()
* - get_classifications()
* - get_disclosure_date()
* - get_description()
* - get_technical_description()
* - get_products()
* - get_solution()
* - get_manual_test()
* - get_external_refs()
* - get_credits()
* - get_status()
* - sanity()
* - get_osvdbid_by_nessusid()
* - get_osvdb()
* - search_classifications()
* - search_external_refs()
* - search_products()
* - get_ext_txt()
* Classes list:
* - Osvdb
*/
class Osvdb {
    var $id;
    var $title;
    var $disclosure_date;
    var $description;
    var $technical_description;
    var $classifications;
    var $products;
    var $solution;
    var $manual_test;
    var $external_refs;
    var $credits;
    var $status;
   
	function Osvdb($id, $title, $disclosure_date, $description, $technical_description, $classifications, $products, $solution, $manual_test, $external_refs, $credits, $status) {
        $this->id = $id;
        $this->title = $title;
        $this->disclosure_date = $disclosure_date;
        $this->description = $description;
        $this->technical_description = $technical_description;
        $this->classifications = $classifications;
        $this->products = $products;
        $this->solution = $solution;
        $this->manual_test = $manual_test;
        $this->external_refs = $external_refs;
        $this->credits = $credits;
        $this->status = $status;
    }
    
	function get_id() {
        return $this->id;
    }
    
	function get_title() {
        return $this->title;
    }
    
	function get_classifications() {
        return $this->classifications;
    }
    
	function get_disclosure_date() {
        return $this->disclosure_date;
    }
    
	function get_description() {
        return $this->description;
    }
    
	function get_technical_description() {
        return $this->technical_description;
    }
    
	function get_products() {
        return $this->products;
    }
    
	function get_solution() {
        return $this->solution;
    }
    
	function get_manual_test() {
        return Osvdb::sanity($this->manual_test);
    }
    
	function get_external_refs() {
        return $this->external_refs;
    }
    
	function get_credits() {
        return $this->credits;
    }
    
	function get_status() {
        return $this->status;
    }
    
	function sanity($text) {
		require_once("classes/Util.inc");
		$tmp = Util::htmlentities($text);
        $tmp = preg_replace("/[^\d{5}];/", "<br/>", nl2br($tmp));
        return $tmp;
	}
    
	function get_osvdbid_by_nessusid($conn_ossim, $nessus_id) {
        $sql = "SELECT plugin_sid FROM plugin_reference WHERE reference_id = 3001 AND plugin_id = 5003 AND reference_sid = ?";
       
	   $params = array(
            $nessus_id
        );
        if (!$rs = $conn_ossim->Execute($sql, $params)) {
            print $conn_ossim->ErrorMsg();
        }
        return $rs->fields["plugin_sid"];
    }
    function get_osvdb($conn_osvdb, $osvdb_id) {
        $sql = "SELECT v.osvdb_id, osvdb_title,disclosure_date FROM vuln v WHERE v.osvdb_id = ?";
        $params = array(
            $osvdb_id
        );
        if (!$rs = $conn_osvdb->Execute($sql, $params)) die($conn_osvdb->ErrorMsg());
        if ($rs->EOF) {
            return null;
        }
        $osvdb = new Osvdb($osvdb_id, $rs->fields["osvdb_title"], $rs->fields["disclosure_date"], Osvdb::get_ext_txt($conn_osvdb, $osvdb_id, 2) , Osvdb::get_ext_txt($conn_osvdb, $osvdb_id, 5) , Osvdb::search_classifications($conn_osvdb, $osvdb_id) , Osvdb::search_products($conn_osvdb, $osvdb_id) , Osvdb::get_ext_txt($conn_osvdb, $osvdb_id, 1) , Osvdb::get_ext_txt($conn_osvdb, $osvdb_id, 3) , Osvdb::search_external_refs($conn_osvdb, $osvdb_id) , array() , '');
        return $osvdb;
    }
    function search_classifications($conn, $id) {
        $classifications_name = array(
            'location_physical' => '1',
            'location_local' => 'Local/Shell Access Required',
            'location_remote' => 'Remote/Network Access Required',
            'location_dialup' => '1',
            'location_unknown' => '1',
            'attack_type_auth_manage' => 'Authentication Attack',
            'attack_type_crypt' => '1',
            'attack_type_dos' => 'Denial Of Service Attack',
            'attack_type_hijack' => '1',
            'attack_type_info_disclose' => 'Information Disclosure Attack',
            'attack_type_infrastruct' => '1',
            'attack_type_input_manip' => 'Input Manipulation',
            'attack_type_miss_config' => 'Misconfiguration Problem',
            'attack_type_race' => '1',
            'attack_type_other' => '1',
            'attack_type_unknown' => '1',
            'impact_confidential' => 'Loss Of Confidentiality',
            'impact_integrity' => 'Loss Of Integrity',
            'impact_available' => 'Loss Of Availability',
            'impact_unknown' => '1',
            'exploit_available' => 'Exploit Available',
            'exploit_unavailable' => '1',
            'exploit_rumored' => '1',
            'exploit_unknown' => 'Exploit Unknown',
            'vuln_verified' => 'Verified',
            'vuln_myth_fake' => '1',
            'vuln_best_prac' => 'Best Practice',
            'vuln_concern' => '1',
            'vuln_web_check' => 'Web Related'
        );
        $params = array(
            $id
        );
        $sql = "SELECT 
                    location_physical,
                    location_local,
                    location_remote,
                    location_dialup,
                    location_unknown,
                    attack_type_auth_manage,
                    attack_type_crypt,
                    attack_type_dos,
                    attack_type_hijack,
                    attack_type_info_disclose,
                    attack_type_infrastruct,
                    attack_type_input_manip,
                    attack_type_miss_config,
                    attack_type_race,
                    attack_type_other,
                    attack_type_unknown,
                    impact_confidential,
                    impact_integrity,
                    impact_available,
                    impact_unknown,
                    exploit_available,
                    exploit_unavailable,
                    exploit_rumored,
                    exploit_unknown,
                    vuln_verified,
                    vuln_myth_fake,
                    vuln_best_prac,
                    vuln_concern,
                    vuln_web_check
                FROM 
                    vuln 
                WHERE 
                    osvdb_id = ?";
        if (!$rs = $conn->Execute($sql, $params)) die($conn->ErrorMsg());
        $classifications_r = $rs->GetArray();
        foreach($classifications_r[0] as $key => $value) {
            if ($value == 1) {
                if (isset($classifications_name[$key]) && $classifications_name[$key] !== null) $classifications[] = $classifications_name[$key];
            }
        }
        return $classifications;
    }
    function search_external_refs($conn, $id) {
        $sql = "SELECT 
                    type_name, 
                    ref_value 
                FROM 
                    ext_ref r,
                    ext_ref_type t,
                    ext_ref_value v 
                WHERE 
                    t.type_id=v.type_id 
                AND 
                    v.value_id=r.value_id 
                AND
                    r.osvdb_id = ?";
        $params = array(
            $id
        );
        if (!$rs = $conn->Execute($sql, $params)) die($conn->ErrorMsg());
        $lista = array();
        while (!$rs->EOF) {
            $lista[] = $rs->fields['type_name'] . " " . $rs->fields['ref_value'];
            $rs->MoveNext();
        }
        return $lista;
    }
    function search_products($conn, $id) {
        $sql = "SELECT 
                    vendor_name,
                    base_name,
                    version_name,
                    type_name
                FROM 
                    object o,
                    object_correlation c, 
                    object_base b, 
                    object_version v, 
                    object_vendor ve,
                    object_affect_type a 
                WHERE 
                    o.type_id=a.type_id 
                AND 
                    c.vendor_id=ve.vendor_id 
                AND 
                    c.base_id=b.base_id 
                AND 
                    c.version_id=v.version_id 
                AND 
                    c.corr_id=o.corr_id
                AND
                    osvdb_id = ?";
        $params = array(
            $id
        );
        if (!$rs = $conn->Execute($sql, $params)) die($conn->ErrorMsg());
        $lista = array();
        while (!$rs->EOF) {
            $lista[] = $rs->fields['vendor_name'] . " " . $rs->fields['base_name'] . " " . $rs->fields['version_name'] . " (" . $rs->fields['type_name'] . ")";
            $rs->MoveNext();
        }
        return $lista;
    }
    function get_ext_txt($conn, $osvdb_id, $type) {
        $sql = "SELECT
                    text
                FROM
                    ext_txt
                WHERE
                    osvdb_id = ? 
                AND
                    type_id = ?";
        $params = array(
            $osvdb_id,
            $type
        );
        if (!$rs = $conn->Execute($sql, $params)) die($conn->ErrorMsg());
        if ($rs->EOF) {
            return "";
        }
        return $rs->fields["text"];
    }
}
?>


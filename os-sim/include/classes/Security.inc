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
* - html_vul_scan()
* - ossim_valid()
* - ossim_set_error()
* - ossim_error()
* - valid_ip()
* - valid_cidr()
* - htm()
* - strip()
* - required()
* - GET()
* - POST()
* - REQUEST()
* Classes list:
*/
ini_set('session.bug_compat_warn','off');
require_once 'ossim_error.inc';
require_once 'IDS/Init.php';
require_once 'classes/Util.inc'; 

define("MIN_ECHAR_VALUE", 1000);

function clean_echars($data)
{
	if ( is_array($data) )
	{
		foreach ($data as $i => $d)
		{
			$data[$i] = clean_echars($d);
		}
	}
	else
	{
		if ( preg_match_all('/&#(\d{4,5});/', $data, $match) )
		{
			foreach ($match[1] as $k => $v)
				$data = str_replace("&#".$v.";", "", $data);
		}
	}
	
	return $data;
}

function get_echars($data)
{
	$echars = ( preg_match_all('/&#(\d{4,5});/', $data, $match) != false ) ? $match[1] : array();
	return $echars;
}

function ids(){
	try {
		
		$general_data = array($_GET, $_POST, $_COOKIE, $_FILES);
				
		foreach ($general_data as $i => $type)
		{
			foreach ($type as $k => $v)
			{
				$v = (mb_detect_encoding($v." ",'UTF-8,ISO-8859-1') == 'UTF-8') ? Util::utf8entities($v) : $v;
				$general_data[$i][$k] = clean_echars($v);
			}
		}
		
		$request = array(
			'GET'    => $general_data[0],
			'POST'   => $general_data[1],
			'COOKIE' => $general_data[2],
			'FILES'  => $general_data[3]
		);
		
		
		/*$request = array(
			'GET' => $_GET,
			'POST' => $_POST,
			//'REQUEST' => $_REQUEST,
			'COOKIE' => $_COOKIE,
			'FILES' => $_FILES
			//'URI' => rawurlencode($_SERVER["REQUEST_URI"])
		);*/
		
		$init = IDS_Init::init('/usr/share/ossim/include/php-ids.ini');
		$init->config['General']['scan_keys'] = true;
		$ids = new IDS_Monitor($request, $init);
		$result = $ids->run();
		
		if (!$result->isEmpty()) 
		{
			//require_once 'IDS/Log/Database.php';
			require_once 'IDS/Log/Composite.php';
			require_once 'IDS/Log/File.php';

			$compositeLog = new IDS_Log_Composite();
			//$compositeLog->addLogger(IDS_Log_Database::getInstance($init));
			$compositeLog->addLogger(IDS_Log_File::getInstance($init));
			$compositeLog->execute($result);
			die(ossim_error("Sorry, operation was not completed due to security reasons. An attack attempt has been logged to the system.<br/>".utf8_encode($result), "ossim_alert"));
		}
	} 
	catch (Exception $e) 
	{
		printf('An error occured: %s',$e->getMessage());
	}
}

function ids_valid($data){
	try {
		$request = array(
			'INPUT' => $data
		);
		$init = IDS_Init::init('/usr/share/ossim/include/php-ids.ini');
		$init->config['General']['scan_keys'] = true;
		$ids = new IDS_Monitor($request, $init);
		$result = $ids->run();
		if (!$result->isEmpty()) {
			require_once 'IDS/Log/Composite.php';
			require_once 'IDS/Log/File.php';

			$compositeLog = new IDS_Log_Composite();
			$compositeLog->addLogger(IDS_Log_File::getInstance($init));
			$compositeLog->execute($result);
			die(ossim_error("Sorry, operation was not completed due to security reasons. An attack attempt has been logged to the system.<br/>".$result, "ossim_alert"));
		}
	} catch (Exception $e) {
		printf('An error occured: %s',$e->getMessage());
	}
}

function html_vul_scan($string, $is_get) {
    $scan_for = array(
        '/<\s*script[^>]*>/Ui'
    );
    if (is_array($string)) {
        foreach($string as $string_piece) {
            foreach($scan_for as $s) {
                if ($is_get) {
                    if (preg_match("/^(\<=|\>=|\>|\<|\<\>)$/",$string_piece)) {
                        continue;
                    }
                    if ($string_piece != strip_tags($string_piece)) {
                        die(ossim_error("HTTP GET " . _("param not allowed for security reasons"), "ossim_alert"));
                    }
                }
                if (preg_match($s, $string_piece)) {
                    die(ossim_error("HTTP POST " . _("param not allowed for security reasons"), "ossim_alert"));
                }
            }
        }
    } else {
        $string = html_entity_decode($string);
        foreach($scan_for as $s) {
            if ($is_get) {
                if (preg_match('/^<<./', $string) || preg_match("/^(\<=|\>=|\>|\<|\<\>)$/", $string)) {
                    continue;
                }
                if ($string !== strip_tags($string)) {
                    die(ossim_error("HTTP GET " . _("param not allowed for security reasons"), "ossim_alert"));
                }
            }
            if (preg_match($s, $string)) {
                die(ossim_error("HTTP POST " . _("param not allowed for security reasons"), "ossim_alert"));
            }
        }
    }
}
/*
* Scan all HTTP vars disallowing common exploits like XSS
*/
$_html_vars_checked = true;
if (!isset($_html_vars_checked)) {
    $_is_get = true;
    foreach(array(
        $_GET,
        //$_REQUEST,
        $_POST
    ) as $globalvar) {
        foreach($globalvar as $key => $value) {
            if (is_array($value)) {
                foreach($value as $v) {
                    html_vul_scan($v, $_is_get);
                }
            } else {
                html_vul_scan($value, $_is_get);
            }
        }
        $_is_get = false;
    }
    
}
/*

Usage examples:

//--- simple validation  ---//

// Note that by default ossim_valid() doesn't permit empty vars.
// It implicitly adds the rule OSS_NOTNULL. If the var is optional
// or can be an empty value, add the rule OSS_NULLABLE

if (!ossim_valid($id, OSS_DIGIT)) {
echo ossim_error();
}

//--- complex validations ---//

// The special rule 'error:BLAHBLAH', means that in case
// the var does not validate against the other rules,
// an error will be registered (internally using ossim_set_error()).
// You can later check and retrieve the error by calling ossim_error()

// The special rule 'illegal:Title' would create automatically an
// error message in the form:
// "Error for the 'Title' field (<what caused the error>)"

// You can add your custom validation rules too, read the comments
// at the ossim_valid() function

$vals = array(
'id'   => array(OSS_DIGIT, 'error:Invalid ID, only numbers allowed'),
'name' => array(OSS_LETTER, OSS_SPACE, 'error:Only letters and spaces')
);

$name = isset($_GET['name']) ? $_GET['name'] : null;

ossim_valid($id, $vals['id']);
ossim_valid($name, $vals['name']);
if (ossim_error()) {
echo ossim_error();
}

*/
define('OSS_DIGIT', '0-9');
define('OSS_LETTER', 'A-Za-z');
define('OSS_SPACE', ' ');
define('OSS_SCORE', '_\-');
define('OSS_DOT', '\.');
define('OSS_MIDDLEDOT', '\·');
define('OSS_COLON', ':');
define('OSS_AT', '@');
define('OSS_MAC', 'func:valid_mac');
define('OSS_BRACKET', '\[\]\{\}');
define('OSS_PUNC_EXT', '\'"\.\!#\$%\^&\*_\-\=\+\:;,@\[\]\{\}\|\?\\\(\)\/���' . OSS_SPACE);
// Temporary fix, need to check out each and every OSS_PUNC use throughout the code
define('OSS_PUNC', '\.,:@_\-\/\?&\=' . OSS_SPACE);
define('OSS_SLASH', '\/');
define('OSS_URL', '\.\/');
define('OSS_EALPHA_LOWER', '���������������������������������');
define('OSS_EALPHA_UPPER', '���������������ܾ�������զ��Ǽ���');
define('OSS_NOECHARS', 'no_echars');
define('OSS_ALPHA', OSS_DIGIT . OSS_LETTER . OSS_EALPHA_LOWER . OSS_EALPHA_UPPER);
define('OSS_CRONTAB', OSS_DIGIT . OSS_LETTER . '\*\-,\/'); // Somewhat dangerous, if "/" is allowed don't allow spaces, tabs or anything similar.
define('OSS_NL', '\r\n');
define('OSS_INPUT', OSS_ALPHA . OSS_PUNC); // for most input text fields
define('OSS_TEXT', OSS_ALPHA . OSS_PUNC . OSS_NL . '\)\(\'"'); // for most textarea fields
define('OSS_USER', OSS_LETTER . OSS_DIGIT . OSS_SCORE . OSS_DOT); // for validating logins/users
define('OSS_NET_NAME', OSS_LETTER . OSS_DIGIT . OSS_SCORE . OSS_DOT . OSS_COLON); // for network names
define('OSS_NOTNULL', 'rule:OSS_NOTNULL'); // default for all vars
define('OSS_NULLABLE', 'rule:OSS_NULLABLE'); // value can be null, false, missing
define('OSS_PORT', 'func:valid_port');  //	Valid port number or pair port-protocol. Port range: [0 - 65535]; Separators allowed : -, _, #; Protocols allowed : tcp, udp, icmp
define('OSS_PROTOCOL', 'tcp | udp | icmp');
define('OSS_IP_ADDR', 'func:valid_ip');
define('OSS_IP_ADDR_0', 'func:valid_ip_2'); //(0.0.0.0. allowed)
define('OSS_IP_CIDR', 'func:valid_cidr');
define('OSS_IP_ADDRCIDR', 'func:valid_ipcidr'); // is ip OR cidr format
define('OSS_IP_ADDRCIDR_0', 'func:valid_ipcidr_2'); // is ip OR cidr format (0.0.0.0. allowed)
define('OSS_SEVERAL_IP_ADDRCIDR_0', 'func:valid_ipcidr_3'); // several IPs OR CIDRs formats (0.0.0.0. allowed)
define('OSS_FQDNS', 'func:valid_fqdns');
define('OSS_QUOTES', 'func:valid_quotes');
define('OSS_DAY', 'func:valid_day');
define('OSS_DATETIME', 'func:valid_datetime');
define('OSS_MAIL_ADDR', 'regex:[a-zA-Z0-9_\-\.+]+@[a-zA-Z0-9_\-\.]+\.[a-zA-Z]+');
define('OSS_ERROR', _("unkown error")); // default error message
define('OSS_SQL', 'illegal:(select |union |update |delete |insert )');
define('OSS_URL_ADDRESS', 'func:valid_url');
define('OSS_WEEKDAY', 'func:valid_weekday');
define('OSS_SHA1', OSS_LETTER . OSS_DIGIT);

// don't use
$GLOBALS['ossim_last_error'] = false; // track the latest error
/*
* Function for validating variables against the given rules
*
* @param1 The var to check
*
* @param2 Array with a list of validations
*         Ex: ossim_valid($foo, array(OSS_AT, OSS_SPACE))
* or
*
* @param2 to @paramN Checks
*         Ex: ossim_valid($foo, OSS_AT, OSS_SPACE)
*
* A check could be:
*      1) a string that would be converted into a regex. Ex. '0-9' -> '/[^0-9]/'
*      2) func:<func_name> Will call <func_name> passing var as the only arg.
*                          Should return true or false.
*                          Func can also be in the form <Class::method>
*      3) regex:<regex>   Will check var against the regex.
*                         Ex. '^[0-9]+' -> '/^[0-9]+/'
*      4) rule:<rules>    Not suitable for users
*/
function ossim_valid() {
	require_once("classes/Util.inc");
    static $cons = false;
    if (func_num_args() < 2) {
        die('Wrong number of params, usage: ' . __FUNCTION__ . '($var, $rules)');
    }
    if (!$cons) 
		$cons = get_defined_constants();
    
	$parms = func_get_args();
    $v_var = $parms[0]; // the var to be validated
    
	if ( is_object($v_var) ) {
        return ossim_set_error(_("Wrong datatype passed, expected string"));
    }
	else if ( !is_array($v_var) )
	{
		$subject[0] = $v_var;
	}
	else
		$subject = $v_var;
	
	array_shift($parms);
	
	
    // array of params instead of list of params
    // ej: ossim_valid($foo, array(OSS_DIGIT, OSS_FOO))
    if (func_num_args() == 2 && is_array($parms[0])) {
        $parms = $parms[0];
    }
    $error_msg   = OSS_ERROR;
    $val_notnull = true; // by default do not allow empty vars
    $count       = count($parms);
	
	
    for ($i = 0; $i < $count; $i++) {
        //
        // error:
        //
        if (preg_match('/^error:(.+)$/', $parms[$i], $m)) {
            $error_msg = $m[1];
            unset($parms[$i]);
            //
            // illegal:
            //
            
        } elseif (preg_match('/^illegal:(.+)$/', $parms[$i], $m)) {
            $error_msg = sprintf(_("Error in the '%s' field") , $m[1]);
            unset($parms[$i]);
            //
            // rule: (NULL permitted or not)
            //
            
        } elseif (preg_match('/^rule:(.+)$/', $parms[$i], $m)) {
            if ($m[1] == 'OSS_NOTNULL') {
                $val_notnull = true;
            } elseif ($m[1] == 'OSS_NULLABLE') {
                $val_notnull = false;
            }
            unset($parms[$i]);
        }
    }
	
	if ( empty($subject) )
		$subject[0] = array();
		
		
	
	foreach ($subject as $k => $v)
	{
		if (is_object($v) || is_array($v))
			return ossim_set_error("Wrong datatype passed, expected string $v");
					
		//
		// NULL check
		//
		if ($v === null || $v === '') {
			if ($val_notnull) {
				return ossim_set_error("$error_msg (" . _("missing required field") . ")");
			} else {
				return true;
			}
		}
		
			
		$v = ( mb_detect_encoding($v." ",'UTF-8,ISO-8859-1') == 'UTF-8' ) ? mb_convert_encoding($v, 'ISO-8859-1', 'UTF-8') : $v ;
		
		$val_str         = '';
		$oss_echars      = false;
		$no_echars       = false;
		$echars          = get_echars($v);
		$exists_echars   = ( is_array($echars) && !empty($echars) ) ? true : false;

		foreach($parms as $p) {
			//
			// func:
			//
			if (preg_match('/^func:(.+)$/', $p, $m)) {
				$func = $m[1];
				if (strpos($func, '::') !== false) { // static Class::method()
					$func = explode('::', $func);
				}
				if (!is_callable($func)) {
					die("The validation function '$func' doesn't exist");
				}
				if (!call_user_func($func, $v)) {
					return false;
				}
				continue;
			}
			//
			// regex:
			//
			if (preg_match('/^regex:(.+)$/', $p, $m)) {
				if (!preg_match('/^' . $m[1] . '$/', $v)) {
					$v= Util::htmlentities($v);
					return ossim_set_error($error_msg . " (the string '<b>$v</b>' has not a valid format)");
				}
				continue;
			}
			//
			// OSS_* constants
			//
			
			if ( $p == OSS_NOECHARS )
				$no_echars = true;
			else
				$val_str.= $p;
		}
				
				
		if ( stripos($val_str, OSS_LETTER) !== false )
				$oss_echars = true;
		
		$clean_v = $v;
		
		if ( $exists_echars == true && $val_str )
		{
			if ( $oss_echars == false  )
				return ossim_set_error($error_msg. "<br/>String: <b>" . sprintf(_("\"%s\"")."</b> "._("not allowed."), $v));
			else
			{
				if ( $no_echars == true )
					return ossim_set_error($error_msg. "<br/>String: <b>" . sprintf(_("\"%s\"")."</b> "._("not allowed."), $v));
				else
				{
					
					foreach ($echars as $k => $char)
					{
						if ($char < MIN_ECHAR_VALUE)
							return ossim_set_error($error_msg. " " . sprintf(_("(\"%s\" not allowed)") , "&#".$char.";") . "<br/>Introduced string: '<b>$v</b>'");
					}
					
					$clean_v = clean_echars($v);
				}
			}
		}
		
		
		
		if ($val_str && preg_match("/[^$val_str]/", $clean_v, $m))
		{
			$not = $m[0];
			if ($not == " ") $not = _("spaces");
			if ($not == "\r" || $not == "\n") $not = _("breaklines");
			
			$v   = Util::htmlentities($v);
			$not = Util::htmlentities($not);
						
			if (preg_match("/Password/i",$error_msg))
				return ossim_set_error($error_msg. " " . sprintf(_("(\"%s\" not allowed)") , $not)); 
			else 
				return ossim_set_error($error_msg. " " . sprintf(_("(\"%s\" not allowed)") , $not) . "<br/>Introduced string: '<b>$v</b>'");
			
		}
	}
	
	/*
	echo "<pre>";
	print_r(preg_match("/[^$val_str]/", $clean_v, $m));
	echo "<br/>";
	print_r($val_str."<=>".$clean_v);
	echo "</pre>";
	*/	
	
    return true;
}
function ossim_set_error($error) {
    $GLOBALS['ossim_last_error'] = $error;
    return false;
}

function ossim_clean_error() {
    $GLOBALS['ossim_last_error'] = false;
    return true;
}

function ossim_get_error() {
    return $GLOBALS['ossim_last_error'];
}

function ossim_get_error_clean() {
    $error=$GLOBALS['ossim_last_error'];
	$normalize = explode("<br/>", $error);
	if (count($normalize) == 2 )
		$error = $normalize[0].".&nbsp;&nbsp;".$normalize[1];
		
	return $error;
		
}


function ossim_error($error_msg = null, $error_type='ERROR', $error_code=null) {
	include_once 'ossim_error.inc';
	
	switch ($error_type)
	{
		case "ERROR":
			$ossim_eclass = new OssimError($error_msg);
		break;
		
		case "WARNING":
			$ossim_eclass = new OssimWarn($error_msg);
		break;
		
		case "NOTICE":
			$ossim_eclass = new OssimNotice($error_msg);
		break;
		
		default:
			$ossim_eclass = new OssimError($error_msg);
	}
	
	$error_msg  = ($error_msg !== null) ? $error_msg : $GLOBALS['ossim_last_error'];
	
	if (!$error_msg) {
        return false;
    }	
	   
    return "<div style='width:95%; margin:auto;'>".$ossim_eclass->get($error_code, array( $error_msg ))."</div>";
}

function validate_form_fields($type, $validate)
{
	require_once('classes/Util.inc');
	$values            = array();
	$validation_errors = array();
	$validate_rules    = array();
	
		
	if ($type == 'POST')
	{
		$validate_rules = $validate;
		foreach ($_POST as $k => $v)
		{
			if ( !is_array($v) )
				$values[$k]= ( mb_detect_encoding($v." ",'UTF-8,ISO-8859-1') == 'UTF-8' )  ? Util::utf8entities(POST($k)) : POST($k);
			else
			{
				foreach ($v as $i => $j)
					$aux[$i] = ( mb_detect_encoding($v." ",'UTF-8,ISO-8859-1') == 'UTF-8' )  ? Util::utf8entities($j) : $j;
				
				$values[$k] = $aux;
			}
		}
	}
	else if ($type == 'GET')
	{
		$k = $_GET['name'];
		
		if ( isset($validate[$k]) )
		{
			$validate_rules[$k] = $validate[$k];
			$values[$k]         = ( mb_detect_encoding($v." ",'UTF-8,ISO-8859-1') == 'UTF-8' ) ? Util::utf8entities(GET($k)) : GET($k);
		}
		
	}
	else
		return 1;
	

	foreach ($validate_rules as $k => $v)
	{
		if ( !isset($values[$k]) )
			$value = null;
		else
			$value = $values[$k];
			
		eval("ossim_valid(\$value, ".$validate_rules[$k]['validation'].", '".$validate_rules[$k]['e_message']."');");
		
		if ($GLOBALS['ossim_last_error'] !== false )
		{
			$validation_errors[] = $GLOBALS['ossim_last_error'];
			ossim_clean_error();
		}
		
	}
		
	return $validation_errors;

}

function valid_cidr($cidrs)
{
	$valid_sep   = array (",", "\n", " ");
	$current_sep = '';
	
	foreach ( $valid_sep as $k => $v )
	{
		if ( preg_match("/$v/", $cidrs) )
		{
			$current_sep = $v;
			break;
		}
	}
	
	$array_cidr = ( $current_sep == '') ? array($cidrs) : explode($current_sep, $cidrs);
	
	foreach ($array_cidr as $k => $v)
	{
		$v = trim($v);
		if (!preg_match("/^0.0.0.0/",$v) && preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)\/(\d+)$/', $v, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 && $m[5] > 0 && $m[5] <= 32) 
			continue;
		else
		{
			ossim_set_error(_("Invalid IP/Mask address. Format allowed: nnn.nnn.nnn.nnn/nn") . "<br/>Entered CIDR: '<strong>".$v."</strong>'");
			return false;
		}
	}
	return true;
}
	
function valid_port($port)
{
	$valid_protocol = array ("tcp", "udp", "icmp");
	$valid_sep = array ("-", "_", "#");
	$current_sep = '';
	
	foreach ( $valid_sep as $k => $v )
	{
		if ( preg_match("/$v/", $port) )
		{
			$current_sep = $v;
			break;
		}
	}
	
	if ( empty($current_sep) )
	{
		$port = trim($port);
		if ( (preg_match('/^\d+$/', $port) && $port>=0 && $port<=65535) ) 
			return true;
		else
		{
			ossim_set_error(_("Port number isn't valid") . "<br/>Entered port: '<b>$port</b>'");
			return false;
		}
	
	}
	else
	{
		$pair_pp = explode($current_sep, $port);
		
		$port     = trim($pair_pp[0]);
		$protocol = trim(strtolower($pair_pp[1]));
		
		if ( (preg_match('/^\d+$/', $port) && $port>=0 && $port<=65535) && (in_array($protocol, $valid_protocol)) )
			return true;
		else
		{
			ossim_set_error(_("Pair port - protocol isn't valid") . "<br/>Entered pair: '<b>$port-$protocol</b>'");
			return false;
		}
	}
}

function valid_ip($ip) {
    
	if ( !is_array($ip) )
	{
		if ($ip!="0.0.0.0" && preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 )
			return true;
		
		ossim_set_error(_("Invalid IP address. Format allowed: nnn.nnn.nnn.nnn") . "<br/>Entered ip: '<b>$ip</b>'");
		return false;
	}
	else
	{
		foreach ($ip as $k => $v)
		{
			if ($v!="0.0.0.0" && preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $v, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 )
				continue;
			else
			{
				ossim_set_error(_("Invalid IP address. Format allowed: nnn.nnn.nnn.nnn") . "<br/>Entered ip: '<b>$v</b>'");
				return false;
			}
		}
		return true;
	}
}

// Address 0.0.0.0 allowed

function valid_ip_2($ip) {
    
	if ( !is_array($ip) )
	{
		if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 )
			return true;
		
		ossim_set_error(_("Invalid IP address. Format allowed: nnn.nnn.nnn.nnn") . "<br/>Entered ip: '<b>$ip</b>'");
		return false;
	}
	else
	{
		foreach ($ip as $k => $v)
		{
			if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $v, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 )
				continue;
			else
			{
				ossim_set_error(_("Invalid IP address. Format allowed: nnn.nnn.nnn.nnn") . "<br/>Entered ip: '<b>$v</b>'");
				return false;
			}
		}
		return true;
	}
}


function valid_ipcidr($ip) {
   	
	if (!preg_match("/^0.0.0.0/",$ip) && preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)\/(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 && $m[5] > 0 && $m[5] <= 32) {
        return true;
    }
    else if ($ip!="0.0.0.0" && preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255) {
        return true;
    }
    
    ossim_set_error(_("Invalid IP/CIDR address. Format allowed: nnn.nnn.nnn.nnn/nn?") . "<br/>Entered ip: '<b>$ip</b>'");
    return false;
}

// Address 0.0.0.0 allowed

function valid_ipcidr_2($ip) {
   	
	if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)\/(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 && $m[5] > 0 && $m[5] <= 32) {
        return true;
    }
    else if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', $ip, $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255) {
        return true;
    }
    
    ossim_set_error(_("Invalid IP/CIDR address. Format allowed: nnn.nnn.nnn.nnn/nn?") . "<br/>Entered ip: '<b>$ip</b>'");
    return false;
}

// Address 0.0.0.0 allowed

function valid_ipcidr_3($ip) {
    $ok   = true;
    
    $ips_cidrs = explode(",", $ip);
    foreach($ips_cidrs as $data) if ($ok){
        if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)\/(\d+)$/', trim($data), $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 && $m[5] > 0 && $m[5] <= 32 ) {
            continue;
        }
        else if ( preg_match('/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/', trim($data), $m) && $m[1] <= 255 && $m[2] <= 255 && $m[3] <= 255 && $m[4] <= 255 )  {
            continue;
        }
        else {
            $ok = false;
        }
    }
    
    if(!$ok) {
        ossim_set_error(_("Invalid IP/CIDR address. Format allowed: nnn.nnn.nnn.nnn/nn?") . "<br/>Entered ip: '<b>$ip</b>'");
        return false;
    }
    else {
        return true;
    }
}

function valid_mac($mac)
{
	if ( preg_match('/^[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}$/i',$mac) )
		return true;
		
	ossim_set_error(_("Invalid MAC address. Format allowed: nn:nn:nn:nn:nn:nn, n=[a-f0-9]") . "<br/>Entered MAC: '<b>$mac</b>'");
    return false;
}



function valid_fqdns($fqdns)
{
	$fqdns       = trim($fqdns);
	$array_fqdns = ( empty($fqdns) ) ? array() :  explode(",", $fqdns);
	
	foreach ($array_fqdns as $k => $v)
	{
		if (ossim_valid(trim($v), OSS_DIGIT, OSS_DOT, OSS_SCORE, OSS_LETTER, OSS_SPACE, 'illegal:' . _("FQDNS/Aliases")) == true) 
			continue;
		else
		{
			ossim_set_error(_("Invalid FQDNS/Aliases")."<br/>Entered FQDNS/Aliases: '<b>$v</b>'");
			return false;
		}
	}
	
	return true;
}

function valid_url($url)
{
	// Scheme
	//$pattern = "/^((https?|ftp)\:\/\/)?";
	$pattern = "/^((https?)\:\/\/)?";

	// User and pass (optional)
	$pattern .= "([a-z0-9+!*(),;?&=\$_.-]+(\:[a-z0-9+!*(),;?&=\$_.-]+)?@)?";

	// Hostname or ip
	$pattern .= "[a-z0-9+\$_-]+(\.[a-z0-9+\$_-]+)*"; // http://x = allowed (ex. http://localhost, http://routerlogin)
	//use only one of the above

	// Port (optional)
	$pattern .= "(\:[0-9]{2,5})?";
	// Path (optional)
	$pattern .= "(\/([a-z0-9+\$_-]\.?)+)*\/?";
	// GET Query (optional)
	$pattern .= "(\?[a-z\+\&\$_.-][a-z0-9;:@\/\&%=\+\$_.-]*)?";
	// Anchor (optional)
	$pattern .= "(#[a-z_.-][a-z0-9+\$_.-]*)?$/";
	
	if ( preg_match($pattern, $url) )
		return true;
	
	ossim_set_error(_("Invalid url. Format allowed: <http[s]|ftp> :// [user[:pass]@] hostname [port] [/path] [?getquery] [anchor]") . "<br/>Entered url: '<strong>$url</strong>'");
    return false;
}


function valid_quotes($value) {
    if (!preg_match('/\'|\"/', $value)) {
        return true;
    }
    ossim_set_error(_("Invalid Quotes. Don't use quotes ' or \""));
    return false;
}

function valid_datetime($value)
{
	if ( strtotime($value)!=FALSE ) {
		return true;
	}
	ossim_set_error(_("Invalid Date-time. Format allowed: YYYY-MM-DD HH:MM:SS") . "<br/>Entered date: '<strong>".$value."</strong>'");
	return false;
}

function valid_day($value)
{
	if ( preg_match('/(\d+)-(\d+)-(\d+)/', $value, $m) )
	{
		$valid_date = checkdate($m[2],$m[3],$m[1]);

		if ($valid_date === true)
			return true;

	}

	ossim_set_error(_("Invalid Date. Format allowed: YYYY-MM-DD") . "<br/>Entered date: '<strong>".$value."</strong>'");
	return false;
}


function valid_weekday($value)
{
	$weekdays = array ( _("Sunday"),
						_("Monday"),
						_("Tuesday"),
						_("Wednesday"),
						_("Thursday"),
						_("Friday"),
						_("Saturday"),
	);
	
	if ( !in_array($value, $weekdays) )
	{
		ossim_set_error(_("Invalid weekday.") . "<br/>Entered day: '<strong>".$value."</strong>'");
		return false;
	}
	
	return true;
	
}



/* Test perms in sensor_ip */
function validate_sensor_perms($conn,$sensor_ip,$sql="") {
    require_once 'classes/Sensor.inc';
    $ossim_sensors = Sensor::get_list($conn,$sql);
    $sensors_list = array();
    foreach ($ossim_sensors as $sensor) $sensors_list[] = $sensor->get_ip();
    return in_array($sensor_ip,$sensors_list);
}

function valid_echars($value)
{
		
	if ( preg_match_all('/&#(\d{4,5});/', $value, $match) )
	{
		
		
		foreach ($match[1] as $k => $v)
		{
			if ($v < 10000)
			{
				ossim_set_error(_("Invalid characters")."<br/>Entered characters: '<b>".Util::htmlentities($text)."</b>'");
			}
				
			$text = str_replace("&#".$v.";", "", $value);
		}
	}
	else
		$text = $value;
		
	
		
	if ( $text != '' )
	{
		$latin_text[0] = "A-Za-z";
		$latin_text[1] = "���������������������������������";
		$latin_text[2] = "���������������ܾ�������զ��Ǽ���";
		$latin_text[3] = "0-9";
		
		$pattern = "/[^".implode("", $latin_text)."]/";
		
		if ( preg_match_all($pattern, $text, $match) )
		{
			ossim_set_error(_("Invalid characters")."<br/>Entered characters: '<b>".Util::htmlentities($text)."</b>'");
		}
	}
	
	
	return true;
	
}


/*
* Safe html escape. Always use that for printing vars comming
* from outside ($_GET, $_POST, databases, etc)
*/
function htm($var) {
    require_once ('classes/Util.inc');
	return nl2br(Util::htmlentities($var));
}
function strip($var) {
    return get_magic_quotes_gpc() ? stripslashes($var) : $var;
}
function required() {
    return '&nbsp;<span class="required" title="' . _("Required field") . '">*</span>';
}
/*
* WARNING: if you use this function, it's a MUST to correct quote
* the values before inserting in the DB. AdoDB provides that, ex:
*
* $get1 = GET('get1');
* $get2 = GET('get2');
* $sql = "INSET INTO foo VALUES (?, ?)";
* $conn->Execute($sql, array($get1, $get2));
*
* $get1 will be automagically quoted against the DB quoting function
* (ex: mysql_real_escape_string()) and the result (ex: 'It\'s tea time')
* will replace the first "?". The same for $get2.
*/
function GET($param) {
    return isset($_GET[$param]) ? strip($_GET[$param]) : null;
}
/*
* @see GET()
*/
function POST($param) {
    if (isset($_POST[$param])) {
        if (!is_array($_POST[$param])) {
            return strip($_POST[$param]);
        } else {
            $ret = array();
            foreach($_POST[$param] as $k => $v) {
                $ret[$k] = strip($v);
            }
            return $ret;
        }
    }
    return null;
}
/*
* @see GET()
*/
function REQUEST($param) {
    return isset($_REQUEST[$param]) ? strip($_REQUEST[$param]) : null;
}
/* TESTS
function test($res, $expected)
{
static $test_num = 0;
echo '#' . ++$test_num;
if ($res != $expected) { echo " FAIL!\n"; return; }
echo " GOOD\n";
}
ini_set('include_path', '../');


test(ossim_valid('24', OSS_DIGIT), true); #1
test(ossim_valid('0.24', OSS_DIGIT), false); #2
test(ossim_valid('hello', OSS_ALPHA), true); #3
test(ossim_valid('hello dasd', OSS_ALPHA), false); #4
test(ossim_valid('', OSS_NOTNULL), false); #5
test(ossim_valid('', OSS_NULLABLE), true); #6
test(ossim_valid('3', OSS_LETTER), false); #7
test(ossim_valid('hello', OSS_LETTER), true); #8
test(ossim_valid('0.1.1.1', OSS_IP_ADDR), true); #9
test(ossim_valid('500.1.1.1', OSS_IP_ADDR), false); #10
test(ossim_valid('hello@hello.com', OSS_MAIL_ADDR), true); #11
test(ossim_valid('hello@hello', OSS_MAIL_ADDR), false); #12
test(ossim_valid('hello@he.llo.', OSS_MAIL_ADDR), false); #13
test(ossim_valid('hello@he.llo.9', OSS_MAIL_ADDR), false); #14
test(ossim_valid('hello@hello.com9', array(OSS_NOTNULL, OSS_MAIL_ADDR)), false); #15
test(ossim_valid('', array(OSS_NULLABLE, OSS_MAIL_ADDR)), true); #16
test(ossim_valid('', OSS_LETTER), false); #17
test(ossim_valid('ho la', OSS_LETTER, OSS_SCORE), false); #18
test(ossim_valid('', OSS_NULLABLE, OSS_IP_ADDR), true); #19
test(ossim_valid('', OSS_IP_ADDR, OSS_NULLABLE), true); #20
test(ossim_valid('', OSS_IP_ADDR), false); #21
test(ossim_valid('123.456.2.3', OSS_IP_ADDR), false); #22
test(ossim_valid('123.220.44.0', OSS_IP_ADDR), true); #23

test(ossim_valid('foo', 'func:no_exists'), true); #should die
//*/
function check_uniqueid($unique_id1,$unique_id2) {
	if ($unique_id1 != "" && $unique_id2 != "" && $unique_id1 == $unique_id2) {
		return 1;
	} else {
		//echo _("Can't do this action for security reasons...");
		return 0;
	}
}

//Check security in passwords

function checkpass($conn, $current_pass, $pass1, $pass2, $user)
{
	require_once "classes/Session.inc";
	require_once "ossim_conf.inc";
	require_once "ossim_db.inc";
	
	$conf         = $GLOBALS["CONF"];
	$conf         = ( !$conf ) ? new ossim_conf() : $conf;
	$info_error   = true;
	$error        = false;
		
	$recent_pass = Log_action::get_last_pass($conn, $user);
	
	
	$parameters['current_pass'] = $current_pass;
	$parameters['pass1'] 	    = $pass1;
	$parameters['pass2'] 	    = $pass2;
	
	
	$validate  = array (
			"current_pass"   => array("validation" => "OSS_ALPHA, OSS_PUNC_EXT"  ,"e_message" => 'illegal:' . _("Current pass")),
			"pass1"          => array("validation" => "OSS_ALPHA, OSS_PUNC_EXT"  ,"e_message" => 'illegal:' . _("New password")),
			"pass2"          => array("validation" => "OSS_ALPHA, OSS_PUNC_EXT"  ,"e_message" => 'illegal:' . _("Rewrite password")),
		);


	foreach ($parameters as $k => $v )
	{
		eval("ossim_valid(\$v, ".$validate[$k]['validation'].", '".$validate[$k]['e_message']."');");
	
		if ( ossim_error() )
		{
			$info_error[] = ossim_get_error();
			$error        = true;
			ossim_clean_error();
		}
	}

	if ( $error == true )
		return $info_error;
	
	$pass_length_min = ($conf->get_conf("pass_length_min", FALSE)) ? $conf->get_conf("pass_length_min", FALSE) : 7;
	$pass_length_max = ($conf->get_conf("pass_length_max", FALSE)) ? $conf->get_conf("pass_length_max", FALSE) : 255;
	
	
	if ( empty($current_pass) )
		return array(_("Current password is empty"));
	
	if ( strcmp($pass1, $pass2) != '0' ) 
		return array(_("Passwords mismatches"));
	
	if (strlen($pass1) < $pass_length_min) 
		return array(_("Password is long enough. The minimum is ").$pass_length_min._(" characters."));
	
	if (strlen($pass1) > $pass_length_max) 
		return array(_("Password is too long. The maximum is ").$pass_length_max._(" characters."));
	
	if (!Session::pass_check_complexity($pass1)) 
		return array(_("Password is not complex enough."));
	
	if (count($recent_pass) > 0 && in_array(md5($pass1),$recent_pass)) 
		return array(_("This password is recently used. Try another."));
	
	if (count($user_list = Session::get_list($conn, "WHERE login = '" . $user . "' and pass = '" . md5($pass1) . "'")) > 0) 
		return array( _("You must change your old password."));
		
	
	return true;
}



// IDS Security
ids();
?>

<?php
/*****************************************************************************
*
*License:
*
*   Copyright (c) 2007-2010 AlienVault
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
include ("base_conf.php");
include ("vars_session.php");
include ("$BASE_path/includes/base_constants.inc.php");
include ("$BASE_path/includes/base_include.inc.php");
include_once ("$BASE_path/base_db_common.php");
include_once ("$BASE_path/base_qry_common.php");
include_once ("$BASE_path/base_stat_common.php");
$db = NewBASEDBConnection($DBlib_path, $DBtype);
$db->baseDBConnect($db_connect_method, $alert_dbname, $alert_host, $alert_port, $alert_user, $alert_password);
$cs = new CriteriaState("base_qry_main.php", "&amp;new=1&amp;submit=" . _QUERYDBP);
$cs->ReadState();

/* Generate the Criteria entered into a human readable form */
$criteria_arr = array();

$tmp_len = strlen($save_criteria);

// META
$criteria_arr['meta']['Sensor'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Plugin'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Plugin Group'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Userdata'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Source Type'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Category'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Signature'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Sig Class'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Sig Prio'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['ag'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Time'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Risk'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Priority'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Reliability'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Asset Dst'] = '<I> ' . _ANY . ' </I>';
$criteria_arr['meta']['Type'] = '<I> ' . _ANY . ' </I>';

$db = NewBASEDBConnection($DBlib_path, $DBtype);
$db->baseDBConnect($db_connect_method, $alert_dbname, $alert_host, $alert_port, $alert_user, $alert_password);

if (!$cs->criteria['sensor']->isEmpty()) $criteria_arr['meta']['Sensor'] = $cs->criteria['sensor']->Description();
if (!$cs->criteria['plugin']->isEmpty()) $criteria_arr['meta']['Plugin'] = $cs->criteria['plugin']->Description();
if (!$cs->criteria['plugingroup']->isEmpty()) $criteria_arr['meta']['Plugin Group'] = $cs->criteria['plugingroup']->Description();
if (!$cs->criteria['userdata']->isEmpty()) $criteria_arr['meta']['Userdata'] = $cs->criteria['userdata']->Description();
if (!$cs->criteria['sourcetype']->isEmpty()) $criteria_arr['meta']['Source Type'] = $cs->criteria['sourcetype']->Description();
if (!$cs->criteria['category']->isEmpty() && $cs->criteria['category']->Description() != "") $criteria_arr['meta']['Category'] = $cs->criteria['category']->Description();
if (!$cs->criteria['sig']->isEmpty() && $cs->criteria['sig']->Description() != "") $criteria_arr['meta']['Signature'] = $cs->criteria['sig']->Description();
if (!$cs->criteria['sig_class']->isEmpty()) $criteria_arr['meta']['Sig Class'] = $cs->criteria['sig_class']->Description();
if (!$cs->criteria['sig_priority']->isEmpty() && $cs->criteria['sig_priority']->Description() != "") $criteria_arr['meta']['Sig Prio'] = $cs->criteria['sig_priority']->Description();
if (!$cs->criteria['ag']->isEmpty()) $criteria_arr['meta']['ag'] = $cs->criteria['ag']->Description();
if (!$cs->criteria['time']->isEmpty()) $criteria_arr['meta']['Time'] = $cs->criteria['time']->Description();
if (!$cs->criteria['ossim_risk_a']->isEmpty()) $criteria_arr['meta']['Risk'] = $cs->criteria['ossim_risk_a']->Description();
if (!$cs->criteria['ossim_priority']->isEmpty() && $cs->criteria['ossim_priority']->Description() != "") $criteria_arr['meta']['Priority'] = $cs->criteria['ossim_priority']->Description();
if (!$cs->criteria['ossim_reliability']->isEmpty() && $cs->criteria['ossim_reliability']->Description() != "") $criteria_arr['meta']['Reliability'] = $cs->criteria['ossim_reliability']->Description();
if (!$cs->criteria['ossim_asset_dst']->isEmpty() && $cs->criteria['ossim_asset_dst']->Description() != "") $criteria_arr['meta']['Asset Dst'] = $cs->criteria['ossim_asset_dst']->Description();
if (!$cs->criteria['ossim_type']->isEmpty()) $criteria_arr['meta']['Type'] = $cs->criteria['ossim_type']->Description();

// IP
if ((!$cs->criteria['ip_addr']->isEmpty() || !$cs->criteria['ip_field']->isEmpty()) && $cs->criteria['ip_addr']->Description() != "") {
	$criteria_arr['ip']['IP Addr'] = $cs->criteria['ip_addr']->Description_full();
	//$criteria_arr['ip']['IP Field'] = $cs->criteria['ip_field']->Description();
} else {
	$criteria_arr['ip']['IP Addr'] = '<I> ' . _ANY . ' </I>';
	//$criteria_arr['ip']['IP Field'] = '<I> ' . _ANY . ' </I>';
}

// LAYER4
$criteria_arr['layer4']['TCP Port'] = '<I> none </I>';
//$criteria_arr['layer4']['TCP Flags'] = '<I> none </I>';
//$criteria_arr['layer4']['TCP Field'] = '<I> none </I>';
$criteria_arr['layer4']['UPD Port'] = '<I> none </I>';
//$criteria_arr['layer4']['UDP Field'] = '<I> none </I>';
$criteria_arr['layer4']['ICMP Field'] = '<I> none </I>';
$criteria_arr['layer4']['RawIP Field'] = '<I> none </I>';		
if ($cs->criteria['layer4']->Get() == "TCP") {
	if (!$cs->criteria['tcp_port']->isEmpty() || !$cs->criteria['tcp_flags']->isEmpty() || !$cs->criteria['tcp_field']->isEmpty()) {
		$criteria_arr['layer4']['TCP Port'] = $cs->criteria['tcp_port']->Description();
		//$criteria_arr['layer4']['TCP Flags'] = $cs->criteria['tcp_flags']->Description();
		//$criteria_arr['layer4']['TCP Field'] = $cs->criteria['tcp_field']->Description();
	}
} else if ($cs->criteria['layer4']->Get() == "UDP") {
	if (!$cs->criteria['udp_port']->isEmpty() || !$cs->criteria['udp_field']->isEmpty()) {
		$criteria_arr['layer4']['UPD Port'] = $cs->criteria['udp_port']->Description();
		//$criteria_arr['layer4']['UDP Field'] = $cs->criteria['udp_field']->Description();
	}
} else if ($cs->criteria['layer4']->Get() == "ICMP") {
	if (!$cs->criteria['icmp_field']->isEmpty()) {
		$criteria_arr['layer4']['ICMP Field'] = $cs->criteria['icmp_field']->Description();
	}
} else if ($cs->criteria['layer4']->Get() == "RawIP") {
	if (!$cs->criteria['rawip_field']->isEmpty()) {
		$criteria_arr['layer4']['RawIP Field'] = $cs->criteria['rawip_field']->Description();
	}
}

/* Payload ************** */
if (!$cs->criteria['data']->isEmpty()) {
	$criteria_arr['payload']['Data'] = $cs->criteria['data']->Description();
} else {
	$criteria_arr['payload']['Data'] = '<I> ' . _ANY . ' </I>';
}

// Report Data
$report_data = array();
$r_meta = preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;|,\s+$/i","",preg_replace("/\<br\>/i",", ",$criteria_arr['meta']));
$r_payload = preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$criteria_arr['payload']);
$r_ip = preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$criteria_arr['ip']);
$r_l4 = preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$criteria_arr['layer4']);
$report_data[] = array (_("META"),strip_tags($r_meta),"","","","","","","","","",0,0,0);
$report_data[] = array (_("PAYLOAD"),strip_tags($r_payload),"","","","","","","","","",0,0,0);
$report_data[] = array (_("IP"),strip_tags($r_ip),"","","","","","","","","",0,0,0);
$report_data[] = array (_("LAYER 4"),strip_tags($r_l4),"","","","","","","","","",0,0,0);

?>
<html>
<HEAD><meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"><META HTTP-EQUIV="pragma" CONTENT="no-cache"><meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" /><META HTTP-EQUIV="REFRESH" CONTENT="180"><TITLE>Forensics Console : Query Results</TITLE><LINK rel="stylesheet" type="text/css" HREF="/ossim/forensics/styles/ossim_style.css"> 
</HEAD>
<body>
<table width="100%">
	<tr>
		<td>
			<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH="100%">
				<TR>
					<TD height="27" align="center" style="background:url('../pixmaps/fondo_col.gif') repeat-x;border:1px solid #CACACA;color:#333333;font-size:14px;font-weight:bold">META</TD>
				</TR>
				<tr>
					<td>
						<table width="100%">
							<?php foreach ($criteria_arr['meta'] as $meta=>$val) { ?>
							<tr>
								<th width="50%" align="right" style="padding-right:10px"><?php echo $meta ?></th>
								<td width="50%" align="left"><?php echo preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;|,\s+$/i","",$val) ?></td>
							</tr>
							<?php } ?>
						</table>
					</td>
				</tr>
			</TABLE>
		</td>
	</tr>
	<tr>
		<td>
			<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH="100%">
				<TR>
					<TD height="27" align="center" style="background:url('../pixmaps/fondo_col.gif') repeat-x;border:1px solid #CACACA;color:#333333;font-size:14px;font-weight:bold">PAYLOAD</TD>
				</TR>
				<tr>
					<td>
						<table width="100%">
							<?php foreach ($criteria_arr['payload'] as $meta=>$val) { ?>
							<tr>
								<th width="50%" align="right" style="padding-right:10px"><?php echo $meta ?></th>
								<td width="50%" align="left"><?php echo preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$val) ?></td>
							</tr>
							<?php } ?>
						</table>
					</td>
				</tr>
			</TABLE>
		</td>
	</tr>
	<tr>
		<td>
			<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH="100%">
				<TR>
					<TD height="27" align="center" style="background:url('../pixmaps/fondo_col.gif') repeat-x;border:1px solid #CACACA;color:#333333;font-size:14px;font-weight:bold">IP</TD>
				</TR>
				<tr>
					<td>
						<table width="100%">
							<?php foreach ($criteria_arr['ip'] as $meta=>$val) { ?>
							<tr>
								<th width="50%" align="right" style="padding-right:10px"><?php echo $meta ?></th>
								<td width="50%" align="left"><?php echo preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$val) ?></td>
							</tr>
							<?php } ?>
						</table>
					</td>
				</tr>
			</TABLE>
		</td>
	</tr>
	<tr>
		<td>
			<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH="100%">
				<TR>
					<TD height="27" align="center" style="background:url('../pixmaps/fondo_col.gif') repeat-x;border:1px solid #CACACA;color:#333333;font-size:14px;font-weight:bold">LAYER 4</TD>
				</TR>
				<tr>
					<td>
						<table width="100%">
							<?php foreach ($criteria_arr['layer4'] as $meta=>$val) { ?>
							<tr>
								<th width="50%" align="right" style="padding-right:10px"><?php echo $meta ?></th>
								<td width="50%" align="left"><?php echo preg_replace("/\<a (.*?)\<\/a\>|\&nbsp;/i","",$val) ?></td>
							</tr>
							<?php } ?>
						</table>
					</td>
				</tr>
			</TABLE>
		</td>
	</tr>
</table>
</body>
</html>

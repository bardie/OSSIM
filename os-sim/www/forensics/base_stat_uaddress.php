<?php
/**
* Class and Function List:
* Function list:
* Classes list:
*/
/*******************************************************************************
** OSSIM Forensics Console
** Copyright (C) 2009 OSSIM/AlienVault
** Copyright (C) 2004 BASE Project Team
** Copyright (C) 2000 Carnegie Mellon University
**
** (see the file 'base_main.php' for license details)
**
** Built upon work by Roman Danyliw <rdd@cert.org>, <roman@danyliw.com>
** Built upon work by the BASE Project Team <kjohnson@secureideas.net>
*/
include ("base_conf.php");
include ("vars_session.php");
include ("$BASE_path/includes/base_constants.inc.php");
include ("$BASE_path/includes/base_include.inc.php");
include_once ("$BASE_path/base_db_common.php");
include_once ("$BASE_path/base_common.php");
include_once ("$BASE_path/base_qry_common.php");

if (GET('sensor') != "") ossim_valid(GET('sensor'), OSS_DIGIT, 'illegal:' . _("sensor"));;

// Geoip
include ("geoip.inc");
$gi = geoip_open("/usr/share/geoip/GeoIP.dat", GEOIP_STANDARD);
$addr_type = ImportHTTPVar("addr_type", VAR_DIGIT);
$submit = ImportHTTPVar("submit", VAR_ALPHA | VAR_SPACE, array(
    gettext("Delete Selected"),
    gettext("Delete ALL on Screen"),
    _ENTIREQUERY
));
$dst_ip = NULL;
// Check role out and redirect if needed -- Kevin
$roleneeded = 10000;
$BUser = new BaseUser();
#if (($BUser->hasRole($roleneeded) == 0) && ($Use_Auth_System == 1)) base_header("Location: " . $BASE_urlpath . "/index.php");
$et = new EventTiming($debug_time_mode);
// The below three lines were moved from line 87 because of the odd errors some users were having
/* Connect to the Alert database */
$db = NewBASEDBConnection($DBlib_path, $DBtype);
$db->baseDBConnect($db_connect_method, $alert_dbname, $alert_host, $alert_port, $alert_user, $alert_password);

$cs = new CriteriaState("base_stat_uaddress.php", "&amp;addr_type=$addr_type");
$cs->ReadState();
/* Dump some debugging information on the shared state */
if ($debug_mode > 0) {
    PrintCriteriaState();
}

//print_r($_SESSION['ip_addr']);
$page_title = _("Unique Address");
$qs = new QueryState();
$qs->AddCannedQuery("most_frequent", $freq_num_uaddr, gettext("Most Frequent IP addresses"), "occur_d");
$qs->MoveView($submit); /* increment the view if necessary */
if ($qs->isCannedQuery()) PrintBASESubHeader($page_title . ": " . $qs->GetCurrentCannedQueryDesc() , $page_title . ": " . $qs->GetCurrentCannedQueryDesc() , $cs->GetBackLink() , 1);
else PrintBASESubHeader($page_title, $page_title, $cs->GetBackLink() , 1);
if ($event_cache_auto_update == 1) UpdateAlertCache($db);
$criteria_clauses = ProcessCriteria();
if (!$printing_ag) {
    /* ***** Generate and print the criteria in human readable form */
    echo '<TABLE WIDTH="100%">
           <TR>
             <TD WIDTH="60%" VALIGN=TOP>';
    if (!array_key_exists("minimal_view", $_GET)) {
        PrintCriteria($caller);
    }
    echo '</TD></tr><tr>
           <TD VALIGN=TOP>';
    if (!array_key_exists("minimal_view", $_GET)) {
        PrintFramedBoxHeader(gettext("Summary Statistics"), "#669999", "#FFFFFF");
        PrintGeneralStats($db, 1, $show_summary_stats, "$join_sql ", "$where_sql $criteria_sql");
    }
    PrintFramedBoxFooter();
    echo ' </TD>
           </TR>
          </TABLE>
		  <!-- END HEADER TABLE -->
		  
		  </div>  </TD>
           </TR>
          </TABLE>';
}
$criteria = $criteria_clauses[0] . " " . $criteria_clauses[1];
$from = " FROM acid_event " . $criteria_clauses[0];
$where = " WHERE " . $criteria_clauses[1];
// use accumulate tables only with timestamp criteria
$use_ac = (preg_match("/AND/", preg_replace("/AND \( timestamp/", "", $criteria_clauses[1]))) ? false : true;
if (preg_match("/ \d\d:\d\d:\d\d/",$criteria_clauses[1])) $use_ac = false;
//$use_ac = false;
if (preg_match("/^(.*)AND\s+\(\s+timestamp\s+[^']+'([^']+)'\s+\)\s+AND\s+\(\s+timestamp\s+[^']+'([^']+)'\s+\)(.*)$/", $where, $matches)) {
    if ($matches[2] != $matches[3]) {
        $where = $matches[1] . " AND timestamp BETWEEN('" . $matches[2] . "') AND ('" . $matches[3] . "') " . $matches[4];
    } else {
        $where = $matches[1] . " AND timestamp >= '" . $matches[2] . "' " . $matches[4];
    }
}
//$qs->AddValidAction("ag_by_id");
//$qs->AddValidAction("ag_by_name");
//$qs->AddValidAction("add_new_ag");
$qs->AddValidAction("del_alert");
//$qs->AddValidAction("email_alert");
//$qs->AddValidAction("email_alert2");
//$qs->AddValidAction("csv_alert");
//$qs->AddValidAction("archive_alert");
//$qs->AddValidAction("archive_alert2");
$qs->AddValidActionOp(gettext("Delete Selected"));
$qs->AddValidActionOp(gettext("Delete ALL on Screen"));
$qs->SetActionSQL($from . $where);
$et->Mark("Initialization");
$qs->RunAction($submit, PAGE_STAT_UADDR, $db);
$et->Mark("Alert Action");
/* Run the query to determine the number of rows (No LIMIT
$cnt_sql = "SELECT count(DISTINCT $addr_type_name) " . $from . $where;
if (!$use_ac) $qs->GetNumResultRows($cnt_sql, $db);)*/

$et->Mark("Counting Result size");
/* Setup the Query Results Table */
$qro = new QueryResultsOutput("base_stat_uaddress.php?caller=" . $caller . "&amp;addr_type=" . $addr_type);
$qro->AddTitle(" ");
$qro->AddTitle(_("IP address"), "addr_a", " ", " ORDER BY ip ASC", "addr_d", " ", " ORDER BY ip DESC");
$qro->AddTitle(gettext("Sensor") . "&nbsp;#");
$qro->AddTitle(gettext("Total") . "&nbsp;#", "occur_a", " ", " ORDER BY num_events ASC", "occur_d", " ", " ORDER BY num_events DESC");
$qro->AddTitle(_("Unique Events Src"), "sigsrc_a", " ", " ORDER BY num_sig_src ASC", "sigsrc_d", " ", " ORDER BY num_sig_src DESC");
$qro->AddTitle(_("Unique Events Dst"), "sigdst_a", " ", " ORDER BY num_sig_dst ASC", "sigdst_d", " ", " ORDER BY num_sig_dst DESC");
$qro->AddTitle(_("Src. Addr."), "saddr_a", " ", " ORDER BY num_sip ASC", "saddr_d", " ", " ORDER BY num_sip DESC");
$qro->AddTitle(_("Dest. Addr."), "daddr_a", "  ", " ORDER BY num_dip ASC", "daddr_d", " ", " ORDER BY num_dip DESC");
$sort_sql = $qro->GetSortSQL($qs->GetCurrentSort() , $qs->GetCurrentCannedQuerySort());

$src_sql = "SELECT DISTINCT ip_src as ip, COUNT(acid_event.cid) as num_events, COUNT( DISTINCT acid_event.sid) as num_sensors, COUNT( DISTINCT acid_event.plugin_id, acid_event.plugin_sid ) as num_sig_src, 0 as num_sig_dst, 0 as num_sip, COUNT( DISTINCT ip_dst ) as num_dip " . $sort_sql[0] . $from . $where . " GROUP BY ip_src HAVING num_events>0 " . $sort_sql[1];

$dst_sql = "SELECT DISTINCT ip_dst as ip, COUNT(acid_event.cid) as num_events, COUNT( DISTINCT acid_event.sid) as num_sensors, 0 as num_sig_src, COUNT( DISTINCT acid_event.plugin_id, acid_event.plugin_sid ) as num_sig_dst, COUNT( DISTINCT ip_src ) as num_sip, 0 as num_dip " . $sort_sql[0] . $from . $where . " GROUP BY ip_dst HAVING num_events>0 " . $sort_sql[1];

$sql = "SELECT SQL_CALC_FOUND_ROWS ip,sum(num_events) as num_events,sum(num_sensors) as num_sensors,sum(num_sig_src) as num_sig_src, sum(num_sig_dst) as num_sig_dst, sum(num_sip) as num_sip,sum(num_dip) as num_dip
    	FROM (($src_sql) UNION ($dst_sql)) as u WHERE ip>0 GROUP BY ip " . $sort_sql[1];
    	
// use accumulate tables only with timestamp criteria
if ($use_ac) {
	// SRC
    $where = $more = $sqla = $sqlb = $sqlc = "";
    if (preg_match("/timestamp/", $criteria_clauses[1])) {
        $where = "WHERE " . str_replace("timestamp", "day", $criteria_clauses[1]);
        $sqla = " and ac_srcaddr_ipsrc.day=ac_srcaddr_sid.day";
        $sqlb = " and ac_srcaddr_ipsrc.day=ac_alerts_ipsrc.day";
        $sqlc = " and ac_srcaddr_ipsrc.day=ac_srcaddr_ipdst.day";
    }
    $orderby = str_replace("acid_event.", "", $sort_sql[1]);
    $src_sql = "SELECT DISTINCT ip_src as ip,
        sum(cid) as num_events,
        (select count(distinct(sid)) from ac_srcaddr_sid where ac_srcaddr_ipsrc.ip_src=ac_srcaddr_sid.ip_src $sqla) as num_sensors,
        (select count(distinct ac_alerts_ipsrc.plugin_id, ac_alerts_ipsrc.plugin_sid) from ac_alerts_ipsrc where ac_srcaddr_ipsrc.ip_src=ac_alerts_ipsrc.ip_src $sqlb) as num_sig_src, 0 as num_sig_dst, 0 as num_sip, 
        (select count(distinct(ip_dst)) from ac_srcaddr_ipdst where ac_srcaddr_ipsrc.ip_src=ac_srcaddr_ipdst.ip_src $sqlc) as num_dip
        FROM ac_srcaddr_ipsrc $where GROUP BY ip_src HAVING num_events>0";
    // DST
    $where = $more = $sqla = $sqlb = $sqlc = "";
    if (preg_match("/timestamp/", $criteria_clauses[1])) {
        $where = "WHERE " . str_replace("timestamp", "day", $criteria_clauses[1]);
        $sqla = " and ac_dstaddr_ipdst.day=ac_dstaddr_sid.day";
        $sqlb = " and ac_dstaddr_ipdst.day=ac_alerts_ipdst.day";
        $sqlc = " and ac_dstaddr_ipdst.day=ac_dstaddr_ipsrc.day";
    }
    $orderby = str_replace("acid_event.", "", $sort_sql[1]);
    $dst_sql = "SELECT DISTINCT ip_dst as ip,
        sum(cid) as num_events,
        (select count(distinct(sid)) from ac_dstaddr_sid where ac_dstaddr_ipdst.ip_dst=ac_dstaddr_sid.ip_dst $sqla) as num_sensors, 0 as num_sig_src,
        (select count(distinct ac_alerts_ipdst.plugin_id, ac_alerts_ipdst.plugin_sid) from ac_alerts_ipdst where ac_dstaddr_ipdst.ip_dst=ac_alerts_ipdst.ip_dst $sqlb) as num_sig_dst,
        (select count(distinct(ip_src)) from ac_dstaddr_ipsrc where ac_dstaddr_ipdst.ip_dst=ac_dstaddr_ipsrc.ip_dst $sqlc) as num_sip, 0 as num_dip
        FROM ac_dstaddr_ipdst $where GROUP BY ip_dst HAVING num_events>0";
    
    $sql = "SELECT SQL_CALC_FOUND_ROWS ip,sum(num_events) as num_events,sum(num_sensors) as num_sensors,sum(num_sig_src) as num_sig_src, sum(num_sig_dst) as num_sig_dst, sum(num_sip) as num_sip,sum(num_dip) as num_dip
    	FROM (($src_sql) UNION ($dst_sql)) as u WHERE ip>0 GROUP BY ip $orderby";

}
//echo "<br>"; print_r ($sql);
//print_r($_SESSION);
/* Run the Query again for the actual data (with the LIMIT) */
$result = $qs->ExecuteOutputQuery($sql, $db);
$qs->GetCalcFoundRows($cnt_sql, $db); // if ($use_ac) 
$et->Mark("Retrieve Query Data");
if ($debug_mode == 1) {
    $qs->PrintCannedQueryList();
    $qs->DumpState();
    echo "$sql<BR>";
}
/* Print the current view number and # of rows */
$qs->PrintResultCnt("",array(),gettext("Displaying unique addresses %d-%d of <b>%s</b> matching your selection. <b>%s</b> total events in database."));
echo '<FORM METHOD="post" name="PacketForm" id="PacketForm" ACTION="base_stat_uaddress.php">';
$qro->PrintHeader();
$i = 0;
$hosts_ips = array_keys($hosts);
$report_data = array(); // data to fill report_data 

if (is_array($_SESSION["server"]) && $_SESSION["server"][0]!="")
	$_conn = $dbo->custom_connect($_SESSION["server"][0],$_SESSION["server"][2],$_SESSION["server"][3]);
else
	$_conn = $dbo->connect();
	
while (($myrow = $result->baseFetchRow()) && ($i < $qs->GetDisplayRowCnt())) {
    $currentIP = baseLong2IP($myrow[0]);
    $num_events = $myrow[1];
    $num_sensors = $myrow[2];
    $num_sig_src = $myrow[3];
    $num_sig_dst = $myrow[4];
    $num_sip = $myrow[5];
    $num_dip = $myrow[6];
    if ($myrow[0] == NULL) $no_ip = true;
    else $no_ip = false;
    qroPrintEntryHeader($i);
    /* Generating checkbox value -- nikns */
    ($addr_type == SOURCE_IP) ? ($src_ip = $myrow[0]) : ($dst_ip = $myrow[0]);
    $tmp_rowid = $src_ip . "_" . $dst_ip;
    echo '    <TD><INPUT TYPE="checkbox" NAME="action_chk_lst[' . $i . ']" VALUE="' . $tmp_rowid . '">';
    echo '    <INPUT TYPE="hidden" NAME="action_lst[' . $i . ']" VALUE="' . $tmp_rowid . '"></TD>';
    /* Check for a NULL IP which indicates an event (e.g. portscan)
    * which has no IP
    */
    if ($no_ip) qroPrintEntry('<A HREF="' . $BASE_urlpath . '/help/base_app_faq.php#1">' . gettext("unknown") . '</A>');
    else {
        $country = strtolower(geoip_country_code_by_addr($gi, $currentIP));
        $country_name = geoip_country_name_by_addr($gi, $currentIP);
        $homelan = (Net::is_ip_in_cache_cidr($_conn, $currentIP) || in_array($currentIP, $hosts_ips)) ? " <a href='javascript:;' class='scriptinfo' style='text-decoration:none' ip='$currentIP'><img src=\"images/homelan.png\" border=0></a>" : "";
        if ($country) {
            $country_img = " <img src=\"/ossim/pixmaps/flags/" . $country . ".png\" title=\"" . $country_name . "\">";
            $slnk = $current_url."/pixmaps/flags/".$country.".png";
        } else {
            $country_img = "";
            $slnk = ($homelan!="") ? $current_url."/forensics/images/homelan.png" : "";
        }
        $sip_aux = ($sensors[$currentIP] != "") ? $sensors[$currentIP] : (($hosts[$currentIP] != "") ? $hosts[$currentIP] : $currentIP);
        $div = '<div id="'.$currentIP.';'.$ip_aux.'" class="HostReportMenu">';
		$bdiv = '</div>';
        qroPrintEntry( $div. BuildAddressLink($currentIP, 32) . $currentIP . '</A>&nbsp;' . $country_img . $homelan . $bdiv,'center','','nowrap');
    }
    if ($resolve_IP == 1) qroPrintEntry('&nbsp;&nbsp;' . baseGetHostByAddr($currentIP, $db, $dns_cache_lifetime) . '&nbsp;&nbsp;');
    /* Print # of Occurances */
    $tmp_iplookup = 'base_qry_main.php?num_result_rows=-1' . '&amp;submit=' . gettext("Query+DB") . '&amp;current_view=-1';
    $tmp_iplookup2 = 'base_stat_alerts.php?num_result_rows=-1' . '&amp;submit=' . gettext("Query+DB") . '&amp;current_view=-1&sort_order=occur_d';

    if ($no_ip) $url_criteria = BuildIPFormVars(NULL_IP);
    else $url_criteria = BuildIPFormVars($currentIP);
        
    if ($no_ip) $url_criteria_src = BuildSrcIPFormVars(NULL_IP);
    else $url_criteria_src = BuildSrcIPFormVars($currentIP);
     
    if ($no_ip) $url_criteria_dst = BuildDstIpFormVars(NULL_IP);
    else $url_criteria_dst = BuildDstIPFormVars($currentIP);
    
    qroPrintEntry($num_sensors);
    qroPrintEntry('<A HREF="' . $tmp_iplookup . $url_criteria . '">' . $num_events . '</A>');
    qroPrintEntry('<A HREF="' . $tmp_iplookup2 . $url_criteria_src . '">' . $num_sig_src . '</A>');
    qroPrintEntry('<A HREF="' . $tmp_iplookup2 . $url_criteria_dst . '">' . $num_sig_dst . '</A>');
    qroPrintEntry($num_sip);
    qroPrintEntry($num_dip);
    qroPrintEntryFooter();
    ++$i;
    
    /* report_data
    $report_data[] = array (
        $currentIP, $slnk,
        $num_sig, $num_ip,
        "", "", "", "", "", "", "",
        intval($_GET['addr_type']), $num_sensors , $num_events
    );*/
}
$result->baseFreeRows();
$dbo->close($_conn);
$qro->PrintFooter();
$qs->PrintBrowseButtons();
$qs->PrintAlertActionButtons();
//$qs->SaveReportData($report_data,$unique_addr_report_type);
$qs->SaveState();
ExportHTTPVar("addr_type", $addr_type);
echo "\n</FORM>\n";
$et->Mark("Get Query Elements");
$et->PrintTiming();
PrintBASESubFooter();
echo "</body>\r\n</html>";
geoip_close($gi);
?>

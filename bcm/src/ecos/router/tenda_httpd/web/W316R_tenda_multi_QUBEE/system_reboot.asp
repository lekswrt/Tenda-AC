<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>System | Reboot</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript">
var lanip = "<%aspTendaGetStatus("lan","lanip");%>";
var time = 0;
var pc = 0;
Butterlate.setTextDomain("system_tool");
function initTranslate(){
	var e=document.getElementById("rebootBtn");
	e.value=_("Reboot the router");
}
function init(f)
{
	initTranslate();
}
function preSubmit() {
	if(window.confirm(_("The system will re-connect to the initial page in about 50 seconds")))
	{	
		document.frmSetup.style.cursor = "wait";
		document.getElementById("rebootBtn").disabled = true;
	    var code = "/goform/SysToolReboot";
		var request = GetReqObj();
		request.open("GET", code, true);
		request.onreadystatechange = RequestRes;
		request.setRequestHeader("If-Modified-Since","0");
		request.send(null);	
		reboot();
	}
	else
		return;
}
function RequestRes()
{
}
function reboot()
{
	pc+=1; 
	if (pc > 100) 
	{ 
		window.top.location = "http://" + lanip;
		document.frmSetup.style.cursor = "auto";
		clearTimeout(time); 
		return;
	} 
	//setWidth(self, "lpc", pc + "%");
	document.getElementById("lpc").style.width= pc+"%";
	document.getElementById("percent").innerHTML = pc + "%";
	time = setTimeout("reboot()",250);
}
function setWidth(windowObj, el, newwidth)
{
    if (document.all)
	{
	  if (windowObj.document.all(el) )
        windowObj.document.all(el).style.width = newwidth ;
	}
	else if (document.getElementById)
	{
	  if (windowObj.document.getElementById(el) )
	    windowObj.document.getElementById(el).style.width = newwidth;
	}
}
</SCRIPT>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init(document.frmSetup);" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
              <tr>
                <td align="center" valign="top">
				<form name=frmSetup method="POST" action=/goform/SysToolReboot>
				<INPUT type=hidden name=CMD value=SYS_CONF>
				<INPUT type=hidden name=GO value=system_reboot.asp>
				<INPUT type=hidden name=CCMD value=0>
				<table cellpadding="0" cellspacing="0" class="content2">
			    <tr><td colspan=2 valign="top">&nbsp;&nbsp;<script>document.write(_("Click this button and the router will reboot"));</script></td>
			    </tr>
				</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table1">
					<tr><td>
					&nbsp;&nbsp;
					<input class="button2" onClick="preSubmit()" value="" type="button" onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'"  id="rebootBtn">
					</td>
					<td height="30" id="percent" style="font-size:24px"></td>
					</tr>		
				<tr> <TD align="left" width="400" colspan="2">
				<table width="100%" align="left" bgcolor="#ffffff" cellpadding="0" cellspacing="0" bordercolor="#999999" id="AutoNumber19" style="border-style: solid; border-width: 1px; margin-left:5px; margin-top:5px;">
			   <tr>
				<td align=left> 
				<table id=lpc bgcolor="#3366CC" height=20>
				<tr>
				  <td></TD>
				</TR>
				</table> 
			   </TD>				
			   </TR>
			   </table> 
				</TD>
				</tr>
				</table>
			</FORM>
				</td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo(_("system_reboot_Help_Inf"));</script>
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload('table1');
    </script>
</BODY>
</HTML>



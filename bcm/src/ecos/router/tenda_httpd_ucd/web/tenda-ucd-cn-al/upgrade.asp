<!DOCTYPE html><html lang="en" class="index-html">	<head>	    <meta charset="utf-8">	    <link href="css/reasy-1.0.0.css" rel="stylesheet">      <title>腾达无线路由器</title>            <script>		function isFileType(name, types) {			var fileTypeArr = name.split('.'),				fileType = fileTypeArr[fileTypeArr.length - 1],				len = types.length,				i;							for (i = 0; i < len; i++) {				if (fileType === types[i]) {					return true;				}			}						return false;		}		        function goUpgrade() {	           var upgradefile = document.getElementById('upgradeFile').value,              upform = document.upgradefrm;                    if (upgradefile == null || upgradefile == "") {            return;                    // 判断文件类型          } //else {//if (!isFileType(upgradefile, ['bin', 'trx'])) {           // alert('请选择文件名以 “trx” 或 “bin”结尾的文件');          //  upform.reset();         //   return;        //  }                    if(confirm('您确信要升级吗?')){            upform.submit();            parent.showUpgrading();          } else {            upform.reset();          }        }		</script>	</head>	<body>    <form name="upgradefrm" method="POST" action="/cgi-bin/upgrade" enctype="multipart/form-data">      <div class="control-group">        <label class="control-label">软件升级</label>        <div class="controls" style="position: relative">          <input type="file" class="fileUpLoadBtn" name="upgradeFile" id="upgradeFile"  onchange="goUpgrade()"/>          <input type="button" name="upgrade" id="upgrade"  class="btn" value="选择文件" >          <span class="help-inline softVer" id="softVer">当前软件版本：<%aspTendaGetStatus("sys","sysver");%></span>        </div>      </div>    </form>			</body></html>
<?php
$host     = $_POST["host"];     // CMS ip
$port     = $_POST["port"];     // CMS port
$id       = $_POST["id"];       // IDS id, used to manage single IDS
$content  = $_POST["content"];  // content of 'Send' or 'File' command
$params   = $_POST["params"];   // parameters of 'List' command
$filename = $_POST["filename"]; // file to sent or fetch

if ($host == "") $host = "127.0.0.1";
if ($port == "") $port = "10101";

function send_package($socket, $data, $size)
{
	$r = @socket_write($socket, pack("L", $size));
	if ( ! $r) return false;
	if ($size > 0)
	{
		$r = @socket_write($socket, $data);
		if ( ! $r) return false;
	}
	return true;
}

function recv_package($socket)
{
	$data = @socket_read($socket, 4);
	if ( ! $data) return false;

	$p = @unpack("L", $data);
	if ( ! $p) return false;
	$size = $p[1];

	$result = "";
	if ($size > 0)
	{
		while ($buffer = @socket_read($socket, $size))
		{
			$result .= "$buffer";
			$size -= strlen($buffer);
			if ($size <= 0) break;
		}
	}
	return $result;
}

function create_json($error, $message, $result = "")
{
	return '{'
		. '"error":' . $error . ','
		. '"message":"' . $message . '",'
		. '"result":{' . $result . '}'
		. '}';
}

function list_ids($host, $port, $params)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ( ! $socket) return create_json(1, '无法创建socket！');

	$connection = @socket_connect($socket, $host, $port);
	if ( ! $connection) return create_json(2, '无法连上CMS服务器（' . $host . ' : ' . $port . '）！');

	$r = @socket_write($socket, "NetPower");
	if ( ! $r) return create_json(3, '发送标志失败！');

	$r = @socket_write($socket, "List");
	if ( ! $r) return create_json(3, '发送命令失败！');

	$r = send_package($socket, $params, strlen($params));
	if ( ! $r) return create_json(3, '发送命令参数失败！');

	$result = recv_package($socket);
	if ( ! $result) return create_json(4, '接收结果失败！');

	socket_close($socket);
	return $result;
}

function send_ids($host, $port, $id, $content)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ( ! $socket) return create_json(1, '无法创建socket！');

	$connection = @socket_connect($socket, $host, $port);
	if ( ! $connection) return create_json(2, '无法连上CMS服务器（' . $host . ' : ' . $port . '）！');

	$r = @socket_write($socket, "NetPower");
	if ( ! $r) return create_json(3, '发送标志失败！');

	$r = @socket_write($socket, "Send");
	if ( ! $r) return create_json(3, '发送命令失败！');

	$r = send_package($socket, $id, strlen($id));
	if ( ! $r) return create_json(3, '发送IDS标识失败！');

	$r = send_package($socket, $content, strlen($content));
	if ( ! $r) return create_json(3, '发送命令参数失败！');

	$result = recv_package($socket);
	if ( ! $result) return create_json(4, '接收结果失败！');

	socket_close($socket);
	return $result;
}

function send_file($host, $port, $id, $content, $filename)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ( ! $socket) return create_json(1, '无法创建socket！');

	$connection = @socket_connect($socket, $host, $port);
	if ( ! $connection) return create_json(2, '无法连上CMS服务器（' . $host . ' : ' . $port . '）！');

	$r = @socket_write($socket, "NetPower");
	if ( ! $r) return create_json(3, '发送标志失败！');

	$r = @socket_write($socket, "File");
	if ( ! $r) return create_json(3, '发送命令失败！');

	$r = send_package($socket, $id, strlen($id));
	if ( ! $r) return create_json(3, '发送IDS标识失败！');

	$r = send_package($socket, $content, strlen($content));
	if ( ! $r) return create_json(3, '发送命令参数失败！');

	$r = send_package($socket, $filename, strlen($filename));
	if ( ! $r) return create_json(3, '发送文件名失败！');

	$result = recv_package($socket);
	if ( ! $result) return create_json(4, '接收结果失败！');

	socket_close($socket);
	return $result;
}

function down_file($host, $port, $id, $filename)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ( ! $socket) return create_json(1, '无法创建socket！');

	$connection = @socket_connect($socket, $host, $port);
	if ( ! $connection) return create_json(2, '无法连上CMS服务器（' . $host . ' : ' . $port . '）！');

	$r = @socket_write($socket, "NetPower");
	if ( ! $r) return create_json(3, '发送标志失败！');

	$r = @socket_write($socket, "Down");
	if ( ! $r) return create_json(3, '发送命令失败！');

	$r = send_package($socket, $id, strlen($id));
	if ( ! $r) return create_json(3, '发送IDS标识失败！');

	$r = send_package($socket, $filename, strlen($filename));
	if ( ! $r) return create_json(3, '发送文件名失败！');

	$result = recv_package($socket);
	if ( ! $result) return create_json(4, '接收结果失败！');

	socket_close($socket);
	return $result;
}

if ($_POST["command"] == "List")
{
	$result = list_ids($host, $port, $params);
	echo $result;
	exit;
}
else if ($_POST["command"] == "Send")
{
	$result = send_ids($host, $port, $id, $content);
	echo $result;
	exit;
}
else if ($_POST["command"] == "File")
{
	$result = send_file($host, $port, $id, $content, $filename);
	echo $result;
	exit;
}
else if ($_POST["command"] == "Down")
{
	$result = down_file($host, $port, $id, $filename);
	echo $result;
	exit;
}
?>
<html>
<head>
<title>Demo for NetPower Messenger</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<script type="text/javascript">
function Clock(divName) {
	updateClock = function() {
		var now = new Date();
		document.getElementById(divName).innerHTML
			= (now.getYear() + 1900) + "年" + (now.getMonth() + 1) + "月"
			+ now.getDate() + "日" + now.getHours() + "时"
			+ now.getMinutes() + "分" + now.getSeconds() + "秒";
	}
	updateClock();
	setInterval(updateClock, 1000);
}

if (typeof(XMLHttpRequest) == "undefined") {
	XMLHttpRequest = function() {
		try { return new ActiveXObject("Msxml2.XMLHTTP.6.0"); } catch(e) {}
		try { return new ActiveXObject("Msxml2.XMLHTTP.3.0"); } catch(e) {}
		try { return new ActiveXObject("Msxml2.XMLHTTP"); } catch(e) {}
		try { return new ActiveXObject("Microsoft.XMLHTTP"); } catch(e) {}
		throw new Error("This browser does not support XMLHttpRequest.");
	};
}
var xmlhttp = new XMLHttpRequest();

function request(params, object, callback)
{
	var url = "<?php echo $_SERVER['PHP_SELF'];?>";

	xmlhttp.open("POST", url, true);
	xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	xmlhttp.setRequestHeader("Content-length", params.length);
	xmlhttp.setRequestHeader("Connection", "close");
	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
		{
			callback.call(object, xmlhttp.responseText);
		}
	}
	xmlhttp.send(params);
}

function createPostParams(command)
{
	var params = 'command=' + command;
	params += '&host=' + encodeURIComponent(document.getElementById('host').value);
	params += '&port=' + encodeURIComponent(document.getElementById('port').value);
	return params;
}

function DeviceList() {
	this.deviceCount = 0;
	this.pageIndex = 0;

	this.listDevices = function() {
		request(createPostParams('List'), this, this.gotDeviceCount);
	}
	this.pageUp = function() { --this.pageIndex; this.listDevices(); };
	this.pageDown = function() { ++this.pageIndex; this.listDevices(); };
}

DeviceList.prototype.gotDeviceCount = function(result) {
	var r = eval('(' + result + ')');
	if (r.error) {
		document.getElementById('content').innerHTML =
			'<font color="red">' + r.message  + '</font>';
	} else {
		this.deviceCount = r.device_count;
		this.refreshDeviceList();
	}
};

DeviceList.prototype.refreshDeviceList = function() {
	var params = createPostParams('List') + "&params=";
	if (this.deviceCount <= 10)
	{
		params += "*";
	}
	else
	{
		var page = Math.ceil(this.deviceCount / 10);
		if (this.pageIndex < 0) this.pageIndex = 0;
		if (this.pageIndex >= page) this.pageIndex = page - 1;
		params += (10 * this.pageIndex + 1).toString() + "-" + (10 * (this.pageIndex + 1)).toString();
	}
	request(params, this, function(result) {
		r = eval('(' + result + ')');
		
		var text = '';
		if (r.error)
		{
			text = '<font color="red">' + r.message + '</font>';
		}
		else
		{
			text += '目前共有 ' + this.deviceCount + ' 台设备在线。<br/>';
			
			var page = Math.ceil(this.deviceCount / 10);
			if (page == 0) page = 1;
			text += '共 ' + page + ' 页，第 ' + (this.pageIndex + 1).toString() + ' 页&nbsp;|&nbsp;';
			if (this.pageIndex == 0)
			{
				text += '上页';
			}
			else
			{
				text += '<a href="javascript:;" onclick="pageUp()">上页</a>';
			}
			text += '&nbsp;';
			if (this.pageIndex == page - 1)
			{
				text += '下页';
			}
			else
			{
				text += '<a href="javascript:;" onclick="pageDown()">下页</a>';
			}
			
			text += '<table id="report" border="0" cellspacing="1" cellpadding="5">';
			text += '<tr><th width="30">ID</th><th width="80">IP</th><th width="100">其他</th></tr>';
			for (var i = 0; i < r.result.list.length; ++i)
			{
				text += '<tr>';
				text += '<td>' + r.result.list[i].id + '</td>';
				text += '<td>' + r.result.list[i].ip + '</td>';
				text += '<td>';
				text += '<a href="javascript:;" onclick="deviceList.testDevice(\'' + r.result.list[i].id + '\', \'info\')">info</a>';
				text += '&nbsp;';
				text += '<a href="javascript:;" onclick="deviceList.testDevice(\'' + r.result.list[i].id + '\', \'time\')">time</a>';
				text += '&nbsp;';
				text += '<a href="javascript:;" onclick="deviceList.testDevice(\'' + r.result.list[i].id + '\', \'file\')">upload</a>';
				text += '&nbsp;';
				text += '<a href="javascript:;" onclick="deviceList.testDevice(\'' + r.result.list[i].id + '\', \'down\')">download</a>';
				text += '</td>';
				text += '</tr>';
			}
			text += '</table><div id="result"></div>';
		}
		document.getElementById('content').innerHTML = text;
	});
};

DeviceList.prototype.sendCommandReturn = function(result) {
	var r = eval('(' + result + ')');
	if (r.error)
	{
		document.getElementById('result').innerHTML = '<font color="red">' + r.message + '</font>';
	}
	else
	{
		var text = '';
		for (var key in r.result)
		{
			text += key + " => " + r.result[key] + "\n";
		}
		document.getElementById('result').innerHTML = text.replace(/\n/g, '<br/>');
	}
};

DeviceList.prototype.sendCommandToIDS = function(command, id, content, filename) {
	var params = createPostParams(command);
	params += "&id=" + encodeURIComponent(id);
	params += "&content=" + encodeURIComponent(content);
	if (filename != '')
	{
		params += "&filename=" + encodeURIComponent(filename);
	}
	request(params, this, this.sendCommandReturn)
};

DeviceList.prototype.testDevice = function(id, type) {
	if (type == 'info')
	{
		this.sendCommandToIDS('Send', id, 'GetHostInfo', '');
	}
	else if (type == 'time')
	{
		this.sendCommandToIDS('Send', id, 'GetSystemTime', '');
	}
	else if (type == 'file')
	{
		this.sendCommandToIDS('File', id, 'UploadFile\nfile:test.txt', 'test.txt');
	}
	else if (type == 'down')
	{
		this.sendCommandToIDS('Down', id, '', 'test2.txt');
	}
}

function Menu() {
	this.group = [];
	this.items = [[]];
	this.procs = [[]];

	this.addGroup = function(name) {
		this.group.push(name);
		this.items.push([]);
		this.procs.push([]);
	};
	this.addMenuItem = function(item, proc) {
		this.items[this.group.length - 1].push(item);
		this.procs[this.group.length - 1].push(proc);
	};
	this.toHTML = function() {
		var s = '';
		for (var i in this.group)
		{
			s += '<div class="menu">';
			s += '<div class="x1"></div>';
			s += '<div class="x2"></div>';
			s += '<div class="x3"></div>';
			s += '<div class="group">' + this.group[i] + '</div>';
			for (var j in this.items[i])
			{
				var s1 = '';
				var s2 = '';
				if (this.procs[i][j] != '')
				{
					s1 = '<a href="javascript:;" onclick="' + this.procs[i][j] + '">';
					s2 = '</a>';
				}
				s += '<div class="item">';
				s += s1;
				s += this.items[i][j];
				s += s2;
				s += '</div>';
			}
			s += '<div class="x4"></div>';
		}
		return s;
	}
}

function initMenu()
{
	menu = new Menu();
	menu.addGroup('设备管理');
	menu.addMenuItem('设备列表', 'deviceList.listDevices()');
	menu.addMenuItem('设备维护', '');
	menu.addGroup('规则管理');
	menu.addMenuItem('规则列表', '');
	menu.addGroup('策略管理');
	menu.addMenuItem('策略列表', '');
	menu.addGroup('其他配置');
	menu.addMenuItem('暂无', '');
	document.getElementById('panel').innerHTML = menu.toHTML();
}

deviceList = new DeviceList();
function init()
{
	new Clock("clock");

	deviceList.listDevices();

	initMenu();

	document.getElementById("host").value = "127.0.0.1";
	document.getElementById("port").value = "10101";
}
</script>
<style type="text/css">
body { margin:0; background-color:#CCCCCC; }
div, td { font-size:12px; }
div { margin:0; padding:0; border:0px solid red; }
#wrapper { width:980px; height:100%; margin:auto; padding:0; background-color:#A0C6D0; }
#header { width:960px; padding:10px; background-color:#336699; color:#DDDDDD; }
#title { margin:0; color:#DDDDDD; }
#cms { float:right; }
#clock { margin:5px 0; color:#CCCCCC; }
#panel { width:130px; padding:0 10px; background-color:#99AAFF; }
#content { padding:5px; background-color:#CCDDFF; }
#footer { padding:10px; text-align:center; font-size:10.5px; border-top:1px solid #336699; }
.menu { display:block; background:transparent; width:130px; margin:10px 0; text-align:center; }
.menu div { display:block; overflow:hidden; }
.x1, .x2, .x3, .x4 { height:1px; }
.x1, .x4 { background-color:#333333; }
.x2, .x3, .menu .group, .item { border-width:0 1px; border-color:#333333; background-color:#6699CC; }
.x1 { margin:0 3px; border-width:0 1px; border-color:#666666; }
.x2 { margin:0 2px; border-width:0 1px; border-color:#333333; }
.x3 { margin:0 2px; }
.menu .group, .item, .x4 { margin:0 1px; }
.menu .group { height:25px; background-color:#6699CC; line-height:20px; font-weight:bold; color:#222222; }
.item { height:25px; line-height:25px; border-top:1px solid #333333; background-color:#99CCFF; }
#report th { background:#328aa4; color:#FFFFFF; font-size:12px; font-weight:bold; }
#report td { background:#e5f1f4; color:#333333; font-size:12px; }
</style>
</head>
<body onLoad="init()">
<div id="wrapper">
<div id="header">
 <h1 id="title">IDS集中管理平台</h1>
 <div id="cms">
  连接到CMS：
  <input id="host" type="text" style="width:150px;" value="" />
  <input id="port" type="text" style="width:50px;" value="" />
 </div>
 <div id="clock">&nbsp;</div>
</div>
<table border="0" cellpadding="0" cellspacing="0" width="100%">
 <tr>
  <td width="150" align="center" valign="top" bgcolor="#99AAFF" id="panel"></td>
  <td id="content" align="left" valign="top"></td>
  <td width="1" bgcolor="#9999CC"></td>
 </tr>
</table>
<div id="footer">&copy;2010，中科网威</div>
</div>
</body>
</html>

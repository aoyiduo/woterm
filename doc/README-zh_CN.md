*本仓库没有包含任何第三方库和kxver模块的源码或库文件，kxver模块主要是用于版本控制，特别是与旗舰版的相关的功能如数据加密，它是团队的唯一资金来源，也是产品健康维护和升级的重要保证，在没有解决资金来源之前，并不计划开放该模块。*
***
<p align="center">
  <img src="doc/woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">跨平台/小内存/多协议/多功能/云同步</h2>
 <h3 style="text-align: center;">
  [<a href="../README.md">English</a>] | 
  [<a href="README-zh_CN.md">简体中文</a>]
  [<a href="README-zh_TW.md">繁體中文</a>]
  [<a href="README-de.md">Deutsch</a>]
  [<a href="README-es.md">Español</a>]
  [<a href="README-fr.md">Français</a>]
  [<a href="README-hi.md">हिंदी</a>]
  [<a href="README-id.md">Bahasa Indonesia</a>]
  [<a href="README-ja.md">日本語</a>]
  [<a href="README-ko.md">한국어</a>]
  [<a href="README-ru.md">Русский</a>]
</h3>
  <h3 style="text-align: center;">[<a href="https://woterm.com">WoTerm官网</a>]</a></h3>
</p>

# 概要
集成主流远程通讯协议，全面满足您的需求：支持 SSH1/SSH2、FTP/FTPS、SFTP、TELNET、RLOGIN、RDP、VNC、SHELL、串口、TCP、UDP 等多种协议，让您无需切换工具，轻松应对各种远程操作和通讯场景。更多的信息请访问官网

## 功能描述
- 支持Windows / Linux / MacOSX / Android。
- 支持主流的通迅协议，包括SSH1/SSH2/SFTP/RLOGIN/TELNET/SERIALlPORT/VNC/RDP等。
- 内置多套皮肤，任君选择，并保留扩展性，让动手能力强的朋友随心所欲。
- 支持管理员模式，可查看会话密码及防止他人盗用等。
- 基于标准SFTP构建的云同步协议，让你自由创建云仓库，并提供8种高强度加密算法，数据安全尽在掌握中。
无限制的备份历史及记录的差异化合并、增、删、替换，可满足不同办公环境。
- SSH终端内嵌SFTP助手，方便协同操作，也支持SFTP独立选项卡，功能更多操作更方便。
- 浮动选项卡设计，更方便多个窗口间的协同工作。
- 无限的窗口分割，只要屏幕够大，想怎样分割就怎样分割。
- 提供会话分组和列表管理，多种关健字搜索过滤，再多的记录，也能极速定位。
- 数十种终端配色，总有心仪的一款。  
- 支持VNC远程桌面访问功能，高级功能需要与[WoVNC](http://wovnc.com)服务端才支持。
  - 支持标准的RFB 3.3/3.7/3.8协议。
  - 支持的标准编码有ZRLE/TRLE/Hextile/CopyRect/RRE/Raw。
  - 支持自扩展编码有H264/JPEG/ZRLE3/TRLE3/ZRLE2/TRLE2。
  - 支持自动琐屏。
  - 支持隐私屏。
  - 支持桌面声音回放。
  - 支持无损画质、高清画质、普通画质、经典16位、15位、8位等图像格式的自由切换。
  - 支持多屏模式。
- 支持远程终端访问功能。
  - 支持远程Windown / Linux / MacOSX的系统终端访问。
  - 支持跳板机配置
  - 支持证书管理，包括创建、导入、导出、删除等。
  - 支持快捷键配置。
  - 支持ZModem文件上传下载。
  - 支持嵌入式SFTP协作交互。
  - 支持多种终端配色，任君选择。


## 仓库模块说明
客户端代码已经全部开源(除kxver版本控制模块外)，其所依赖的第三方代码均来自GitHub或CodeProject或其它开放社区，部份模块来自[WoVNC](http://wovnc.com)。

## WoVNCServer
推荐使用[WoVNCServer](http://www.wovnc.com)服务端，开启更多高级特性。

## 更多信息: 
<a href="http://www.woterm.com">http://www.woterm.com</a>

## 程序下载:
<a href="http://woterm.com">http://woterm.com</a>

## 界面预览
<div>主界面<br><img src="doc/main.gif"/></div>
<div>SSH密钥管理<br><img src="doc/keymgr.gif"></div>
<div>串口服务<br><img src="doc/serialport.gif"></div>
<div>隧道<br><img src="doc/tunnel.png"></div>
<div>多皮肤<br><img src="doc/skins.png"></div>
<div>剧本<br><img src="doc/playbook.gif"></div>
<div>选项卡合并及分离<br><img src="doc/merge.gif"></div>
<div>云同步<br><img src="doc/sync.gif"></div>
<div>关键字搜索<br><img src="doc/filter.gif"/></div>
<div>SFTP的上传下载<br><img src="doc/sftp.gif"/></div>
<div>选项卡浮动<br><img src="doc/float.gif"/></div>
<div>窗口分割<br><img src="doc/split.gif"/></div>
<div>终端配色表<br><img src="doc/patten.gif"/></div>
<div>VNC终端<br><img src="doc/vnc.gif"/></div>
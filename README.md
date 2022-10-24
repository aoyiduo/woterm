<p align="center">
  <img src="doc/woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant"><br>
  [<a href="README.md">English</a>] | [<a href="README-zh.md">中文</a>]  
</p>

# Overview
Cross platform remote access management tool, out of the box, no configuration, safe and stable.

## Functions description
- Support Windows / Linux / MacOSX.
- Support mainstream communication protocols, including SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALlPORT / VNC / RDP etc.
- SQLite is used to save server related data to avoid mutual overwriting due to server content changes in multiple processes.
- Support database backup and recovery and multi terminal data synchronization to prevent accidental loss.
- Support the tab floating function, which is convenient for multiple windows to work together.
- Support keyword search and filtering to quickly locate target items.
- Support Chinese and English language selection.
  
- Support VNC remote desktop access function, advanced functions need to be supported with [WoVNCServer](http://wovnc.com).
  - Support the standard RFB 3.3/3.7/3.8 protocol.
  - Support the standard encodings such as ZRLE / TRLE / Hextile / CopyRect / RRE / Raw.
  - Support self extend encodings such as H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2.
  - support automatic screen lock when the network is disconnected or remote operation is ended to prevent accidental loss caused by manipulation by others.
  - Support remote black screen mode to prevent the work process from being watched.
  - Support sound playback of remote desktop.
  - Support the free switching of lossless image quality, high-definition image quality, ordinary image quality, classic 16 bit, 15 bit, 8-bit and other image formats.
  - Support multi screen mode and switching modes such as stretching, shared screen and split screen.
- Support remote terminal access function.
  - Support remote Windows/Linux/MacOSX system terminal access.
  - Support Proxy jump access.
  - Support certificate management, including creation, import, export, deletion, etc.
  - Shortcut key configuration is supported.
  - Support ZModem file upload and download.
  - Support embedded SFTP collaborative interaction.
  - Support a variety of terminal color matching for your choice.


## Module description
The client code has been completely open source, and the third-party code which comes from GitHub or CodeProject or other open communities. Some modules come from [WoVNC](http://wovnc.com).

## WoVNCServer
[WoVNCServer](http://www.wovnc.com) is recommended, The server enables more advanced features.

## More information:
<a href="http://www.woterm.com">http://www.woterm.com</a>

## Binary download:
<a href="http://woterm.com">http://woterm.com</a>

## UI preview:
<img src="doc/main.png"/>
<img src="doc/main2.png"/>
<img src="doc/main3.png"/>
<img src="doc/option.png"/>
<img src="doc/identity.png"/>
<img src="doc/floattab.gif"/>
<img src="doc/split.gif"/>
<img src="doc/vnc.png"/>
<img src="doc/color.gif"/>
<img src="doc/vnc.png"/>
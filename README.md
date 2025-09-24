<p align="center">
  <img src="doc/woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">Cross platform / small memory / multi-protocol / multi-function / cloud sync</h2>
<h3 style="text-align: center;">
  [<a href="README.md">English</a>] | 
  [<a href="doc/README-zh_CN.md">简体中文</a>]
  [<a href="doc/README-zh_TW.md">繁體中文</a>]
  [<a href="doc/README-de.md">Deutsch</a>]
  [<a href="doc/README-es.md">Español</a>]
  [<a href="doc/README-fr.md">Français</a>]
  [<a href="doc/README-hi.md">हिंदी</a>]
  [<a href="doc/README-id.md">Bahasa Indonesia</a>]
  [<a href="doc/README-ja.md">日本語</a>]
  [<a href="doc/README-ko.md">한국어</a>]
  [<a href="doc/README-ru.md">Русский</a>]
</h3>
  <h3 style="text-align: center;">[<a href="https://woterm.com">WoTerm offical Website</a>]</a></h3>
</p>

# Overview
Integrated with all major remote communication protocols to meet your needs: supports SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, Serial Port, TCP, UDP, and more — no more switching between tools. Easily handle diverse remote access and communication scenarios in one unified platform.

## Functions description[<a href="https://en.woterm.com/versions/"> &gt;&gt;&gt;click here to check more functions&lt;&lt;&lt;</a>]
- Support Windows / Linux / MacOSX / Android.
- Support mainstream communication protocols, including SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALlPORT / VNC / RDP etc.
- Built in multiple sets of skins for you to choose from, while retaining scalability, allowing friends with strong hands-on abilities to do whatever they want.
- Support administrator mode, which can view session passwords and prevent others from stealing.
- The cloud synchronization protocol built based on standard SFTP allows you to freely create cloud warehouses and provides 8 high-strength encryption algorithms, ensuring data security at your fingertips.
Unlimited backup history and differential merging, addition, deletion, and replacement of records can meet different office environments.
- The SSH terminal is embedded with an SFTP assistant, which is convenient for collaborative operation and also supports SFTP independent tabs, making it more convenient to operate with more functions.
- Floating tab design makes it more convenient for collaborative work between multiple windows.
- Infinite window segmentation, as long as the screen is large enough, you can segment it as you want.
- Provide session grouping and list management, multiple key word search filters, and even with multiple records, it can quickly locate.
- There are dozens of terminal colors, and there is always a favorite one
  
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
The client code has been completely open source(except for the kxver version control module), and the third-party code which comes from GitHub or CodeProject or other open communities. Some modules come from [WoVNC](http://wovnc.com).

## WoVNCServer
[WoVNCServer](http://www.wovnc.com) is recommended, The server enables more advanced features.

## More information:
<a href="http://www.woterm.com">http://www.woterm.com</a>

## Binary download:
<a href="http://woterm.com">http://woterm.com</a>

## High performance

Self-developed terminal parsing engine, capable of parsing and displaying millions of records within seconds, while maintaining extremely low memory usage. [View detailed process](doc/Performance-en.md)

![](doc/timeseq1.png)  
![](doc/urandom_test_speed.png)  
![](doc/urandom_test_memory.png)  

Perform rapid search and location in millions of lines of text.  
![](doc/search.gif)

---

## UI preview

**Main UI**  
![](doc/main.gif)  

**SSH key manage**  
![](doc/keymgr2.gif)  

**Serial port**  
![](doc/serialport.gif)  

**Tunnel**  
![](doc/tunnel.png)  

**Multiple skins**  
![](doc/skins.png)  

**Playbook**  
![](doc/playbook.gif)  

**Merge and separate**  
![](doc/merge.gif)  

**Cloud sync**  
![](doc/sync.gif)  

**Keyword filter**  
![](doc/filter.gif)  

**File transfer**  
![](doc/sftp.gif)  

**Tab float**  
![](doc/float.gif)  

**Window split**  
![](doc/split.gif)  

**Terminal theme**  
![](doc/patten.gif)  

**Syntax highlighting**  
![](doc/highlight.gif)  

**VNC terminal**  
![](doc/vnc.gif)


# Open Source Statement
Since v10, we have adjusted our open-source license. In the future, whether we continue partial open source or move towards full open source, we will maintain an open and transparent approach.  

It should be noted that this repository does not contain any third-party libraries or the source code/library files of the **kxver module**.  
The **kxver module** is mainly used for version control, especially related to professional edition features such as data encryption.  
It is also the team’s only source of funding and an important guarantee for the healthy maintenance and continuous upgrading of the product.  
Before the funding issue is resolved, we do not plan to open-source this module.  

Overall, our goals are twofold:  
1. **Achieve reasonable revenue through appropriate commercialization** — to provide long-term momentum for continuous development and maintenance;  
2. **Ensure code remains available for user auditing** — so everyone can be confident that the software adheres to the principle of “do no evil” and can be used with peace of mind.  

We hope to stay true to our original intention while also ensuring sustainable development, so that the software can go steadily and go far.  

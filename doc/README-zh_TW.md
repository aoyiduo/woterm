*本存儲庫不包含任何第三方庫或 kxver 模組的源碼。  
kxver 模組主要用於版本控制，特別是與終極版本相關的功能，例如資料加密。  
它是團隊的唯一資金來源，也是產品健康維護和升級的重要保障。  
在資金來源未解決之前，沒有計畫開放該模組的源碼。*  
***
<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">跨平台 / 小內存 / 多協議 / 多功能 / 雲端同步</h2>
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
  <h3 style="text-align: center;">[<a href="https://woterm.com">WoTerm 官方網站</a>]</a></h3>
</p>

# 概述
整合所有主要遠端通訊協議以滿足您的需求：支援 SSH1/SSH2、FTP/FTPS、SFTP、TELNET、RLOGIN、RDP、VNC、SHELL、串列埠、TCP、UDP 等 — 無需再切換工具。輕鬆應對各種遠端存取及通訊場景於同一平台。

## 功能描述
- 支援 Windows / Linux / MacOSX / Android。  
- 支援主流通訊協議，包括 SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP 等。  
- 內建多套介面皮膚，可自由選擇並保持可擴展性，讓高手自由自定義。  
- 支援管理員模式，可查看會話密碼並防止被竊取。  
- 基於標準 SFTP 的雲端同步協議，可自由建立雲端倉庫，提供 8 種高強度加密算法，確保資料安全。  
- 無限備份歷史與差異合併，可新增、刪除、替換記錄，滿足不同辦公環境需求。  
- SSH 終端內嵌 SFTP 助手，方便協作操作，亦支援獨立 SFTP 分頁。  
- 浮動分頁設計，更方便多窗口協作。  
- 無限窗口分割，只要螢幕足夠大，可自由分割。  
- 提供會話分組及列表管理、多重關鍵字搜尋篩選，即使多筆記錄也能快速定位。  
- 多達數十種終端顏色，總有一款喜愛。

- 支援 VNC 遠端桌面，進階功能需配合 [WoVNCServer](http://wovnc.com)。  
  - 支援 RFB 3.3/3.7/3.8 標準協議。  
  - 支援標準編碼：ZRLE / TRLE / Hextile / CopyRect / RRE / Raw。  
  - 支援自訂擴展編碼：H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2。  
  - 支援網路斷線或遠端操作結束時自動鎖屏，防止他人操作造成意外損失。  
  - 支援遠端黑屏模式，防止工作過程被觀看。  
  - 支援遠端桌面聲音播放。  
  - 支援無損圖像、高畫質、普通畫質、經典 16/15/8 位等影像格式自由切換。  
  - 支援多螢幕模式及延展、共享、分割模式切換。  

- 支援遠端終端功能。  
  - 支援遠端 Windows/Linux/MacOSX 系統終端存取。  
  - 支援 Proxy jump 存取。  
  - 支援憑證管理，包括建立、匯入、匯出、刪除等。  
  - 支援快捷鍵配置。  
  - 支援 ZModem 文件上傳與下載。  
  - 支援內嵌 SFTP 協作操作。  
  - 支援多種終端配色選擇。  

## 模組說明
客戶端程式碼完全開源（kxver 版本控制模組除外），第三方程式碼來自 GitHub、CodeProject 或其他開源社群。部分模組來自 [WoVNC](http://wovnc.com)。  

## WoVNCServer
推薦使用 [WoVNCServer](http://wovnc.com) 以獲得更多高級功能。  

## 更多資訊:
<a href="http://www.woterm.com">http://www.woterm.com</a>  

## 二進位下載:
<a href="http://woterm.com">http://woterm.com</a>  

## 性能表現:
<div>自研終端解析引擎，數秒內解析並顯千萬行記錄，並保持極低內存。<a href="Performance-zh_TW.md">查看詳細過程<a>
<br><img src="timeseq1.png"/>
<br><img src="urandom_test_speed.png"/>
<br><img src="urandom_test_memory.png"/>
</div>
<div>在百萬行文本中，極速搜索及定位<br><img src="search.gif"/></div>

## UI 預覽:
<div>主介面<br><img src="main.gif"/></div>
<div>SSH 金鑰管理<br><img src="keymgr2.gif"></div>
<div>串列埠<br><img src="serialport.gif"></div>
<div>通道<br><img src="tunnel.png"></div>
<div>多套介面皮膚<br><img src="skins.png"></div>
<div>Playbook<br><img src="playbook.gif"></div>
<div>合併與拆分<br><img src="merge.gif"></div>
<div>雲端同步<br><img src="sync.gif"></div>
<div>關鍵字篩選<br><img src="filter.gif"></div>
<div>文件傳輸<br><img src="sftp.gif"></div>
<div>浮動分頁<br><img src="float.gif"></div>
<div>窗口分割<br><img src="split.gif"></div>
<div>終端主題<br><img src="patten.gif"></div>
<div>語法高亮<br><img src="highlight.gif"/></div>
<div>VNC 終端<br><img src="vnc.gif"/></div>

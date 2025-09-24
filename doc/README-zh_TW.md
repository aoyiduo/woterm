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

## 功能描述[<a href="https://cn.woterm.com/versions/">&gt;&gt;&gt;點擊此處查看更多功能&lt;&lt;&lt;</a>]
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

## 性能表現

自研終端解析引擎，數秒內解析並顯示千萬行記錄，並保持極低內存。  
[查看詳細過程](Performance-zh_TW.md)  

![](timeseq1.png)  
![](urandom_test_speed.png)  
![](urandom_test_memory.png)  

在百萬行文本中，極速搜索及定位  
![](search.gif)  


## UI 預覽

**主介面**  
![](main.gif)  

**SSH 金鑰管理**  
![](keymgr2.gif)  

**串列埠**  
![](serialport.gif)  

**通道**  
![](tunnel.png)  

**多套介面皮膚**  
![](skins.png)  

**Playbook**  
![](playbook.gif)  

**合併與拆分**  
![](merge.gif)  

**雲端同步**  
![](sync.gif)  

**關鍵字篩選**  
![](filter.gif)  

**文件傳輸**  
![](sftp.gif)  

**浮動分頁**  
![](float.gif)  

**窗口分割**  
![](split.gif)  

**終端主題**  
![](patten.gif)  

**語法高亮**  
![](highlight.gif)  

**VNC 終端**  
![](vnc.gif)  


# 開源聲明
自 v10 起，我們對開源協議進行了調整。未來，無論是繼續保持部分開源，還是逐步邁向完全開源，我們都會保持開放與透明的態度。  

需要說明的是：本倉庫不包含任何第三方庫及 **kxver 模組** 的原始碼或庫文件。  
其中 **kxver 模組** 主要用於版本控制，尤其與專業版功能（如數據加密）相關。  
它同時也是團隊唯一的資金來源，是產品健康維護與持續升級的重要保障。  
在資金來源問題尚未解決之前，我們暫不計劃開放該模組。  

整體而言，我們的目標有兩個：  
1. **透過適度商業化獲取合理收益** —— 為持續開發與維護提供長期動力；  
2. **保持程式碼可供使用者審計** —— 讓大家能確認軟體遵循「不作惡」的原則，安心使用。  

我們希望在堅持初心的同時，也能兼顧可持續發展，讓軟體既能走得穩，也能走得遠。  

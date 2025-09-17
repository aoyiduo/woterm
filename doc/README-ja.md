*このリポジトリには kxver モジュールのサードパーティライブラリやソースコードは含まれていません。  
kxver モジュールは主にバージョン管理に使用され、特にアルティメットバージョンの機能（データ暗号化など）に関連しています。  
チームの唯一の資金源であり、製品の健康維持とアップグレードの重要な保証です。  
資金源が解決されるまでは、このモジュールのソースコードを公開する予定はありません。*  
***
<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">クロスプラットフォーム / 小メモリ / マルチプロトコル / マルチ機能 / クラウド同期</h2>
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
  <h3 style="text-align: center;">[<a href="https://woterm.com">WoTerm 公式サイト</a>]</a></h3>
</p>

# 概要
主要なリモート通信プロトコルすべてに統合：SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, シリアルポート, TCP, UDP などに対応 — ツールの切り替え不要。  
統合プラットフォームで多様なリモートアクセスシナリオを簡単に管理。

## 機能説明
- Windows / Linux / MacOSX / Android 対応。  
- 主流通信プロトコル対応：SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP など。  
- 複数の組み込みスキン、拡張可能、上級ユーザーが自由にカスタマイズ可能。  
- 管理者モード：セッションパスワード閲覧、盗難防止。  
- SFTPベースのクラウド同期プロトコル、8つの高強度暗号アルゴリズム。  
- SSHターミナルにSFTPアシスタント統合、独立タブもサポート。  
- 浮動タブデザイン、多ウィンドウ共同作業に便利。  
- 無制限ウィンドウ分割。  
- セッショングループとキーワードフィルター。  
- ターミナルカラー多数。

- VNCリモートデスクトップ対応、高度な機能は [WoVNCServer](http://wovnc.com) 推奨。  
  - RFB 3.3/3.7/3.8 標準プロトコル対応。  
  - 標準エンコーディング対応：ZRLE / TRLE / Hextile / CopyRect / RRE / Raw。  
  - 拡張エンコーディング対応：H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2。  
  - ネットワーク切断時やリモート操作終了時の自動画面ロック。  
  - リモートブラックスクリーンモード。  
  - リモートデスクトップ音声再生。  
  - 画質切替自由：ロスレス、HD、通常、16/15/8ビット。  
  - マルチスクリーン、ストレッチ、共有、分割モード対応。

- リモートターミナル対応。  
  - Windows/Linux/MacOSX ターミナルアクセス。  
  - プロキシジャンプ対応。  
  - 証明書管理。  
  - ショートカットキー設定。  
  - ZModem ファイルアップロード/ダウンロード。  
  - 組み込みSFTP協力操作。  
  - 複数ターミナルカラー選択可能。

## モジュール説明
クライアントコードは完全にオープンソース（kxverモジュール除く）、サードパーティコードは GitHub, CodeProject または他のオープンコミュニティ由来。いくつかのモジュールは [WoVNC](http://wovnc.com) 由来。

## WoVNCServer
高度な機能のため [WoVNCServer](http://wovnc.com) 推奨。

## 詳細情報:
<a href="http://www.woterm.com">http://www.woterm.com</a>

## バイナリダウンロード:
<a href="http://woterm.com">http://woterm.com</a>

## パフォーマンス:
<div>自社開発のターミナル解析エンジンで、数秒以内に数千万行を解析・表示し、極めて低いメモリ使用を維持します。<a href="Performance-ja.md">詳細なプロセスを確認<a>
<br><img src="timeseq1.png"/>
<br><img src="urandom_test_speed.png"/>
<br><img src="urandom_test_memory.png"/>
</div>
<div>数百万行のテキスト内での超高速検索と位置特定<br><img src="search.gif"/></div>


## UIプレビュー:
<div>メインUI<br><img src="main.gif"/></div>
<div>SSHキー管理<br><img src="keymgr2.gif"></div>
<div>シリアルポート<br><img src="serialport.gif"></div>
<div>トンネル<br><img src="tunnel.png"></div>
<div>複数スキン<br><img src="skins.png"></div>
<div>プレイブック<br><img src="playbook.gif"></div>
<div>結合と分割<br><img src="merge.gif"></div>
<div>クラウド同期<br><img src="sync.gif"></div>
<div>キーワードフィルター<br><img src="filter.gif"></div>
<div>ファイル転送<br><img src="sftp.gif"></div>
<div>浮動タブ<br><img src="float.gif"></div>
<div>ウィンドウ分割<br><img src="split.gif"></div>
<div>ターミナルテーマ<br><img src="patten.gif"></div>
<div>構文ハイライト<br><img src="highlight.gif"/></div>
<div>VNCターミナル<br><img src="vnc.gif"/></div>

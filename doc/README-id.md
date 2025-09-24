<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">Lintas platform / memori kecil / multi-protokol / multi-fungsi / sinkronisasi cloud</h2>
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
  <h3 style="text-align: center;">[<a href="https://woterm.com">Situs resmi WoTerm</a>]</a></h3>
</p>

# Ikhtisar
Terintegrasi dengan semua protokol komunikasi jarak jauh utama: mendukung SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, Port Serial, TCP, UDP, dan lainnya — tidak perlu lagi berganti alat.  
Tangani berbagai skenario akses jarak jauh dalam satu platform terpadu.

## Deskripsi fungsi[<a href="https://en.woterm.com/versions/">&gt;&gt;&gt;Klik di sini untuk melihat lebih banyak fitur&lt;&lt;&lt;</a>]
- Mendukung Windows / Linux / MacOSX / Android.  
- Mendukung protokol utama: SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP, dll.  
- Banyak tema bawaan yang dapat diperluas, memungkinkan pengguna mahir menyesuaikan sesuka hati.  
- Mode administrator untuk melihat kata sandi sesi dan mencegah pencurian.  
- Protokol sinkronisasi cloud berbasis SFTP, 8 algoritme enkripsi kuat.  
- Terminal SSH dengan asisten SFTP bawaan, mendukung tab SFTP independen.  
- Desain tab mengambang, memudahkan kolaborasi multi-jendela.  
- Segmentasi jendela tak terbatas.  
- Pengelompokan sesi dan filter kata kunci.  
- Puluhan warna terminal tersedia.

- Mendukung desktop jarak jauh VNC, fitur lanjutan memerlukan [WoVNCServer](http://wovnc.com).  
  - Mendukung protokol RFB 3.3/3.7/3.8.  
  - Mendukung encoding standar: ZRLE / TRLE / Hextile / CopyRect / RRE / Raw.  
  - Mendukung encoding tambahan: H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2.  
  - Kunci layar otomatis saat jaringan terputus atau operasi jarak jauh selesai.  
  - Mode layar hitam jarak jauh.  
  - Pemutaran suara desktop jarak jauh.  
  - Beralih bebas kualitas gambar: lossless, HD, biasa, 16/15/8 bit.  
- Mendukung multi-layar dan mode stretch, shared screen, split screen.  

- Dukungan terminal jarak jauh.  
  - Akses terminal Windows/Linux/MacOSX.  
  - Dukungan Proxy jump.  
  - Manajemen sertifikat.  
  - Konfigurasi shortcut key.  
  - Upload/download file ZModem.  
  - Interaksi kolaboratif SFTP bawaan.  
  - Pilihan tema warna terminal.

## Deskripsi modul
Kode klien sepenuhnya open source (kecuali modul kontrol versi kxver), kode pihak ketiga berasal dari GitHub, CodeProject, atau komunitas terbuka lainnya. Beberapa modul dari [WoVNC](http://wovnc.com).  

## WoVNCServer
Disarankan [WoVNCServer](http://wovnc.com) untuk fitur lanjutan.  

## Informasi lebih lanjut:
<a href="http://www.woterm.com">http://www.woterm.com</a>  

## Unduh biner:
<a href="http://woterm.com">http://woterm.com</a>  

## Kinerja

Mesin parsing terminal buatan sendiri, memproses dan menampilkan puluhan juta baris dalam hitungan detik dengan penggunaan memori yang sangat rendah. [Lihat proses detail](Performance-id.md)

![](timeseq1.png)  
![](urandom_test_speed.png)  
![](urandom_test_memory.png)  

Pencarian dan pelokalan super cepat di dalam jutaan baris teks  
![](search.gif)

---

## Pratinjau UI

**UI utama**  
![](main.gif)  

**Manajemen kunci SSH**  
![](keymgr2.gif)  

**Port serial**  
![](serialport.gif)  

**Terowongan**  
![](tunnel.png)  

**Beberapa tema**  
![](skins.png)  

**Playbook**  
![](playbook.gif)  

**Gabung dan pisah**  
![](merge.gif)  

**Sinkronisasi cloud**  
![](sync.gif)  

**Filter kata kunci**  
![](filter.gif)  

**Transfer file**  
![](sftp.gif)  

**Tab mengambang**  
![](float.gif)  

**Pembagian jendela**  
![](split.gif)  

**Tema terminal**  
![](patten.gif)  

**Penyorotan sintaksis**  
![](highlight.gif)  

**Terminal VNC**  
![](vnc.gif)



# Pernyataan Sumber Terbuka
Sejak versi v10, kami telah menyesuaikan lisensi open source kami. Ke depan, baik kami tetap mempertahankan open source sebagian maupun beralih secara bertahap ke open source penuh, kami akan selalu menjaga sikap terbuka dan transparan.  

Perlu dicatat bahwa repositori ini tidak berisi pustaka pihak ketiga atau kode sumber/berkas pustaka dari **modul kxver**.  
**Modul kxver** terutama digunakan untuk kontrol versi, khususnya terkait dengan fitur edisi profesional seperti enkripsi data.  
Modul ini juga merupakan satu-satunya sumber pendanaan tim dan menjadi jaminan penting untuk pemeliharaan yang sehat serta peningkatan produk secara berkelanjutan.  
Sampai masalah pendanaan terselesaikan, kami tidak berencana untuk membuka modul ini.  

Secara keseluruhan, tujuan kami ada dua:  
1. **Mencapai pendapatan yang wajar melalui komersialisasi yang tepat** — untuk memberikan dorongan jangka panjang bagi pengembangan dan pemeliharaan berkelanjutan;  
2. **Menjaga kode tetap tersedia untuk audit pengguna** — sehingga semua orang dapat yakin bahwa perangkat lunak ini mematuhi prinsip “tidak berbuat jahat” dan dapat digunakan dengan tenang.  

Kami berharap tetap setia pada niat awal kami, sekaligus memastikan pembangunan yang berkelanjutan, agar perangkat lunak ini dapat berjalan stabil dan bertahan lama.  

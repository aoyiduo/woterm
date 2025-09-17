
---

# Laporan Pengujian Performa WoTerm

Pengujian ini dilakukan pada **versi v10**, karena versi setelahnya telah mengalami optimasi inti yang mendalam.  
Terima kasih kepada netizen atas dorongan dan dukungannya, serta kepada pengembang **WindTerm**: meski belum pernah bertemu dan tidak ada pertukaran teknis, WindTerm memang perangkat lunak yang luar biasa dan menginspirasi beberapa fitur WoTerm.

---

## 🖥 Lingkungan Pengujian

| Komponen        | Konfigurasi                              |
|-----------------|------------------------------------------|
| **Host WoTerm** | i5 generasi ke-12, RAM 48 GB             |
| **Server Uji**  | 2 inti, 8 GB RAM, Fedora, VM mode NAT    |

---

## ⚙ Perintah Pengujian

| Perintah            | Keterangan                     |
|---------------------|--------------------------------|
| `time seq 10000000` | Menghasilkan 10 juta baris     |
| `urandom_test.sh`   | Menghasilkan 100 MB data acak  |

---

## 📦 Versi Program

| Program     | Versi   |
|-------------|---------|
| **WoTerm**   | v10.2.1 |
| **WindTerm** | v2.7.0  |
| **PuTTY**    | v0.83   |

---

## ⏱ Metodologi

- **Tanggal pengujian:** 18 April 2025  
- Setiap perintah dijalankan **minimal 10 kali**  
- **Nilai maksimal/minimal dibuang**, rata-rata diambil  

---

## Waktu Pemrosesan Perintah

Perintah `time seq 10000000` dijalankan pada server, durasi total diukur dengan stopwatch ponsel.  
Waktu yang dicetak perintah hanya mencerminkan akhir penerimaan data SSH, tidak adil.  
Alur data:

*thread SSH → thread utama → thread terminal → tampilan utama*

Maka digunakan stopwatch.  
<div>Waktu pemrosesan perintah<br><img src="timeseq1.png"/></div>

## Throughput urandom_test

Membandingkan **rekaman per detik (throughput)**.  
<div>Throughput eksekusi<br><img src="urandom_test_speed.png"/></div>

## Penggunaan Memori Setelah urandom_test

Menampilkan **penambahan memori** setelah perintah (dikurangi baseline awal).  
<div>Penggunaan memori<br><img src="urandom_test_memory.png"/></div>

*Catatan: hasil berdasarkan konfigurasi di atas, perangkat keras lain bisa berbeda.*
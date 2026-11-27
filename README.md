# Game Waktu Reaksi — ESP32 + Tombol + LED + Buzzer

Refleksmu lambat atau cepat? Buktikan dalam tiga detik! Game kecil ini
mengukur waktu reaksimu: tekan tombol MULAI, tunggu LED hijau menyala
(setelah jeda acak 2–5 detik yang tidak bisa ditebak), lalu tekan tombol
REAKSI secepat mungkin. ESP32 menghitung selisih waktunya dalam
milidetik dan menampilkannya di Serial Monitor, lengkap dengan skor
terbaikmu. Kalau kamu menekan tombol reaksi SEBELUM LED menyala, itu
dianggap curang — buzzer berbunyi panjang dan ronde diulang dari awal.

Video penjelasan (60 detik):
link menyusul setelah upload

## Komponen

| Bagian | Keterangan |
|---|---|
| ESP32 DevKit | Otak game, menghitung waktu reaksi |
| Tombol tekan × 2 | Tombol MULAI dan tombol REAKSI (aktif LOW) |
| LED hijau + resistor 220 ohm | Sinyal "tekan sekarang!" |
| Buzzer aktif 5V | Bunyi saat LED menyala, saat hasil keluar, dan saat curang |
| Adaptor USB 5V 2A | Catu daya |

## Wiring

| Dari | Ke |
|---|---|
| Tombol MULAI kaki 1 | ESP32 GPIO12 |
| Tombol MULAI kaki 2 | GND |
| Tombol REAKSI kaki 1 | ESP32 GPIO13 |
| Tombol REAKSI kaki 2 | GND |
| LED hijau anoda | ESP32 GPIO25 |
| LED hijau katoda | Resistor 220 ohm, lalu GND |
| Buzzer + | ESP32 GPIO26 |
| Buzzer − | GND |
| Adaptor USB 5V | ESP32 VIN |
| Semua GND | Digabung |

Skema lengkap: [schematic.png](schematic.png)

## Cara kerja

Game berjalan dalam empat state:

1. **MENUNGGU_MULAI** — menunggu tombol MULAI ditekan.
2. **MENUNGGU_LED** — jeda acak 2–5 detik berjalan (diundi dengan
   `random()` tiap ronde). Selama jeda ini LED masih mati; menekan
   tombol REAKSI sekarang berarti curang.
3. **MENUNGGU_REAKSI** — LED menyala, buzzer berbunyi pendek, dan
   ESP32 mencatat `millis()`. Begitu tombol REAKSI ditekan, selisih
   waktu dihitung dalam milidetik.
4. **TAMPIL_HASIL** — hasil dan skor terbaik tampil di Serial Monitor,
   buzzer berbunyi dua kali, lalu tekan MULAI untuk ronde berikutnya.

Tombol dibaca dengan `INPUT_PULLUP` (aktif LOW, tidak butuh resistor
pull-up eksternal) plus debounce 50 ms dan deteksi transisi turun, jadi
menahan tombol tidak memicu berulang dan pantulan mekanik tidak
dihitung dua kali. Skor terbaik disimpan di RAM selama alat menyala
(hilang saat alat dimatikan).

Sebagai patokan: rata-rata waktu reaksi visual manusia dewasa sekitar
250 ms. Di bawah 200 ms hampir selalu berarti menebak, bukan bereaksi
— makanya jeda acaknya penting.

## Konfigurasi di sketch

| Konstanta | Nilai bawaan | Arti |
|---|---|---|
| `PIN_TOMBOL_MULAI` | 12 | Tombol mulai (aktif LOW) |
| `PIN_TOMBOL_REAKSI` | 13 | Tombol reaksi (aktif LOW) |
| `PIN_LED_HIJAU` | 25 | LED hijau via resistor 220 ohm |
| `PIN_BUZZER` | 26 | Buzzer aktif (HIGH bunyi) |
| `JEDA_ACAK_MIN_MS` | 2000 | Jeda terpendek sebelum LED menyala |
| `JEDA_ACAK_MAKS_MS` | 5000 | Jeda terpanjang sebelum LED menyala |
| `DEBOUNCE_MS` | 50 | Saring pantulan mekanik tombol |
| `BAUD_SERIAL` | 115200 | Kecepatan Serial Monitor |

## Compile (terverifikasi)

- Board: ESP32 DevKit (esp32:esp32:esp32), ESP32 core 4.0.0, arduino-cli 1.6.0
- Sketch: 298.668 byte (22%) dari program storage; RAM 23.452 byte (7%)
- Library: tidak butuh library tambahan (hanya Arduino core)

## Catatan

- Kedua tombol memakai `INPUT_PULLUP` internal — satu kaki ke GPIO,
  satu kaki ke GND, tanpa resistor tambahan.
- LED hijau WAJIB lewat resistor 220 ohm; tanpa resistor LED bisa rusak
  dan pin GPIO25 bisa kelebihan arus.
- Buzzer yang dipakai adalah buzzer AKTIF (bunyi saat diberi HIGH).
  Buzzer pasif tidak akan berbunyi dengan sketch ini.
- Kalau hasil selalu 0 ms atau sangat kecil, periksa debounce dan
  pastikan tombol reaksi tidak tertahan saat LED menyala.

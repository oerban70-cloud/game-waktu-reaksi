/*
  Game Waktu Reaksi dengan ESP32 — tombol + LED + buzzer
  ======================================================
  Cara main:
  1. Tekan tombol MULAI. ESP32 lalu menunggu dalam diam selama waktu
     ACAK antara 2 sampai 5 detik (tidak bisa ditebak).
  2. Begitu LED hijau menyala, segera tekan tombol REAKSI secepat
     mungkin. ESP32 menghitung selisih waktunya dalam milidetik.
  3. Hasilnya tampil di Serial Monitor (115200 baud) lengkap dengan
     skor terbaik. Buzzer berbunyi pendek saat LED menyala, dan
     berbunyi dua kali saat hasil keluar.

  Anti curang:
  Kalau tombol REAKSI ditekan SEBELUM LED menyala, itu dianggap
  curang — buzzer berbunyi panjang dengan nada rendah, LED tetap
  mati, dan ronde diulang dari awal.

  Catatan waktu reaksi manusia:
  Rata-rata waktu reaksi visual manusia dewasa sekitar 250 ms.
  Di bawah 200 ms hampir selalu berarti menebak, bukan bereaksi.

  Video penjelasan (60 detik): link menyusul setelah upload
  Repo: https://github.com/oerban70-cloud/game-waktu-reaksi
*/

// ---------- Konfigurasi pin ----------
const uint8_t PIN_TOMBOL_MULAI  = 12;  // tombol mulai, aktif LOW (INPUT_PULLUP)
const uint8_t PIN_TOMBOL_REAKSI = 13;  // tombol reaksi, aktif LOW (INPUT_PULLUP)
const uint8_t PIN_LED_HIJAU     = 25;  // LED hijau via resistor 220 ohm ke GND
const uint8_t PIN_BUZZER        = 26;  // buzzer aktif, bunyi saat HIGH
const uint8_t PIN_ACAK          = 34;  // pin analog mengambang, sumber seed random

// ---------- Konfigurasi waktu (milidetik) ----------
const unsigned long JEDA_ACAK_MIN_MS  = 2000;  // jeda terpendek sebelum LED menyala
const unsigned long JEDA_ACAK_MAKS_MS = 5000;  // jeda terpanjang sebelum LED menyala
const unsigned long DEBOUNCE_MS       = 50;    // saring pantulan mekanik tombol
const unsigned long DURASI_BIP_MS     = 100;   // bunyi pendek buzzer
const unsigned long DURASI_BIP_CURANG_MS = 600;  // bunyi panjang tanda curang
const unsigned long BAUD_SERIAL       = 115200;

// ---------- State permainan ----------
enum StateGame {
  MENUNGGU_MULAI,    // menunggu tombol MULAI ditekan
  MENUNGGU_LED,      // jeda acak berjalan, LED masih mati
  MENUNGGU_REAKSI,   // LED menyala, menunggu tombol REAKSI
  TAMPIL_HASIL       // hasil sudah tampil, kembali ke awal
};

StateGame stateGame = MENUNGGU_MULAI;
unsigned long waktuLedMenyalaMs  = 0;  // millis() saat LED dinyalakan
unsigned long waktuJedaBerakhirMs = 0;  // millis() saat jeda acak selesai
unsigned long skorTerbaikMs = 0;       // 0 = belum ada skor (hilang saat alat mati)

// ---------- Bunyi buzzer ----------
// Bunyi pendek sekali (tanda LED menyala atau hasil keluar).
void bunyiPendek() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(DURASI_BIP_MS);
  digitalWrite(PIN_BUZZER, LOW);
}

// Bunyi dua kali pendek (tanda hasil sudah tampil).
void bunyiHasil() {
  bunyiPendek();
  delay(DURASI_BIP_MS);
  bunyiPendek();
}

// Bunyi panjang sekali (tanda pemain curang menekan terlalu cepat).
void bunyiCurang() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(DURASI_BIP_CURANG_MS);
  digitalWrite(PIN_BUZZER, LOW);
}

// ---------- Baca tombol dengan debounce ----------
// Mengembalikan true hanya pada TRANSISI turun (HIGH -> LOW) yang
// bertahan selama DEBOUNCE_MS. Menahan tombol tidak memicu berulang,
// dan pantulan mekanik tombol tidak dihitung sebagai dua kali tekan.
// Parameter terakhirTekanMs dan sebelumnyaTinggi menyimpan state tiap
// tombol di antara pemanggilan (satu pasang per tombol).
bool tombolBaruDitekan(uint8_t pinTombol, unsigned long &terakhirTekanMs,
                       bool &sebelumnyaTinggi) {
  bool sekarangTinggi = (digitalRead(pinTombol) == HIGH);
  bool ditekan = false;
  if (sebelumnyaTinggi && !sekarangTinggi) {
    unsigned long sekarangMs = millis();
    if (sekarangMs - terakhirTekanMs >= DEBOUNCE_MS) {
      terakhirTekanMs = sekarangMs;
      ditekan = true;
    }
  }
  sebelumnyaTinggi = sekarangTinggi;
  return ditekan;
}

unsigned long terakhirTekanMulaiMs  = 0;
unsigned long terakhirTekanReaksiMs = 0;
bool tombolMulaiSebelumnyaTinggi  = true;
bool tombolReaksiSebelumnyaTinggi = true;

// ---------- Mulai ronde baru ----------
// Matikan LED dan buzzer, lalu undi jeda acak 2-5 detik.
void mulaiRondeBaru() {
  digitalWrite(PIN_LED_HIJAU, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  unsigned long jedaAcakMs = random(JEDA_ACAK_MIN_MS, JEDA_ACAK_MAKS_MS + 1);
  waktuJedaBerakhirMs = millis() + jedaAcakMs;
  stateGame = MENUNGGU_LED;
  Serial.println("Ronde dimulai... tunggu LED menyala, JANGAN tekan dulu!");
}

// ---------- Tampilkan hasil ----------
void tampilkanHasil(unsigned long waktuReaksiMs) {
  digitalWrite(PIN_LED_HIJAU, LOW);
  Serial.print("Waktu reaksimu: ");
  Serial.print(waktuReaksiMs);
  Serial.println(" ms");
  if (skorTerbaikMs == 0 || waktuReaksiMs < skorTerbaikMs) {
    skorTerbaikMs = waktuReaksiMs;
    Serial.println("Skor terbaik baru!");
  }
  Serial.print("Skor terbaik: ");
  Serial.print(skorTerbaikMs);
  Serial.println(" ms");
  Serial.println("Tekan tombol MULAI untuk main lagi.");
  bunyiHasil();
  stateGame = TAMPIL_HASIL;
}

void setup() {
  Serial.begin(BAUD_SERIAL);
  pinMode(PIN_TOMBOL_MULAI, INPUT_PULLUP);
  pinMode(PIN_TOMBOL_REAKSI, INPUT_PULLUP);
  pinMode(PIN_LED_HIJAU, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED_HIJAU, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Seed acak dari pin analog yang mengambang + waktu boot.
  randomSeed(analogRead(PIN_ACAK) + millis());

  Serial.println("=== GAME WAKTU REAKSI ===");
  Serial.println("Tekan tombol MULAI untuk mulai bermain.");
}

void loop() {
  switch (stateGame) {
    case MENUNGGU_MULAI:
      if (tombolBaruDitekan(PIN_TOMBOL_MULAI, terakhirTekanMulaiMs, tombolMulaiSebelumnyaTinggi)) {
        mulaiRondeBaru();
      }
      break;

    case MENUNGGU_LED:
      // Curang: tombol reaksi ditekan sebelum LED menyala.
      if (tombolBaruDitekan(PIN_TOMBOL_REAKSI, terakhirTekanReaksiMs, tombolReaksiSebelumnyaTinggi)) {
        Serial.println("CURANG! Kamu menekan sebelum LED menyala. Ronde diulang.");
        bunyiCurang();
        stateGame = MENUNGGU_MULAI;
        Serial.println("Tekan tombol MULAI untuk mulai lagi.");
        break;
      }
      // Jeda acak selesai: nyalakan LED, bunyikan buzzer, mulai hitung.
      if (millis() >= waktuJedaBerakhirMs) {
        digitalWrite(PIN_LED_HIJAU, HIGH);
        waktuLedMenyalaMs = millis();
        bunyiPendek();
        Serial.println("LED MENYALA! Tekan tombol REAKSI sekarang!");
        stateGame = MENUNGGU_REAKSI;
      }
      break;

    case MENUNGGU_REAKSI:
      if (tombolBaruDitekan(PIN_TOMBOL_REAKSI, terakhirTekanReaksiMs, tombolReaksiSebelumnyaTinggi)) {
        tampilkanHasil(millis() - waktuLedMenyalaMs);
      }
      break;

    case TAMPIL_HASIL:
      if (tombolBaruDitekan(PIN_TOMBOL_MULAI, terakhirTekanMulaiMs, tombolMulaiSebelumnyaTinggi)) {
        mulaiRondeBaru();
      }
      break;
  }
}

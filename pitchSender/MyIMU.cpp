#include <Arduino.h>
#include "MyIMU.h"

// [130950]6軸IMUセンサーモジュール AE-LSM6DSV16X
// https://akizukidenshi.com/catalog/g/g130950/
// 本プログラムは上記モジュールを簡易的に動作確認するプログラムです。
//
// ======== 注意！ ========
// 本モジュールは5V系のマイコンボード（Arduino Unoなど）ではお使いいただけません！
// Raspberry Pi PicoやEPS32など3.3V系のマイコンボードに付属のQwiicケーブルを使い以下のように繋いでください。
//  黒 : GND
//  赤 : 3.3V (5V不可)
//  青 : SDA
//  黄 : SCL
// 本サンプルプログラムはI2Cで通信するため、J2（「6B」刻印近く）もしくはJ3（「6A」刻印近く）のジャンパーパターンをハンダで短絡させてください。
// また、プルアップ抵抗を外部で用意しない場合は、J1もハンダで短絡させてください。
//
// (株）秋月電子通商 2025-12-22

#include <Wire.h>

// 使用するシリアルやI2Cポートに応じて変更してください。
auto& mySerial = Serial;
auto& myWire = Wire;

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr int CLOCK_SPEED = 400000;

constexpr uint8_t LSM6_ADDR_GND = 0x6A;  // J3を短絡したとき(SDO/SA0 : LOW)のアドレス.
constexpr uint8_t LSM6_ADDR_VCC = 0x6B;  // J2を短絡したとき(SDO/SA : HIGH)のアドレス.
uint8_t lsm6_addr = 0;

// レジスタマップ -> `https://akizukidenshi.com/goodsaffix/lsm6dsv16x.pdf#page=52`.
constexpr uint8_t REG_WHO_AM_I = 0x0F;
constexpr uint8_t REG_CTRL1 = 0x10;
constexpr uint8_t REG_CTRL2 = 0x11;
constexpr uint8_t REG_CTRL3 = 0x12;
constexpr uint8_t REG_CTRL6 = 0x15;
constexpr uint8_t REG_CTRL8 = 0x17;
constexpr uint8_t REG_OUTX_L_G = 0x22;

constexpr uint8_t REG_FUNC_CFG_ACCESS = 0x01;     // 組み込み関数へのアクセス設定.
constexpr uint8_t REG_EMB_FUNC_EN_A = 0x04;       // 組み込み関数有効化の設定.
constexpr uint8_t REG_EMB_FUNC_INIT_A = 0x66;     // 組み込み関数初期化の設定．
constexpr uint8_t REG_EMB_FUNC_FIFO_EN_A = 0x44;  // 組み込み関数用FIFOの設定.

constexpr uint8_t REG_FIFO_CTRL3 = 0x09;         // FIFOの通信速度設定.
constexpr uint8_t REG_FIFO_CTRL4 = 0x0A;         // FIFOの設定.
constexpr uint8_t REG_FIFO_DATA_OUT_TAG = 0x78;  // FIFOタグの位置.
constexpr uint8_t REG_FIFO_STATUS1 = 0x1B;       // 未読のセンサーデータの数(7~0).
constexpr uint8_t REG_FIFO_STATUS2 = 0x1C;       // 未読のセンサーデータの数(8).
constexpr uint8_t REG_FIFO_DATA_OUT = 0x79;      // FIFOデータの開始位置（長さは6バイト）.

constexpr uint8_t TAG_FIFO_QUATERNION = 0x13;  // クオータニオンのFIFOタグ.

struct Quaternion {  // クオータニオン用の構造体.
  float w, x, y, z;
};
volatile Quaternion qua;

volatile EulerAngles angles;

void writeReg(uint8_t reg, uint8_t val) {
  myWire.beginTransmission(lsm6_addr);
  myWire.write(reg);
  myWire.write(val);
  myWire.endTransmission();
}

void readRegs(uint8_t startReg, uint8_t* buf, uint8_t len) {
  myWire.beginTransmission(lsm6_addr);
  myWire.write(startReg);
  myWire.endTransmission(false);
  myWire.requestFrom(lsm6_addr, len);
  for (uint8_t i = 0; i < len; i++) { buf[i] = myWire.read(); }
}

bool lsm6_init() {
  // デバイス固有IDチェック
  uint8_t who;
  readRegs(REG_WHO_AM_I, &who, 1);
  mySerial.print("WHO_AM_I = 0x");
  mySerial.println(who, HEX);
  if (who != 0x70) { return false; }

  // ===== SYSTEM =====
  writeReg(REG_CTRL3, 0b00000001);  // ソフトウェアのリセット.
  delay(10);                        // リセット待ち.
  writeReg(REG_CTRL3, 0b01000100);
  // Block Data Updateの有効化（読み出し中のデータ更新をブロック）.
  // 複数バイト読み出し時にアドレスを自動で進める機能の有効化.

  // ===== EMB_FUNC =====
  writeReg(REG_FUNC_CFG_ACCESS, 0b10000000);     // 組み込み関数の設定へのアクセスを開始.
  writeReg(REG_EMB_FUNC_EN_A, 0b00000010);       // センサーフュージョンのみ有効化.
  writeReg(REG_EMB_FUNC_INIT_A, 0b00000010);     // センサーフュージョンを初期化.
  writeReg(REG_EMB_FUNC_FIFO_EN_A, 0b00000010);  // クオータニオンのFIFOへの送信を有効化.
  writeReg(REG_FUNC_CFG_ACCESS, 0b00000000);     // 組み込み関数の設定へのアクセスを停止.

  // ===== FIFO =====
  writeReg(REG_FIFO_CTRL3, 0b10011001);  // BDR_GY=960Hz, BDR_XL=960Hz
  writeReg(REG_FIFO_CTRL4, 0b00000110);  // FIFOを連続書込モードに設定.

  // ===== SENSOR =====
  // 初期設定
  // writeReg(REG_CTRL1, 0b00001001);
  writeReg(REG_CTRL1, 0b10011001);  // ODR=960Hz
  // writeReg(REG_CTRL2, 0b00001001);
  writeReg(REG_CTRL2, 0b10011001);  // ODR=960Hz
  writeReg(REG_CTRL6, 0b00000100);
  writeReg(REG_CTRL8, 0b00000000);  // 低周波フィルタOFF

  delay(50);

  writeReg(REG_FIFO_CTRL4, 0b00000000);  // FIFOを一度無効に（空に）.
  writeReg(REG_FIFO_CTRL4, 0b00000110);  // FIFOを連続書込モードに.

  return true;
}

void Normalize();
void ToEulerAngles();

void quick_test() {
  // 例：Z軸に +30° 回転の四元数（度 → ラジアン）
  float deg = 30.0f;
  float rad = deg * PI / 180.0f;
  qua.w = cosf(rad / 2.0f);  // ≈ 0.9659
  qua.x = 0.0f;
  qua.y = 0.0f;
  qua.z = sinf(rad / 2.0f);  // ≈ 0.2588

  Normalize();
  ToEulerAngles();

  Serial.print("TEST Euler: ");
  Serial.print(angles.roll, 3);
  Serial.print(" ");
  Serial.print(angles.pitch, 3);
  Serial.print(" ");
  Serial.println(angles.yaw, 3);  // ← だいたい (0, 0, 30) 付近になる
}

void init_imu() {
  myWire.begin(SDA_PIN, SCL_PIN, CLOCK_SPEED);
  //mySerial.begin(115200);
  //while (!mySerial);
  delay(100);

  mySerial.println("\n\nStart!");

  myWire.beginTransmission(LSM6_ADDR_GND);
  if (myWire.endTransmission() == 0) {
    mySerial.print("LSM6DSV16X found.\naddress: 0x");
    mySerial.println(LSM6_ADDR_GND, 16);
    lsm6_addr = LSM6_ADDR_GND;
  }

  myWire.beginTransmission(LSM6_ADDR_VCC);
  if (myWire.endTransmission() == 0) {
    mySerial.print("LSM6DSV16X found.\naddress: 0x");
    mySerial.println(LSM6_ADDR_VCC, 16);
    lsm6_addr = LSM6_ADDR_VCC;
  }

  if (lsm6_addr == 0) {
    mySerial.print("LSM6DSV16X not found.");
    while (1)
      ;
  }

  if (!lsm6_init()) {
    mySerial.println("LSM6DSV16X init error!");
    while (1)
      ;
  }

  // quick_test();

  mySerial.println("LSM6DSV16X initialized.");
}


void Normalize() {  // クオータニオンを正規化.
  float n = sqrt(qua.w * qua.w + qua.x * qua.x + qua.y * qua.y + qua.z * qua.z);
  if (n > 0.0f) {
    float inv = 1.0f / n;
    qua.w *= inv;
    qua.x *= inv;
    qua.y *= inv;
    qua.z *= inv;
  }
}

void ToEulerAngles() {
  float sinr_cosp = 2.0f * (qua.w * qua.x + qua.y * qua.z);
  float cosr_cosp = 1.0f - 2.0f * (qua.x * qua.x + qua.y * qua.y);
  angles.roll = atan2(sinr_cosp, cosr_cosp) * 180.0f / PI;

  float sinp = 2.0f * (qua.w * qua.y - qua.z * qua.x);
  sinp = min(max(sinp, -1.0f), 1.0f);
  angles.pitch = asin(sinp) * 180.0f / PI;

  float siny_cosp = 2.0f * (qua.w * qua.z + qua.x * qua.y);
  float cosy_cosp = 1.0f - 2.0f * (qua.y * qua.y + qua.z * qua.z);
  angles.yaw = atan2(siny_cosp, cosy_cosp) * 180.0f / PI;
}


// --- half(16-bit) → float 変換 ---
static inline uint32_t halfbits_to_floatbits(uint16_t h) {
  uint16_t h_exp = h & 0x7C00u;
  uint32_t f_sgn = ((uint32_t)h & 0x8000u) << 16;

  if (h_exp == 0) {  // zero or subnormal
    uint16_t h_sig = h & 0x03FFu;
    if (h_sig == 0) return f_sgn;  // signed zero
    // normalize subnormal
    while ((h_sig & 0x0400u) == 0) {
      h_sig <<= 1;
      h_exp++;
    }
    uint32_t f_exp = (uint32_t)(127 - 15 - h_exp) << 23;
    uint32_t f_sig = ((uint32_t)(h_sig & 0x03FFu)) << 13;
    return f_sgn | f_exp | f_sig;
  } else if (h_exp == 0x7C00u) {  // inf or NaN
    return f_sgn | 0x7F800000u | (((uint32_t)(h & 0x03FFu)) << 13);
  } else {  // normalized
    return f_sgn | (((uint32_t)(h & 0x7FFFu) + 0x1C000u) << 13);
  }
}

static inline float half_to_float(uint16_t h) {
  union {
    uint32_t ui;
    float f;
  } u;
  u.ui = halfbits_to_floatbits(h);
  return u.f;
}


void refresh_euler_angles() {
  uint8_t status[2];
  readRegs(0x1B, status, 2);
  uint16_t unread_words = status[0] | ((status[1] & 0x03) << 8);
  // Serial.print("status1: ");
  // Serial.println(status[0], BIN);
  // Serial.print("status2: ");
  // Serial.println(status[1], BIN);
  // Serial.print("unread: ");
  // Serial.println(unread_words, DEC);

  if (unread_words == 0) return;

  while (unread_words > 1) {
    uint8_t dump[7];
    readRegs(REG_FIFO_DATA_OUT_TAG, dump, 7);
    unread_words--;
  }

  uint8_t data[7];
  readRegs(REG_FIFO_DATA_OUT_TAG, data, 7);
  uint8_t tag = data[0] >> 3;

  if (tag == TAG_FIFO_QUATERNION) {
    uint16_t hx = (uint16_t)(data[1] | (data[2] << 8));
    uint16_t hy = (uint16_t)(data[3] | (data[4] << 8));
    uint16_t hz = (uint16_t)(data[5] | (data[6] << 8));

    float qx = half_to_float(hx);
    float qy = half_to_float(hy);
    float qz = half_to_float(hz);

    // Serial.printf("RAW bytes: %02X %02X %02X %02X %02X %02X\n",
    //   data[1],data[2],data[3],data[4],data[5],data[6]);
    // Serial.printf("RAW half : %04X %04X %04X\n", hx, hy, hz);
    // Serial.printf("q(xyz)   : %.6f %.6f %.6f\n", qx, qy, qz);

    float s = (qx * qx + qy * qy + qz * qz);
    if (s > 1.0f) {
      // 数値誤差で 1 を超えた場合は正規化.
      float inv = 1.0f / sqrt(s);
      qx *= inv;
      qy *= inv;
      qz *= inv;
      s = 1.0f;
    }
    float qw = sqrt(1.0f - s);

    // センサーからのクオータニオンは左手系.
    // x, yを反転し，右手系に変換.

    qua.w = qw;
    qua.x = qx;
    qua.y = qy;
    qua.z = qz;

    Normalize();
    ToEulerAngles();
  }

  // Serial.print("Qua(w, x, y, z): ");
  // Serial.print(qua.w);
  // Serial.print(", ");
  // Serial.print(qua.x);
  // Serial.print(", ");
  // Serial.print(qua.y);
  // Serial.print(", ");
  // Serial.println(qua.z);

  // Serial.print("Euler: ");
  // Serial.printf("%.5f %.5f %.5f\n", angles.roll, angles.pitch, angles.yaw);
}
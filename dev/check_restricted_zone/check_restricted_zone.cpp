#include "check_restricted_zone.h"
#include <TinyGPS++.h>

// 緯度経度を扱う構造体
struct Point {
  double lat;
  double lon;
};

// =========================================
// 飛行可能区域の頂点設定（反時計回りに定義すること）
// =========================================
// 例)
// const Point boundary[] = {
//   {35.300000, 136.260000}, // B1: プラホ側
//   {35.295000, 136.260000}, // B2: 観覧席側
//   {35.295000, 136.250000}, // B3: 沖側1
//   {35.300000, 136.250000}  // B4: 沖側2
// };

// にいじゅく未来公園内
const Point boundary[] = {
  {35.461565, 139.514682}, // ピン1
  {35.461614, 139.514432}, // ピン2
  {35.461490, 139.514395}, // ピン3
  {35.461308, 139.514494}, // ピン4
  {35.461211, 139.515152}, // ピン5
  {35.461366, 139.515248}  // ピン6
};

const int NUM_VERTICES = sizeof(boundary) / sizeof(boundary[0]); // 頂点の数を計算


// ========================================================
// ゾーン内外判定 (外積による判定)
// 戻り値: true (飛行可能区域内) / false (禁止区域内)
// ========================================================
bool isInsideArea(double lat, double lon) {
  for (int k = 0; k < NUM_VERTICES; k++) {
    
    int k_prev = k; 
    int k_curr = k + 1;
    if (k_curr == NUM_VERTICES) {
      // BnB1ベクトルのために例外処理
      k_curr = 0;
    }

    // 境界線ベクトル: B_{k-1} -> B_k
    double Bk1_Bk_lon = boundary[k_curr].lon - boundary[k_prev].lon;
    double Bk1_Bk_lat = boundary[k_curr].lat - boundary[k_prev].lat;
    
    // 機体ベクトル: B_{k-1} -> P
    double Bk1_P_lon = lon - boundary[k_prev].lon;
    double Bk1_P_lat = lat - boundary[k_prev].lat;

    // 外積 Z = (Bk.x - Bk-1.x)(P.y - Bk-1.y) - (Bk.y - Bk-1.y)(P.x - Bk-1.x)
    double z = (Bk1_Bk_lon * Bk1_P_lat) - (Bk1_Bk_lat * Bk1_P_lon);

    // 1つでも負があれば境界線の右側（禁止区域）にいる
    if (z < 0) {
      return false; 
    }
  }
  return true; // 全て正（または0）なら飛行可能区域内
}


// ==========================================
// 最短到達予想時間(TTC)の計算
// 戻り値: 到達までの秒数 (遠ざかっている場合は-1.0を返す)
// ==========================================
float calc_time_to_reach(double lat, double lon, float groundspeed, float heading) {
  float min_t = 999999.0; // 比較用の初期値

  for (int k = 0; k < NUM_VERTICES; k++) {
    
    int k_prev = k;
    int k_curr = k + 1;
    if (k_curr == NUM_VERTICES) {
      k_curr = 0;
    }

    // θ_k: 境界線（B_{k-1} -> B_k）の真北となす方位角
    double theta_k = TinyGPSPlus::courseTo(
      boundary[k_prev].lat, boundary[k_prev].lon, 
      boundary[k_curr].lat, boundary[k_curr].lon
    );

    // 分母: 接近速度 = v * sin(α - θ_k)
    float approachVelocity = groundspeed * sin(radians(heading - theta_k));

    // 接近速度が0以下（平行に飛んでいる、または遠ざかっている）場合は無視
    if (approachVelocity <= 0.05) { 
      continue; 
    }

    // L_{B_k P}の計算 (終点B_kから機体Pまでの距離)
    double L_Bk_P = TinyGPSPlus::distanceBetween(
      boundary[k_curr].lat, boundary[k_curr].lon, 
      lat, lon
    );

    // φ_k (∠ B_{k-1} B_k P) の計算
    // B_kからB_{k-1}を見た角度
    double angle_Bk_Bk1 = TinyGPSPlus::courseTo(
      boundary[k_curr].lat, boundary[k_curr].lon,
      boundary[k_prev].lat, boundary[k_prev].lon
    );
    // 終点B_kから機体Pを見た角度
    double angle_Bk_P = TinyGPSPlus::courseTo(
      boundary[k_curr].lat, boundary[k_curr].lon,
      lat, lon
    );
    
    // φ_kの計算
    double phi_k = radians(angle_Bk_Bk1 - angle_Bk_P);

    // 分子全体の計算．垂直距離= L_{B_k P} * |sin(φ_k)|
    float shortestDistance = L_Bk_P * abs(sin(phi_k));

    // 到達予想時間 t = 最短距離 / 接近速度
    float t = shortestDistance / approachVelocity;

    // 一番早く到達する境界線の時間を記録
    if (t > 0 && t < min_t) {
      min_t = t;
    }
  }

  // どの境界線にも向かっていない（完全に安全な方向に飛んでいる）場合
  if (min_t == 999999.0) {
    return -1.0; 
  }

  return min_t;
}
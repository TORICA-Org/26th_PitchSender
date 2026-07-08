# 飛行禁止区域接近警報
## 機能の構成
禁止区域侵入判定と禁止区域到着予想計算で構成されている．

## アルゴリズムとソースコードの解説

### 禁止区域侵入判定（関数名：`isInsideArea()`）
#### 1. 外積での判定
まずは複数の頂点$`\mathrm{B_1}`$～$`\mathrm{B_n}`$ ($`n=3,4,5...`$)を設定し，飛行区域を構成する．また機体の位置を点$`\mathrm{P}`$とする．


<img src="https://raw.githubusercontent.com/TORICA-Org/26th_PitchSender/refs/heads/dev/dev/check_restricted_zone/figures/fig1.svg"/>


<img src="https://raw.githubusercontent.com/TORICA-Org/26th_PitchSender/refs/heads/dev/dev/check_restricted_zone/figures/fig2.svg"/>

上図において，任意の境界線ベクトル$`\overrightarrow{\mathrm{B_k B_{k+1}}}`$について次式が成立する．

```math
\begin{align}
\overrightarrow{\mathrm{B_{k-1} B_{k}}} \times \overrightarrow{\mathrm{B_{k-1}P}}  & = \left| \overrightarrow{\mathrm{B_{k-1} B_{k}}} \right|  \left| \overrightarrow{\mathrm{B_{k-1}P}} \right| \sin \theta \\
& = \begin{vmatrix} \mathrm{ (B_{k-1} B_k)_{lon} } &  \mathrm{ (B_{k-1} B_k)_{lat} } \\ \mathrm{(B_{k-1} P)_{lon}} & \mathrm{(B_{k-1} P)_{lat} } \end{vmatrix} \\
& = \mathrm{ (B_{k-1} B_k)_{lon} } \cdot \mathrm{(B_{k-1} P)_{lat} } - \mathrm{ (B_{k-1} B_k)_{lat} } \cdot \mathrm{(B_{k-1} P)_{lon}}
\end{align}
```
（ただし$`k=1`$のとき$`\mathrm{B_{k-1}}`$は$`\mathrm{B_n}`$とする．）


プログラム上の実装：
```cpp
// 境界線ベクトル: B_{k-1} -> B_k
    double Bk1_Bk_lon = boundary[k_curr].lon - boundary[k_prev].lon;
    double Bk1_Bk_lat = boundary[k_curr].lat - boundary[k_prev].lat;
    
    // 機体ベクトル: B_{k-1} -> P
    double Bk1_P_lon = lon - boundary[k_prev].lon;
    double Bk1_P_lat = lat - boundary[k_prev].lat;

    // 外積 Z = (Bk.x - Bk-1.x)(P.y - Bk-1.y) - (Bk.y - Bk-1.y)(P.x - Bk-1.x)
    double z = (Bk1_Bk_lon * Bk1_P_lat) - (Bk1_Bk_lat * Bk1_P_lon);
```
注）
- $`k-1`$を`k_prev`（または$`\mathrm{B_{k-1}}`$=`boundary[k_prev]`），$`k`$=`k_curr`（または$`\mathrm{B_k}`$=`boundary[k_curr]`）と読み替える．
- $`k`$については以下の対応表を参照．
プログラムの配列インデックスは 0 から始まるため，説明書きにおける数式（頂点 $`\mathrm{B_k}`$）とプログラムのループ変数（`k_prev`, `k_curr`）には以下のような対応関係がある．（頂点が4つ：$`B_1 \dots B_4`$ の場合の例）

| この説明書きにおける $k$ | この説明書きにおけるベクトル | プログラム中の`k_prev` (始点) | プログラム中の `k_curr` (終点) | プログラムが計算するベクトル |
| :--- | :--- | :--- | :--- | :--- |
| **$k = 2$** | $\overrightarrow{\mathrm{B_1 B_2}}$ | `0` | `1` | `boundary[0] -> boundary[1]` |
| **$k = 3$** | $\overrightarrow{\mathrm{B_2 B_3}}$ | `1` | `2` | `boundary[1] -> boundary[2]` |
| **$k = 4$** | $\overrightarrow{\mathrm{B_3 B_4}}$ | `2` | `3` | `boundary[2] -> boundary[3]` |
| **$k = 1$** (例外) | $\overrightarrow{\mathrm{B_4 B_1}}$ | `3` | `0` | `boundary[3] -> boundary[0]` |

#### 2. for文によるループ処理



反時計回りのベクトルで飛行可能区域を囲んでいるので，境界線ベクトルの左側（飛行可能区域）にいるとき$`\theta>0`$．言い換えると飛行可能区域内にいるということは$`\sin \theta>0`$である．

そのため以下の外積計算を行い，その正負によって領域を判定すればよい．

```math
\begin{gather}
\overrightarrow{\mathrm{B_1 B_2}} \times \overrightarrow{\mathrm{B_1 P}} \\
\overrightarrow{\mathrm{B_2 B_3}} \times \overrightarrow{\mathrm{B_2 P}} \\
\vdots\\
\overrightarrow{\mathrm{B_{n-1} B_n}} \times \overrightarrow{\mathrm{B_{n-1} P}} \\
\overrightarrow{\mathrm{B_n B_1}} \times \overrightarrow{\mathrm{B_n P}}
\end{gather}
```

```math
\left\{
\begin{align}
& > 0 \text{（飛行可能区域内）}\\
& = 0 \text{（境界線上）} \\
& < 0 \text{（禁止区域内）}
\end{align}
\right.
```

プログラム上の実装：
```cpp
bool isInsideArea(double lat, double lon) {
  for (int k = 0; k < NUM_VERTICES; k++) { // 頂点の数だけループを実行
    
    int k_prev = k; 
    int k_curr = k + 1;
    if (k_curr == NUM_VERTICES) {
      k_curr = 0; // BnB1ベクトルのために例外処理
    }

（中略）外積計算

    // 1つでも負があれば境界線の右側（禁止区域）にいる
    if (z < 0) {
      return false; 
    }
  }
  return true; // 全て正（または0）なら飛行可能区域内
}
```



### 禁止区域到着予想計算
現在地から最も近い境界線までの到達予想時間を計算する．

| GPSデータ等マイコンから取得する値  | 図中のパラメータ |
| ------------- | ------------- |
| `data_air_gps_groundspeed_ms`  | $`v`$  |
| `data_air_gps_heading_deg`  | $`\alpha`$  |

他に定数として境界線用に設定した緯度経度を用いる．



ある境界線（点$`\mathrm{B_{k-1}}`$から点$`\mathrm{B_k}`$に向かう線分）について考える．機体と境界線の位置関係を以下の図に示した．
<img src="https://raw.githubusercontent.com/TORICA-Org/26th_PitchSender/refs/heads/dev/dev/check_restricted_zone/figures/fig3.svg"/>

機体から境界線までの最短距離は，機体から境界線を延長した直線へ下ろした垂線の長さに等しい．これとGPSから受信した対地速度と方位を組み合わせることで接近速度成分を得ることができ，次式で到達予想時間(time to reach: $`t`$)が求められる。

```math
t = \dfrac{\mathrm{L_{B_k P}} \cdot \sin \phi_k}{v \sin(\alpha - \theta_k)}
```




# 胴体桁基板(Pitchsender)用プログラム
## 胴体桁基板(Pitchsender)が担う役割
- テール保持者にBluetoothイヤホン経由でPitchを伝える．
- BNO055で姿勢角を取得する．
  - oll, Pitch, Yawを計算
- BMP390で気温・気圧を取得．
  - 気圧高度を計算
- MicroSDにデータを記録．
- スピーカーを通してPに速度や高度，飛行禁止区域への接近を伝達．

## Pitchsenderの構成
- ESP32-WROOM-32E
- 9軸センサーモジュール BNO055
- 6軸センサーモジュール AE-LSM6DSV16X [https://akizukidenshi.com/catalog/g/g130950/](https://akizukidenshi.com/catalog/g/g130950/)
- 気圧気温センサー BMP390 
- アンプ・スピーカー [https://akizukidenshi.com/catalog/g/g108217/](https://akizukidenshi.com/catalog/g/g108217/)
- MicroSDスロット

## プログラムの構成
とにかく6軸IMUを高頻度で更新しないといけない．実行する関数に優先度を割り当てる．

### 関数の優先度
- センサー読み取り
read_bmp_fslg()
read_bno()
read_lsm()
- UART送受信
- GPSで進入禁止区域判定
- SDカード書き込み
- BluetoothでPitch送信
- ステータスLED制御




### 飛行禁止区域接近警報について
Under construction
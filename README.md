# 26th_PitchSender
テール保持者にBluetoothイヤホンを介してピッチを伝えるための装置

# 構成
- [\[115673\]ESP32-DevKitC-32E ESP32-WROOM-32E開発ボード 4MB](https://akizukidenshi.com/catalog/g/g115673/)
- [\[130950\]6軸IMUセンサーモジュール](https://akizukidenshi.com/catalog/g/g130950/)

これら2つを搭載した基板を桁に直接巻き付け，メインの電装から電源供給のみ受ける．

# 動作
1. LSM6DSV16Xの高度なセンサーフュージョン機能（Sensor Fusion Low-Power : SFLP）を使用して，クオータニオンを取得．
2. クオータニオンをオイラー角に変換し，グローバルに宣言されたオイラー角用の構造体（angles）を更新
3. 取得したピッチ（angles.pitch）に応じて，BTイヤホンに送信する音の周波数（音程）と間隔を変更．

# ソフトウェアの詳細
- センサーフュージョンに関する公式ドキュメント -> https://community.st.com/t5/mems-and-sensors/how-lsm6dsv16x-enables-sensor-fusion-low-power-sflp-algorithm/ta-p/585084
- センサーフュージョンのサンプルコード -> https://github.com/STMicroelectronics/STMems_Standard_C_drivers/blob/master/lsm6dsv16x_STdC%2Fexamples%2Flsm6dsv16x_sensor_fusion.c
- LSM6DSV16XのSFLPにより計算されたクオータニオンはFIFOに保存される．
  - FIFOに保存されるデータは`xyz`のみで`w`は失われるため，ソフトウェア側で復元する必要がある．(-> )
  - 出力されるクオータニオンは半精度浮動小数点数（Half precision floating-point）なので`float`（単精度浮動小数点数）として使うには変換処理が必要．(-> https://github.com/STMicroelectronics/lsm6dsv16x-pid/blob/main/lsm6dsv16x_reg.c#L200-L244)
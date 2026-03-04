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

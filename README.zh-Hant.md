# Muse Gadget 繁體中文擴充

這個社群分支以 Muse ESP32 Device SDK 為基礎，加入繁體中文介面、CJK 字型和廣東話／普通話回覆選項。Fish Audio 直接為兩種語言合成語音。這是連結到韌體的原始碼擴充，不是可在裝置執行期間載入的外掛；每種板型都要重新編譯並刷寫對應韌體。

## 使用流程

1. **為自己的板型編譯韌體。** 目前支援 Waveshare ESP32-S3-Touch-AMOLED-1.75C、Waveshare 1.75 和 M5Stack CoreS3。三種韌體映像不能互換。每位使用者都要在本機建置設定輸入自己的 Muse SDK token；本專案不包含作者的 token，也不要把本機 `sdkconfig` 或建置輸出提交到 Git。
2. **刷寫並配對 Muse。** 在 Muse App 開啟「Settings > Devices > Developer mode」，然後按「Settings > Devices > Add Device」右上角的「+」，依指示配對及完成裝置確認。手機設定頁的「Device Token」是另一種可選憑證；一般使用 App 配對時留空即可。
3. **先設定 Wi-Fi。** 在裝置開啟「設定 > 藍牙 > 手機設定」，用支援 Web Bluetooth 的 Chrome 開啟 [`esp32/tools/muse/ble_setup.html`](esp32/tools/muse/ble_setup.html)，連線並輸入 Muse 螢幕上的配對碼，再連接 Wi-Fi。
4. **設定 Fish Audio。** 在同一手機設定頁輸入自己的 Fish Audio API 金鑰，以及廣東話和普通話各自使用的 voice ID。voice ID 是 Fish Audio 語音模型的識別碼；可在 [Fish Audio](https://fish.audio/) 管理 API 金鑰和語音模型。設定頁不會把 API 金鑰存進瀏覽器；金鑰經已驗證的 BLE 傳送後會清空，並保存到裝置的加密 NVS。
5. **選擇語音並開始對話。** 在裝置「設定 > 語言與語音」選擇「廣東話」或「普通話」。螢幕字幕使用繁體中文，語音使用對應的 Fish Audio voice ID。裝置直接以 HTTPS 呼叫 [Fish Audio TTS API](https://beta.fish.audio/text-to-speech-api/)，語音由 Fish Audio 雲端合成，無需本機語音伺服器或常駐電腦。未設定 API 金鑰或所選語言的 voice ID 時，裝置仍會顯示字幕，但不會播放語音。

## 憑證與裝置安全

- Muse SDK token 和 Fish Audio API 金鑰是不同憑證。SDK token 由每位使用者在本機建置時設定；Fish Audio 金鑰由每位使用者在手機設定頁輸入。任何一種都不要提交到 Git、放進公開韌體或貼到聊天訊息。
- Fish Audio API 金鑰只會在啟用加密 NVS 的板型韌體上保存。它經過配對驗證的 BLE 傳送，呼叫語音服務時放在 HTTPS Bearer 標頭；韌體不會回傳或記錄金鑰。Fish Audio 會收到待朗讀的回覆文字和 voice ID。
- 升級到此版本後，韌體首次啟動會從 NVS 移除舊版的 TTS 設定和憑證。Fish Audio 金鑰不會沿用舊版憑證。
- NVS 加密保護快閃記憶體內的憑證，但裝置執行時會在記憶體中使用金鑰；能控制裝置或執行修改韌體的人仍可能取出它。請使用專用的 Fish Audio API 金鑰，設定用量或帳單提示，懷疑外洩時撤銷並換新。
- 這三種板型都要求加密 NVS，並要求 eFuse HMAC 區塊 0 已預先設為 `HMAC_UP`。韌體會檢查用途；若區塊空白或用途不符，啟動會停止，不會自動產生或燒錄 eFuse 金鑰。eFuse 寫入不可逆。首次使用另一塊板前，先用 `espefuse.py -p PORT summary` 檢查：

  ```sh
  openssl rand -out nvs_hmac_key.bin 32
  idf.py -p PORT efuse-burn-key BLOCK_KEY0 nvs_hmac_key.bin HMAC_UP
  ```

  每部裝置請使用不同金鑰並妥善保存；不要提交金鑰檔。刷寫後要維持相同的 NVS 加密設定，否則裝置可能無法讀取原有設定。

## 建置

韌體使用 ESP-IDF 6.0.1。查看 [`esp32/AGENTS.md`](esp32/AGENTS.md) 的依賴和建置說明。板型設定分別使用 `s3`、`s3n` 和 `cores3`。例如建置 Waveshare 1.75C：

```sh
cd esp32
tools/muse/board.sh build s3
```

請在該建置目錄的本機 `sdkconfig` 或 `menuconfig` 設定自己的 `CONFIG_GADGET_SDK_TOKEN`。不要提交含有 token 的設定檔或韌體映像。

## 此版本狀態

Fish Audio 韌體修改尚未建置、刷寫或在裝置上驗證。需要在 Waveshare 1.75C 上實際設定 Fish Audio API 金鑰和兩個 voice ID，再確認廣東話及普通話語音播放、字幕顯示和回覆語言切換。

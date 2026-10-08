/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "muse_lang.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *english;
    const char *traditional_chinese;
} entry_t;

static const entry_t STRINGS[] = {
    { "WAKING UP", "正在啟動" },
    { "READY", "就緒" },
    { "LISTENING", "聆聽中" },
    { "THINKING", "思考中" },
    { "SPEAKING", "回覆中" },
    { "ERROR", "錯誤" },
    { "GOODBYE", "再見" },
    { "TAP TO TAKE PHOTO", "點按拍照" },
    { "Pairing code", "配對碼" },
    { "Enter on phone", "在手機輸入" },
    { "Enter it on your phone", "在手機輸入此配對碼" },
    { "SETTINGS", "設定" },
    { "Settings", "設定" },
    { "Wi-Fi", "Wi-Fi" },
    { "Muse", "Muse" },
    { "Bluetooth", "藍牙" },
    { "Sound", "音效" },
    { "Sleep", "休眠" },
    { "Battery", "電池" },
    { "Power off", "關機" },
    { "Volume", "音量" },
    { "Speaker", "揚聲器" },
    { "Brightness", "螢幕亮度" },
    { "Mic gain", "麥克風增益" },
    { "Auto-sleep", "自動休眠" },
    { "Phone setup", "手機設定" },
    { "Status", "狀態" },
    { "Reset pairing", "重設配對" },
    { "Screen off", "關閉螢幕" },
    { "Close menu", "關閉選單" },
    { "Change", "調整" },
    { "Toggle", "切換" },
    { "Open", "開啟" },
    { "Select", "選擇" },
    { "Close", "關閉" },
    { "On", "開啟" },
    { "Off", "關閉" },
    { "Never", "永不" },
    { "30 seconds", "30 秒" },
    { "1 minute", "1 分鐘" },
    { "2 minutes", "2 分鐘" },
    { "5 minutes", "5 分鐘" },
    { "10 minutes", "10 分鐘" },
    { "WIFI", "Wi-Fi" },
    { "MUSE", "Muse 帳戶" },
    { "BLUETOOTH", "藍牙" },
    { "SOUND", "音效" },
    { "SLEEP", "休眠" },
    { "BATTERY", "電池" },
    { "POWER", "電源" },
    { "Network name", "網絡名稱" },
    { "Password", "密碼" },
    { "Other network...", "其他網絡…" },
    { "Other network", "其他網絡" },
    { "Saved networks", "已儲存的網絡" },
    { "Tap to forget", "再次點按以忘記" },
    { "Hidden", "隱藏" },
    { "No networks found", "找不到 Wi-Fi 網絡" },
    { "MAC address", "MAC 地址" },
    { "Muse remembers up to 8 networks and joins the strongest one in range. Tap a saved one twice to forget it.",
      "Muse 最多會記住 8 個網絡，並連接到訊號最強的網絡。連點已儲存的網絡兩次即可忘記。" },
    { "Wi-Fi is off", "Wi-Fi 已關閉" },
    { "Empty if it's open", "開放網絡請留空" },
    { "Empty for the default", "留空則使用預設值" },
    { "Empty keeps the current one", "留空以保留目前設定" },
    { "Optional", "可選" },
    { "Tap again to reset", "再點按一次以重設" },
    { "Visible as %s", "顯示為 %s" },
    { "Phone connected\n%s", "手機已連線\n%s" },
    { "Waiting for pairing", "等待配對" },
    { "Muse app", "Muse App" },
    { "Pair with Muse app", "與 Muse App 配對" },
    { "Press", "按下" },
    { "Press the %s button", "按下 %s 按鈕" },
    { "%s button", "%s 按鈕" },
    { "Resetting...", "正在重設…" },
    { "Wakes", "喚醒次數" },
    { "Turn the screen off after Muse has been idle for:", "Muse 閒置多久後關閉螢幕：" },
    { "Tap the screen or press either button to wake.", "點按螢幕或按下任一按鈕即可喚醒。" },
    { "Used", "已使用" },
    { "A full charge", "充滿電可用時間" },
    { "Chip asleep", "晶片休眠" },
    { "CPU busy", "CPU 運作" },
    { "HOLD TO MUTE", "長按以靜音" },
    { "HOLD TO UNMUTE", "長按以取消靜音" },
    { "SPEAKER OFF", "揚聲器已關閉" },
    { "SPEAKER ON", "揚聲器已開啟" },
    { "Tap screen", "點按螢幕" },
    { "Tap to confirm pairing", "點按以確認配對" },
    { "Press button", "按下按鈕" },
    { "SET UP MUSE FIRST", "請先設定 Muse" },
    { "MENU", "選單" },
    { "POWER OFF", "關機" },
    { "RESET PAIRING", "重設配對" },
    { "Custom", "自訂" },
    { "30 s", "30 秒" },
    { "1 min", "1 分鐘" },
    { "2 min", "2 分鐘" },
    { "5 min", "5 分鐘" },
    { "10 min", "10 分鐘" },
    { "On batt", "使用電池" },
    { "Last run", "上次記錄" },
    { "Esc Back", "Esc 返回" },
    { "Esc Cancel", "Esc 取消" },
    { "Back", "返回" },
    { "Reset", "重設" },
    { "Enter %s", "按 Enter 鍵%s" },
    { "RESETTING...", "正在重設…" },
    { "Muse app: %s\n%s", "Muse App：%s\n%s" },
    { "Set (%u chars)", "已設定（%u 個字元）" },
    { "On battery for %s", "使用電池：%s" },
    { "Last run: %s on battery", "上次使用電池：%s" },
    { "None", "無" },
    { "lasts ~%d h", "約可使用 %d 小時" },
    { "%d%% so far", "目前已使用 %d%%" },
    { "saved  %d dBm", "已儲存  %d dBm" },
    { "open  %d dBm", "開放  %d dBm" },
    { "%s %s\nBatt  %d>%d%%\nRate  %s\nFull  %s\nOff   %s\nSleep %s\nWakes %s\nBusy  %s",
      "%s %s\n電量  %d>%d%%\n耗電  %s\n充滿  %s\n螢幕  %s\n休眠  %s\n喚醒  %s\n運作  %s" },
    { "MENU  ^v Move  <> Change", "選單  ^v 移動  <> 調整" },
    { "STATUS", "狀態" },
    { "Down", "向下" },
    { "Starting", "啟動中" },
    { "Ready to pair", "準備配對" },
    { "App connected", "App 已連線" },
    { "Confirm pairing", "確認配對" },
    { "Online", "已上線" },
    { "Error", "錯誤" },
    { "Through Home Link, text replies", "透過 Home Link 傳送文字回覆" },
    { "Connecting...", "正在連線…" },
    { "Unplug USB to\nmeasure how\nlong the\nbattery lasts.", "拔除 USB 後開始\n測量電池可用時間。" },
    { "Measures from unplugging USB until it's plugged back in. The gauge moves in 1% steps, so give it a few hours. Chip asleep is time in light sleep; CPU busy is time a core was running a task.",
      "從拔除 USB 開始測量，直到重新接上 USB。電量計每次變化 1%，請至少測量數小時。「晶片休眠」是輕度睡眠時間；「CPU 運作」是處理器執行工作的時間。" },
    { "Talk at arm's length: the bar should reach green (-30 to -15 dBFS) without going orange.",
      "請在約一臂距離說話：音量條應進入綠色區（-30 至 -15 dBFS），但不要進入橙色區。" },
    { "~%d h", "約 %d 小時" },
    { "%s %.1fs", "%s %.1f 秒" },
    { "LISTENING...", "聆聽中…" },
    { "RECORDING...", "錄音中…" },
    { "SENDING VOICE NOTE", "正在傳送語音訊息" },
    { "NOTE SENT - WAITING FOR MUSE", "訊息已送出，等待 Muse 回覆" },
    { "SENDING SAVED NOTE", "正在傳送已儲存訊息" },
    { "SAVED NOTE SENT", "已送出儲存訊息" },
    { "AUDIO INIT FAILED", "音訊初始化失敗" },
    { "Wi-Fi %s\nIP    %s\nLink  %s\nMuse  %s\nPhone %s\nPower %s\nVer   %s",
      "Wi-Fi %s\nIP    %s\n連線  %s\nMuse  %s\n手機  %s\n電源  %s\n版本  %s" },
    { "Turn Muse off?\n\nPress the %s button to turn it back on.", "要關閉 Muse 嗎？\n\n按下 %s 按鈕重新開機。" },
    { "Forget Wi-Fi and the Muse app pairing, then restart?", "要清除 Wi-Fi 和 Muse App 配對並重新啟動嗎？" },
    { "Paired", "已配對" },
    { "paired", "已配對" },
    { "not paired", "尚未配對" },
    { "No saved networks. Scan and pick one.", "沒有已儲存的網絡。請掃描並選擇網絡。" },
    { "Joining %s\n%s", "正在連線至 %s\n%s" },
    { "Connected to %s\n%s  -  %d dBm", "已連線至 %s\n%s  -  %d dBm" },
    { "No saved network nearby\nLooking again within a minute", "附近找不到已儲存網絡\n將於一分鐘內再次搜尋" },
    { "Couldn't join %s\n%s", "無法連線至 %s\n%s" },
    { "Show", "顯示" },
    { "Hide", "隱藏" },
    { "Join", "連線" },
    { "Forget all networks", "忘記所有網絡" },
    { "Scanning...", "正在掃描…" },
    { "Scan for networks", "掃描 Wi-Fi 網絡" },
    { "Not set", "尚未設定" },
    { "Joining", "連線中" },
    { "Failed", "失敗" },
    { "Not found", "找不到網絡" },
    { "Not nearby", "附近找不到網絡" },
    { "Server", "伺服器" },
    { "VM ID", "VM ID" },
    { "Device token", "裝置 Token" },
    { "Test connection", "測試連線" },
    { "Pair with the Muse app to use your account; a device token here overrides it, and a long one is easier to send over Bluetooth. The VM ID picks one of your VMs. Reset pairing forgets Wi-Fi and the app pairing, then restarts.",
      "使用 Muse App 配對即可連結帳戶；在此輸入的裝置 Token 會取代 App 配對。較長的 Token 可透過藍牙傳送。VM ID 用來選擇虛擬機。重設配對會清除 Wi-Fi 與 App 配對，然後重新啟動。" },
    { "Forget paired phones", "忘記已配對手機" },
    { "When on, Muse is visible to phones nearby. Open tools/ble_setup.html in Chrome, connect, and enter the code Muse shows to pair.",
      "開啟後，附近手機可找到 Muse。請用 Chrome 開啟 tools/ble_setup.html 並連線，再輸入 Muse 螢幕上的配對碼。" },
    { "Mic level", "麥克風音量" },
    { "Muted", "已靜音" },
    { "Language & speech", "語言與語音" },
    { "LANGUAGE & SPEECH", "語言與語音" },
    { "Language", "語言" },
    { "Cantonese", "廣東話" },
    { "Mandarin", "普通話" },
    { "Fish Audio supports Cantonese and Mandarin. Traditional Chinese captions stay on screen.",
      "Fish Audio 支援廣東話和普通話；螢幕字幕會維持繁體中文。" },
    { "Set your Fish Audio API key and voice IDs in menuconfig to enable speech.",
      "請在 menuconfig 設定 Fish Audio API 金鑰和語音 ID，以啟用語音回覆。" },
    { "Fish Audio: speech ready", "Fish Audio：語音已就緒" },
    { "Fish Audio: captions only", "Fish Audio：只顯示字幕" },
    { "Reply voice", "回覆語音" },
    { "Cantonese voice", "廣東話語音" },
    { "Mandarin voice", "普通話語音" },
    { "Selected", "已選擇" },
    { "Traditional Chinese captions stay on screen. Choose the language Muse speaks in.",
      "螢幕字幕會維持繁體中文。請選擇 Muse 的回覆語音。" },
    { "Connected", "已連線" },
    { "Not paired", "尚未配對" },
    { "Not set up", "尚未設定" },
    { "Offline", "離線" },
    { "Saved", "已儲存" },
    { "Connecting", "連線中" },
    { "Can't connect", "無法連線" },
    { "Pair in the Muse app", "請在 Muse App 配對" },
    { "Waiting for Wi-Fi", "等待 Wi-Fi" },
    { "Connects when you talk", "開始對話時連線" },
    { "Cancel", "取消" },
    { "Power Muse off completely?", "要完全關閉 Muse 嗎？" },
    { "Press the %s button to turn it back on. To just turn the screen off, press the %s button.",
      "按下 %s 按鈕重新開機。若只想關閉螢幕，請按下 %s 按鈕。" },
    { "No battery", "沒有電池" },
    { "Unplug USB to start measuring.", "拔除 USB 後開始測量。" },
    { "WI-FI OFF", "Wi-Fi 已關閉" },
    { "SET UP WI-FI", "請設定 Wi-Fi" },
    { "NO WI-FI", "找不到 Wi-Fi" },
    { "RECONNECTING", "重新連線中" },
    { "CONNECTING", "連線中" },
    { "CHARGING", "充電中" },
    { "Battery level", "電量" },
    { "CHARGING %d%%", "充電中 %d%%" },
    { "BATTERY %d%%", "電量 %d%%" },
    { "WAKING UP...", "正在啟動…" },
    { "CAN'T REACH MUSE", "無法連接 Muse" },
    { "MUSE NOT SET UP", "尚未設定 Muse" },
    { "DIDN'T CATCH THAT", "沒有聽清楚" },
    { "MUSE STOPPED LISTENING", "Muse 停止聆聽" },
    { "MUSE COULDN'T LISTEN", "Muse 無法聆聽" },
    { "SAY HEY MUSE", "請說 Hey Muse" },
    { "PRESS TALK", "請按下通話按鈕" },
    { "measuring", "測量中" },
    { "Also kept awake by: %s", "仍在運作：%s" },
    { "Enter", "確認" },
    { "Volume %d%%", "音量 %d%%" },
    { "Vol %d%%", "音量 %d%%" },
    { "USB POWER", "USB 供電" },
    { "USB", "USB" },
};

const char *muse_lang_get(const char *english)
{
    if (!english) {
        return "";
    }
    for (size_t i = 0; i < sizeof(STRINGS) / sizeof(STRINGS[0]); i++) {
        if (strcmp(english, STRINGS[i].english) == 0) {
            return STRINGS[i].traditional_chinese;
        }
    }
    return english;
}

int muse_lang_snprintf(char *out, size_t out_size, const char *english_format, ...)
{
    if (!out || !out_size || !english_format) {
        return -1;
    }
    va_list args;
    va_start(args, english_format);
    int result = vsnprintf(out, out_size, muse_lang_get(english_format), args);
    va_end(args);
    return result;
}

const char *muse_lang_chat_instruction(muse_reply_language_t language)
{
    return language == MUSE_REPLY_CANTONESE
        ? "Reply in Traditional Chinese using natural Hong Kong written Cantonese. Keep names and technical terms clear."
        : "Reply in Traditional Chinese using natural Mandarin wording. Keep names and technical terms clear.";
}

int muse_lang_build_chat_prompt(muse_reply_language_t language, const char *text, char *out, size_t out_size)
{
    if (!text || !out || !out_size) {
        return -1;
    }
    return snprintf(out, out_size, "%s\n\n%s", muse_lang_chat_instruction(language), text);
}

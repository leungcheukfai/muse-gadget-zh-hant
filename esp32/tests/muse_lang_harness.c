#include <assert.h>
#include <string.h>

#include "muse_lang.h"

int main(void)
{
    char prompt[256];
    char wifi_status[96];
    int wifi_status_len = muse_lang_snprintf(wifi_status, sizeof(wifi_status),
                                             "Joining %s\n%s", "Home Wi-Fi", "Connecting");
    assert(wifi_status_len == (int)strlen(wifi_status));
    assert(strcmp(wifi_status, "正在連線至 Home Wi-Fi\nConnecting") == 0);
    int prompt_len = muse_lang_build_chat_prompt(MUSE_REPLY_CANTONESE, "你好", prompt, sizeof(prompt));
    assert(prompt_len == (int)strlen(prompt));
    assert(strncmp(prompt, muse_lang_chat_instruction(MUSE_REPLY_CANTONESE),
                   strlen(muse_lang_chat_instruction(MUSE_REPLY_CANTONESE))) == 0);
    assert(strstr(prompt, "\n\n你好") != NULL);
    char short_prompt[4];
    assert(muse_lang_build_chat_prompt(MUSE_REPLY_MANDARIN, "你好", short_prompt, sizeof(short_prompt)) >=
           (int)sizeof(short_prompt));
    assert(strcmp(muse_lang_get("READY"), "就緒") == 0);
    assert(strcmp(muse_lang_get("Language"), "語言") == 0);
    assert(strcmp(muse_lang_get("Cantonese"), "廣東話") == 0);
    assert(strcmp(muse_lang_get("Mandarin"), "普通話") == 0);
    assert(strcmp(muse_lang_get("Fish Audio: API key saved"), "Fish Audio：API 金鑰已儲存") == 0);
    assert(strcmp(muse_lang_get("Fish Audio: API key not set"), "Fish Audio：尚未設定 API 金鑰") == 0);
    assert(strcmp(muse_lang_get("Fish Audio voice ID (Cantonese)"), "Fish Audio 語音 ID（廣東話）") == 0);
    assert(strcmp(muse_lang_get("Show"), "顯示") == 0);
    assert(strcmp(muse_lang_get("Hide"), "隱藏") == 0);
    assert(strcmp(muse_lang_get("No networks found"), "找不到 Wi-Fi 網絡") == 0);
    assert(strcmp(muse_lang_get("MAC address"), "MAC 地址") == 0);
    assert(strcmp(muse_lang_get("Saved networks"), "已儲存的網絡") == 0);
    assert(strcmp(muse_lang_get("Other network..."), "其他網絡…") == 0);
    assert(strcmp(muse_lang_get("Wi-Fi is off"), "Wi-Fi 已關閉") == 0);
    assert(strcmp(muse_lang_get("Tap to forget"), "再次點按以忘記") == 0);
    assert(strcmp(muse_lang_get("Hidden"), "隱藏") == 0);
    assert(strcmp(muse_lang_get("Other network"), "其他網絡") == 0);
    assert(strcmp(muse_lang_get("Empty if it's open"), "開放網絡請留空") == 0);
    assert(strcmp(muse_lang_get("Empty for the default"), "留空則使用預設值") == 0);
    assert(strcmp(muse_lang_get("Empty keeps the current one"), "留空以保留目前設定") == 0);
    assert(strcmp(muse_lang_get("Optional"), "可選") == 0);
    assert(strcmp(muse_lang_get("Tap again to reset"), "再點按一次以重設") == 0);
    assert(strcmp(muse_lang_get("Visible as %s"), "顯示為 %s") == 0);
    assert(strcmp(muse_lang_get("Phone connected\n%s"), "手機已連線\n%s") == 0);
    assert(strcmp(muse_lang_get("Waiting for pairing"), "等待配對") == 0);
    assert(strcmp(muse_lang_get("Muse app"), "Muse App") == 0);
    assert(strcmp(muse_lang_get("Pair with Muse app"), "與 Muse App 配對") == 0);
    assert(strcmp(muse_lang_get("Press"), "按下") == 0);
    assert(strcmp(muse_lang_get("Press the %s button"), "按下 %s 按鈕") == 0);
    assert(strcmp(muse_lang_get("30 s"), "30 秒") == 0);
    assert(strcmp(muse_lang_get("Resetting..."), "正在重設…") == 0);
    assert(strcmp(muse_lang_get("Wakes"), "喚醒次數") == 0);
    assert(strcmp(muse_lang_get("Turn the screen off after Muse has been idle for:"),
                  "Muse 閒置多久後關閉螢幕：") == 0);
    assert(strcmp(muse_lang_get("Tap the screen or press either button to wake."),
                  "點按螢幕或按下任一按鈕即可喚醒。") == 0);
    assert(strcmp(muse_lang_get("Used"), "已使用") == 0);
    assert(strcmp(muse_lang_get("A full charge"), "充滿電可用時間") == 0);
    assert(strcmp(muse_lang_get("Screen off"), "關閉螢幕") == 0);
    assert(strcmp(muse_lang_get("Chip asleep"), "晶片休眠") == 0);
    assert(strcmp(muse_lang_get("CPU busy"), "CPU 運作") == 0);
    assert(strcmp(muse_lang_get("HOLD TO MUTE"), "長按以靜音") == 0);
    assert(strcmp(muse_lang_get("HOLD TO UNMUTE"), "長按以取消靜音") == 0);
    assert(strcmp(muse_lang_get("SPEAKER OFF"), "揚聲器已關閉") == 0);
    assert(strcmp(muse_lang_get("SPEAKER ON"), "揚聲器已開啟") == 0);
    assert(strcmp(muse_lang_get("Tap screen"), "點按螢幕") == 0);
    assert(strcmp(muse_lang_get("Tap to confirm pairing"), "點按以確認配對") == 0);
    assert(strcmp(muse_lang_get("Press button"), "按下按鈕") == 0);
    assert(strcmp(muse_lang_get("SET UP MUSE FIRST"), "請先設定 Muse") == 0);
    assert(strcmp(muse_lang_get("MENU"), "選單") == 0);
    assert(strcmp(muse_lang_get("POWER OFF"), "關機") == 0);
    assert(strcmp(muse_lang_get("RESET PAIRING"), "重設配對") == 0);
    assert(strcmp(muse_lang_get("Custom"), "自訂") == 0);
    assert(strcmp(muse_lang_get("On batt"), "使用電池") == 0);
    assert(strcmp(muse_lang_get("Last run"), "上次記錄") == 0);
    assert(strcmp(muse_lang_get("Esc Back"), "Esc 返回") == 0);
    assert(strcmp(muse_lang_get("Esc Cancel"), "Esc 取消") == 0);
    assert(strcmp(muse_lang_get("Back"), "返回") == 0);
    assert(strcmp(muse_lang_get("Reset"), "重設") == 0);
    assert(strcmp(muse_lang_get("Enter %s"), "按 Enter 鍵%s") == 0);
    assert(strcmp(muse_lang_get("RESETTING..."), "正在重設…") == 0);
    assert(strcmp(muse_lang_get("Measures from unplugging USB until it's plugged back in. The gauge moves in 1% steps, so give it a few hours. Chip asleep is time in light sleep; CPU busy is time a core was running a task."),
                  "從拔除 USB 開始測量，直到重新接上 USB。電量計每次變化 1%，請至少測量數小時。「晶片休眠」是輕度睡眠時間；「CPU 運作」是處理器執行工作的時間。") == 0);
    assert(strcmp(muse_lang_get("Muse remembers up to 8 networks and joins the strongest one in range. Tap a saved one twice to forget it."),
                  "Muse 最多會記住 8 個網絡，並連接到訊號最強的網絡。連點已儲存的網絡兩次即可忘記。") == 0);
    char battery_status[48];
    int battery_status_len = muse_lang_snprintf(battery_status, sizeof(battery_status), "On battery for %s", "2h30m");
    assert(battery_status_len == (int)strlen(battery_status));
    assert(strcmp(battery_status, "使用電池：2h30m") == 0);
    char duration[48];
    muse_lang_snprintf(duration, sizeof(duration), "%s %.1fs", muse_lang_get("LISTENING"), 1.5);
    assert(strcmp(duration, "聆聽中 1.5 秒") == 0);
    assert(strstr(muse_lang_chat_instruction(MUSE_REPLY_CANTONESE), "Hong Kong") != NULL);
    assert(strstr(muse_lang_chat_instruction(MUSE_REPLY_MANDARIN), "Mandarin") != NULL);
    assert(strcmp(muse_lang_get("untranslated"), "untranslated") == 0);
    return 0;
}

/****************************************************************************
 * Project: AeonGrid Smart Charge Controller & Grid Telemetry
 * Author: Mukim Billah Prodhan (https://github.com/MukimBillahProdhan)
 * License: CC BY-NC 4.0
 ****************************************************************************/

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Updater.h> 

#include "dashboard.h"

#define PIN_VOLTAGE_ANALOG A0  
#define PIN_AC_SENSE       5   
#define PIN_RELAY_CHARGER  12  
#define PIN_INBUILT_LED    2   
#define PIN_BUZZER         13  

// --- Limits & Config ---
float vLowCut = 21.0;
float vResume = 25.2;
float vHighCut = 29.0;
float calibrationFactor = 0.03695; 

float currentVoltage = 0.0;
float rawAnalogAvg = 0.0; 
bool isAcOnline = true; 
bool relayState = false;
int chargerMode = 0; 

// --- System Engine Variables ---
unsigned long sessionUptime = 0;           
unsigned long totalUptimeSeconds = 0;      
unsigned long totalDowntimeSeconds = 0;    
unsigned long currentOutageStartSecs = 0;  

// --- Epoch Tracking ---
unsigned long lastFullChargeEpoch = 0;
unsigned long systemBirthEpoch = 0; 
unsigned long baseTimeEpoch = 0; 
bool timeSynced = false;

// --- OTA State Flags ---
bool isOTAUpdating = false;
bool shouldRebootAfterOTA = false;

String wifiSSID = "";
String wifiPass = "";

String tgToken = "";
String tgChat = "";
bool tgEnable = true; 

String schStart = "";
String schEnd = "";
bool schEnable = false; 
bool isBuzzerMuted = false;

// --- Alert Flags ---
bool alert50Sent = false;
bool alert30Sent = false;
bool lowBatAlertSent = false;
bool highCutAlertSent = false; 

// --- Bot Polling Memory ---
unsigned long lastTgCheck = 0;
long lastTgUpdateId = 0;

unsigned long prevMillis500 = 0;
unsigned long prevMillis1000 = 0;
unsigned long prevWiFiReconnectMillis = 0;
unsigned long lastNtpRetryMillis = 0; 
unsigned long lastNtpPeriodicSyncMillis = 0; 
unsigned long lastSaveUptime = 0; 

const unsigned long WIFI_RECONNECT_INTERVAL = 15000; 
const unsigned long NTP_PERIODIC_INTERVAL   = 6 * 3600 * 1000UL; // Resync time every 6 hours

bool isAPMode = false;
AsyncWebServer server(80);

int buzzerBeepsRemaining = 0;
unsigned long buzzerOnDuration = 0;
unsigned long buzzerOffDuration = 0;
unsigned long buzzerStateChangeMillis = 0;
bool buzzerPinState = false;

void playBuzzer(int beeps, int onTime, int offTime, bool forceCritical = false) {
    if (isBuzzerMuted) return;
    if (buzzerBeepsRemaining > 0 && !forceCritical) return; 

    buzzerBeepsRemaining = beeps * 2; 
    buzzerOnDuration = onTime;
    buzzerOffDuration = offTime;
    buzzerPinState = true;
    digitalWrite(PIN_BUZZER, HIGH);
    buzzerStateChangeMillis = millis();
}

void handleBuzzer() {
    if (buzzerBeepsRemaining > 0) {
        unsigned long currentMillis = millis();
        unsigned long waitTime = buzzerPinState ? buzzerOnDuration : buzzerOffDuration;
        if (currentMillis - buzzerStateChangeMillis >= waitTime) {
            buzzerPinState = !buzzerPinState;
            digitalWrite(PIN_BUZZER, buzzerPinState ? HIGH : LOW);
            buzzerStateChangeMillis = currentMillis;
            buzzerBeepsRemaining--;
            if (buzzerBeepsRemaining == 0) digitalWrite(PIN_BUZZER, LOW);
        }
    }
}

String getFormattedTime(unsigned long pastSecondsOffset = 0) {
    if (!timeSynced && baseTimeEpoch == 0) { 
        unsigned long targetUp = sessionUptime > pastSecondsOffset ? (sessionUptime - pastSecondsOffset) : 0;
        unsigned long h = targetUp / 3600;
        unsigned long m = (targetUp % 3600) / 60;
        unsigned long s = targetUp % 60;
        char offTime[30];
        sprintf(offTime, "Offline_Up-%02lu:%02lu:%02lu", h, m, s);
        return String(offTime);
    } else {
        time_t targetTime = (baseTimeEpoch + sessionUptime) - pastSecondsOffset + 21600; // UTC+6
        struct tm* timeinfo = gmtime(&targetTime); 
        char timeBuff[30];
        strftime(timeBuff, sizeof(timeBuff), "%Y-%m-%d %H:%M:%S", timeinfo);
        return String(timeBuff);
    }
}

String format12HourStr(String dtStr) {
    if (dtStr.length() < 16 || dtStr.startsWith("Offline")) return dtStr;
    int hh = dtStr.substring(11, 13).toInt();
    int mm = dtStr.substring(14, 16).toInt();
    String ampm = (hh >= 12) ? "PM" : "AM";
    hh = hh % 12;
    if (hh == 0) hh = 12;
    char buff[15];
    sprintf(buff, "%02d:%02d %s", hh, mm, ampm.c_str());
    return String(buff);
}

bool tryHttpTimeSync() {
    if (WiFi.status() != WL_CONNECTED) return false;
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://worldtimeapi.org/api/timezone/Etc/UTC"); 
    http.setTimeout(4000);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        DynamicJsonDocument doc(1024);
        DeserializationError err = deserializeJson(doc, http.getStream());
        if (!err) {
            long unixtime = doc["unixtime"] | 0;
            if (unixtime > 1700000000) {
                baseTimeEpoch = unixtime - sessionUptime;
                File f = LittleFS.open("/last_epoch.txt", "w");
                if (f) { f.print(baseTimeEpoch + sessionUptime); f.close(); }
                timeSynced = true;
                if (systemBirthEpoch == 0) {
                    systemBirthEpoch = baseTimeEpoch + sessionUptime;
                    saveTotals();
                }
                http.end();
                return true;
            }
        }
    }
    http.end();
    return false;
}

String urlEncode(String text) {
    String encoded = "";
    for (int i = 0; i < text.length(); i++) {
        unsigned char c = text.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += (char)c;
        } else {
            char hex[4];
            sprintf(hex, "%%%02X", c);
            encoded += String(hex);
        }
    }
    return encoded;
}

// Sends proactive notifications (respects tgEnable)
void sendTelegramMessage(String message) {
    if (!tgEnable || tgToken == "" || tgToken == "null" || tgChat == "" || tgChat == "null" || WiFi.status() != WL_CONNECTED) return;
    WiFiClientSecure client;
    client.setInsecure(); 
    client.setBufferSizes(512, 512); 
    client.setTimeout(4000); 
    HTTPClient http;
    
    String url = "https://api.telegram.org/bot" + tgToken + "/sendMessage?chat_id=" + tgChat + "&text=" + urlEncode(message);
    http.begin(client, url);
    http.GET();
    http.end();
}

// Sends explicit bot replies (works even if tgEnable alerts are OFF)
void sendTelegramDirectReply(String message) {
    if (tgToken == "" || tgToken == "null" || tgChat == "" || tgChat == "null" || WiFi.status() != WL_CONNECTED) return;
    WiFiClientSecure client;
    client.setInsecure(); 
    client.setBufferSizes(512, 512); 
    client.setTimeout(4000); 
    HTTPClient http;
    
    String url = "https://api.telegram.org/bot" + tgToken + "/sendMessage?chat_id=" + tgChat + "&text=" + urlEncode(message);
    http.begin(client, url);
    http.GET();
    http.end();
}

void handleTelegramCommands() {
    // Only requires valid credentials and Wi-Fi connection; independent of tgEnable
    if (tgToken == "" || tgToken == "null" || tgChat == "" || tgChat == "null" || WiFi.status() != WL_CONNECTED) return;
    
    unsigned long currentMillis = millis();
    if (currentMillis - lastTgCheck < 6000) return; 
    lastTgCheck = currentMillis;

    String pendingReply = ""; 
    bool shouldUpdateMode = false;
    int newMode = 0;

    {
        WiFiClientSecure client;
        client.setInsecure();
        client.setBufferSizes(512, 512); 
        client.setTimeout(4000); 
        HTTPClient http;

        String url = "https://api.telegram.org/bot" + tgToken + "/getUpdates?limit=1";
        if (lastTgUpdateId > 0) {
            url += "&offset=" + String(lastTgUpdateId + 1);
        }

        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode == 200) {
            StaticJsonDocument<200> filter;
            filter["ok"] = true;
            filter["result"][0]["update_id"] = true;
            filter["result"][0]["message"]["text"] = true;
            filter["result"][0]["message"]["chat"]["id"] = true;

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
            
            if (!error && doc["ok"]) {
                JsonArray results = doc["result"].as<JsonArray>();
                for (JsonObject result : results) {
                    long updateId = result["update_id"];
                    if (updateId > lastTgUpdateId) {
                        lastTgUpdateId = updateId;
                        
                        String text = result["message"]["text"].as<String>();
                        String chatId = result["message"]["chat"]["id"].as<String>();

                        if (chatId == tgChat) {
                            if (text == "/status" || text == "/stats") {
                                float currentSoc = 0;
                                if (currentVoltage >= vHighCut) currentSoc = 100;
                                else if (currentVoltage <= vLowCut) currentSoc = 0;
                                else currentSoc = ((currentVoltage - vLowCut) / (vHighCut - vLowCut)) * 100;

                                String modeStr = "";
                                if (chargerMode == 1) modeStr = "FORCE ON";
                                else if (chargerMode == 2) modeStr = "FORCE OFF";
                                else modeStr = relayState ? "AUTO (ON)" : "AUTO (OFF)";

                                String chargingStateStr = relayState ? "⚡ ACTIVE (Charging)" : "🛑 OFF (Discharging / Standby)";

                                String lastFullStr = "Not recorded";
                                if (lastFullChargeEpoch > 0) {
                                    time_t targetTime = lastFullChargeEpoch + 21600; // UTC+6
                                    struct tm* t = gmtime(&targetTime);
                                    int hh = t->tm_hour;
                                    int mm = t->tm_min;
                                    String ampm = (hh >= 12) ? "PM" : "AM";
                                    hh = hh % 12;
                                    if (hh == 0) hh = 12;
                                    char buff[30];
                                    sprintf(buff, "%02d:%02d %s", hh, mm, ampm.c_str());
                                    lastFullStr = String(buff);
                                }

                                String lastCutInfo = "⏱️ Last Cut: None";
                                if (!isAcOnline) {
                                    unsigned long dur = sessionUptime - currentOutageStartSecs;
                                    String start12 = format12HourStr(getFormattedTime(dur));
                                    lastCutInfo = "⚠️ Live Cut: " + String(dur/3600) + "h " + String((dur%3600)/60) + "m " + String(dur%60) + "s (Since " + start12 + ")";
                                } else {
                                    if (LittleFS.exists("/outages.json")) {
                                        File file = LittleFS.open("/outages.json", "r");
                                        if (file) {
                                            DynamicJsonDocument docOut(4096);
                                            deserializeJson(docOut, file);
                                            JsonArray arr = docOut.as<JsonArray>();
                                            if (arr.size() > 0) {
                                                JsonObject lastOutage = arr[arr.size() - 1];
                                                String st = lastOutage["start"].as<String>();
                                                String en = lastOutage["end"].as<String>();
                                                unsigned long d = lastOutage["dur_s"].as<unsigned long>();
                                                
                                                String st12 = format12HourStr(st);
                                                String en12 = format12HourStr(en);
                                                
                                                lastCutInfo = "⏱️ Last Cut: " + String(d/3600) + "h " + String((d%3600)/60) + "m " + String(d%60) + "s (" + st12 + " to " + en12 + ")";
                                            }
                                            file.close();
                                        }
                                    }
                                }

                                pendingReply = "📊 AeonGrid Live Status\n\n";
                                pendingReply += "🔋 Battery: " + String(currentVoltage, 1) + "V (" + String((int)currentSoc) + "%)\n";
                                pendingReply += "⚡ Charger: " + chargingStateStr + "\n";
                                pendingReply += "🕒 Last Full: " + lastFullStr + "\n";
                                pendingReply += "🔌 Grid: " + String(isAcOnline ? "🟢 ONLINE" : "🔴 OFFLINE") + "\n";
                                pendingReply += "⚙️ Mode: " + modeStr + "\n\n";
                                pendingReply += lastCutInfo;
                            } 
                            else if (text == "/auto") {
                                newMode = 0; shouldUpdateMode = true;
                                pendingReply = "✅ Charger Mode set to: AUTO";
                            }
                            else if (text == "/force_on") {
                                newMode = 1; shouldUpdateMode = true;
                                pendingReply = "✅ Charger Mode set to: FORCE ON";
                            }
                            else if (text == "/force_off") {
                                newMode = 2; shouldUpdateMode = true;
                                pendingReply = "✅ Charger Mode set to: FORCE OFF";
                            }
                            else if (text == "/start" || text == "/help") {
                                pendingReply = "🤖 AeonGrid Bot Commands\n\n";
                                pendingReply += "/status or /stats - Full telemetry report\n";
                                pendingReply += "/auto - Switch to Auto Mode\n";
                                pendingReply += "/force_on - Turn Charger ON\n";
                                pendingReply += "/force_off - Turn Charger OFF";
                            }
                        }
                    }
                }
            }
        }
        http.end(); 
    }

    if (shouldUpdateMode) {
        chargerMode = newMode;
        saveSettings();
    }
    if (pendingReply != "") {
        sendTelegramDirectReply(pendingReply);
    }
}

bool isWithinSchedule() {
    if (!schEnable || schStart == "" || schEnd == "") return true;  
    if (!timeSynced && baseTimeEpoch == 0) return true; 

    time_t targetTime = (baseTimeEpoch + sessionUptime) + 21600;
    struct tm* t = gmtime(&targetTime); 
    
    int currMins = t->tm_hour * 60 + t->tm_min;
    int startMins = schStart.substring(0,2).toInt() * 60 + schStart.substring(3,5).toInt();
    int endMins = schEnd.substring(0,2).toInt() * 60 + schEnd.substring(3,5).toInt();

    if (startMins < endMins) {
        return (currMins >= startMins && currMins < endMins);
    } else { 
        return (currMins >= startMins || currMins < endMins);
    }
}

void setRelayState(bool turnOn) {
    bool prevState = relayState;
    if (prevState != turnOn) {
        relayState = turnOn;
        if (turnOn) {
            digitalWrite(PIN_RELAY_CHARGER, LOW); 
            playBuzzer(2, 100, 100); 
            highCutAlertSent = false; 
        } else {
            digitalWrite(PIN_RELAY_CHARGER, HIGH); 
            delay(200);                            
            digitalWrite(PIN_RELAY_CHARGER, LOW);  
            delay(50);                             
            digitalWrite(PIN_RELAY_CHARGER, HIGH); 
            playBuzzer(1, 100, 100); 
        }
    }
}

void saveOutageLog(String startTime, String endTime, unsigned long durationSec) {
    if (durationSec < 1) return;
    DynamicJsonDocument doc(4096);
    if (LittleFS.exists("/outages.json")) {
        File file = LittleFS.open("/outages.json", "r");
        if (file) { deserializeJson(doc, file); file.close(); }
    }
    JsonArray array = doc.as<JsonArray>();
    if (array.isNull()) array = doc.to<JsonArray>();
    if (array.size() >= 30) array.remove(0); 

    JsonObject newLog = array.createNestedObject();
    newLog["start"] = startTime;
    newLog["end"]   = endTime;
    newLog["dur_s"] = durationSec;

    File saveFile = LittleFS.open("/outages.json", "w");
    if (saveFile) { serializeJson(doc, saveFile); saveFile.close(); }
}

void loadSettings() {
    if (LittleFS.exists("/config.json")) {
        File file = LittleFS.open("/config.json", "r");
        if (file) {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, file);
            calibrationFactor = doc["calib"] | 0.03695;
            chargerMode       = doc["mode"]  | 0;
            vLowCut           = doc["vLow"]  | 21.0;
            vResume           = doc["vRes"]  | 25.2;
            vHighCut          = doc["vHigh"] | 29.0;
            tgToken           = doc["tg_token"] | "";
            tgChat            = doc["tg_chat"] | "";
            tgEnable          = doc["tg_enable"] | true;
            schStart          = doc["sch_start"] | "";
            schEnd            = doc["sch_end"] | "";
            schEnable         = doc["sch_enable"] | false;
            isBuzzerMuted     = doc["mute_buzzer"] | false;
            if (tgToken == "null") tgToken = "";
            if (tgChat == "null") tgChat = "";
            file.close();
        }
    }
    if (LittleFS.exists("/sys_totals.json")) {
        File file = LittleFS.open("/sys_totals.json", "r");
        if (file) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, file);
            totalUptimeSeconds = doc["up"] | 0;     
            totalDowntimeSeconds = doc["down"] | 0; 
            lastFullChargeEpoch = doc["lfc"] | 0;
            systemBirthEpoch = doc["birth"] | 0;
            file.close();
        }
    }
    if (LittleFS.exists("/last_epoch.txt")) {
        File file = LittleFS.open("/last_epoch.txt", "r");
        if (file) {
            String savedEpoch = file.readString();
            baseTimeEpoch = savedEpoch.toInt();
            file.close();
        }
    }
}

void saveSettings() {
    DynamicJsonDocument doc(1024);
    doc["calib"] = calibrationFactor;
    doc["mode"]  = chargerMode;
    doc["vLow"]  = vLowCut;
    doc["vRes"]  = vResume;
    doc["vHigh"] = vHighCut;
    doc["tg_token"] = tgToken;
    doc["tg_chat"] = tgChat;
    doc["tg_enable"] = tgEnable;
    doc["sch_start"] = schStart;
    doc["sch_end"] = schEnd;
    doc["sch_enable"] = schEnable;
    doc["mute_buzzer"] = isBuzzerMuted;
    File file = LittleFS.open("/config.json", "w");
    if (file) { serializeJson(doc, file); file.close(); }
}

void saveTotals() {
    DynamicJsonDocument doc(256);
    doc["up"] = totalUptimeSeconds; 
    doc["down"] = totalDowntimeSeconds;
    doc["lfc"] = lastFullChargeEpoch;
    doc["birth"] = systemBirthEpoch;
    File file = LittleFS.open("/sys_totals.json", "w");
    if (file) { serializeJson(doc, file); file.close(); }
}

bool loadWiFiCredentials() {
    if (LittleFS.exists("/wifi.json")) {
        File configFile = LittleFS.open("/wifi.json", "r");
        if (configFile) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, configFile);
            wifiSSID = doc["ssid"].as<String>();
            wifiPass = doc["pass"].as<String>();
            configFile.close();
            return (wifiSSID.length() > 0);
        }
    }
    return false;
}

void processLocalAutomation() {
    long rawAnalogSum = 0;
    for (int i = 0; i < 10; i++) {
        rawAnalogSum += analogRead(PIN_VOLTAGE_ANALOG);
        delayMicroseconds(100);
    }
    rawAnalogAvg = rawAnalogSum / 10.0;
    currentVoltage = rawAnalogAvg * calibrationFactor;

    if (currentVoltage >= vHighCut && (timeSynced || baseTimeEpoch > 0)) {
        lastFullChargeEpoch = baseTimeEpoch + sessionUptime;
    }

    bool prevAcState = isAcOnline;
    isAcOnline = (digitalRead(PIN_AC_SENSE) == HIGH); 

    if (prevAcState && !isAcOnline) { 
        currentOutageStartSecs = sessionUptime;
        playBuzzer(3, 100, 100); 
        sendTelegramMessage("🔴 GRID OFFLINE: Power Cut Detected!");
    } 
    else if (!prevAcState && isAcOnline) { 
        unsigned long duration = sessionUptime - currentOutageStartSecs;
        
        time_t startT = (baseTimeEpoch + currentOutageStartSecs) + 21600;
        time_t endT = (baseTimeEpoch + sessionUptime) + 21600;
        
        struct tm* tmStart = gmtime(&startT);
        int startDay = tmStart->tm_yday;
        struct tm* tmEnd = gmtime(&endT);
        int endDay = tmEnd->tm_yday;
        
        if (startDay != endDay && baseTimeEpoch > 0) {
            unsigned long secsToMidnight = 86400 - (startT % 86400); 
            unsigned long dur1 = secsToMidnight;
            unsigned long dur2 = duration - dur1;
            
            String startTimeStr = getFormattedTime(duration);
            String midnight1Str = getFormattedTime(dur2);
            String midnight2Str = getFormattedTime(dur2 - 1); 
            String endTimeStr = getFormattedTime(0);
            
            saveOutageLog(startTimeStr, midnight1Str, dur1);
            saveOutageLog(midnight2Str, endTimeStr, dur2);
        } else {
            String endTimeStr = getFormattedTime(0);
            String startTimeStr = getFormattedTime(duration); 
            saveOutageLog(startTimeStr, endTimeStr, duration);
        }
        
        saveTotals(); 
        
        sendTelegramMessage("🟢 GRID ONLINE: Power Restored! Duration: " + String(duration/3600) + "h " + String((duration%3600)/60) + "m " + String(duration%60) + "s");
        
        alert50Sent = false; alert30Sent = false; lowBatAlertSent = false;
    }

    float currentSoc = 0;
    if (currentVoltage >= vHighCut) currentSoc = 100;
    else if (currentVoltage <= vLowCut) currentSoc = 0;
    else currentSoc = ((currentVoltage - vLowCut) / (vHighCut - vLowCut)) * 100;

    if (!isAcOnline) {
        if (currentSoc <= 50.0 && !alert50Sent) {
            sendTelegramMessage("🔋 Update: Battery dropped to 50% (" + String(currentVoltage, 1) + "V)");
            alert50Sent = true;
        }
        if (currentSoc <= 30.0 && !alert30Sent) {
            playBuzzer(5, 100, 100); 
            sendTelegramMessage("🔋 Update: Battery dropped to 30% (" + String(currentVoltage, 1) + "V)");
            alert30Sent = true;
        }
    }

    if (currentVoltage <= vLowCut) {
        if (!lowBatAlertSent && !isAcOnline) {
            playBuzzer(1, 5000, 0, true); 
            sendTelegramMessage("⚠️ CRITICAL: Battery Very Low (" + String(currentVoltage, 1) + "V)! Auto-Cut imminent.");
            lowBatAlertSent = true;
        }
    } else if (currentVoltage >= vLowCut + 1.0) {
        lowBatAlertSent = false;
    }

    if (currentVoltage >= vHighCut) {
        if (!highCutAlertSent && relayState) {
            sendTelegramMessage("✅ BATTERY FULL: Reached High Cut (" + String(currentVoltage, 1) + "V). Charger Stopped.");
            highCutAlertSent = true;
        }
    } else if (currentVoltage < vHighCut - 1.0) {
        highCutAlertSent = false;
    }

    if (chargerMode == 1) { 
        setRelayState(true); 
    } 
    else if (chargerMode == 2) { 
        setRelayState(false); 
    } 
    else { 
        if (!isAcOnline) setRelayState(false); 
        else if (currentVoltage >= vHighCut) setRelayState(false);
        else if (currentVoltage <= vResume && isAcOnline) {
            if (isWithinSchedule()) setRelayState(true);
            else setRelayState(false);
        }  
        else if (currentVoltage <= vLowCut) setRelayState(false);
        else if (relayState && !isWithinSchedule()) setRelayState(false); 
    }
}

void updateLedStatus() {
    digitalWrite(PIN_INBUILT_LED, (WiFi.status() == WL_CONNECTED) ? LOW : HIGH);
}

void handleWiFiRecovery() {
    if (wifiSSID.length() == 0) return;
    if (WiFi.status() == WL_CONNECTED) {
        if (isAPMode) { WiFi.mode(WIFI_STA); isAPMode = false; }
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - prevWiFiReconnectMillis >= WIFI_RECONNECT_INTERVAL) {
            prevWiFiReconnectMillis = currentMillis;
            if (!isAPMode) {
                isAPMode = true;
                WiFi.mode(WIFI_AP_STA);
                WiFi.softAP("AeonGrid_AP", "12345678");
            }
            WiFi.disconnect(); WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    digitalWrite(PIN_RELAY_CHARGER, HIGH);
    pinMode(PIN_RELAY_CHARGER, OUTPUT);
    
    pinMode(PIN_AC_SENSE, INPUT);
    pinMode(PIN_INBUILT_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    
    relayState = false;
    digitalWrite(PIN_INBUILT_LED, HIGH);
    digitalWrite(PIN_BUZZER, LOW);

    delay(200); 
    int acStableCount = 0;
    for(int i = 0; i < 10; i++) { 
        acStableCount += digitalRead(PIN_AC_SENSE); 
        delay(10); 
    }
    isAcOnline = (acStableCount > 5); 
    
    sessionUptime = 0; 
    playBuzzer(1, 100, 100); 

    if (!LittleFS.begin()) Serial.println("LittleFS Mount Failed!");
    else loadSettings();

    if (!isAcOnline) {
        currentOutageStartSecs = sessionUptime;
    }
    
    processLocalAutomation();

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);

    if (loadWiFiCredentials()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 8) { delay(500); attempts++; }
    }

    if (WiFi.status() != WL_CONNECTED) {
        isAPMode = true;
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP("AeonGrid_AP", "12345678");
    } else {
        isAPMode = false;
        configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    }

    if (MDNS.begin("aeongrid")) MDNS.addService("http", "tcp", 80);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", INDEX_HTML);
    });

    server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(4096);
        
        doc["voltage"]    = String(currentVoltage, 2);
        doc["grid"]       = isAcOnline ? "ONLINE" : "OFFLINE";
        doc["relay"]      = relayState ? "ON" : "OFF";
        doc["mode"]       = chargerMode;
        doc["session_uptime"] = sessionUptime;
        doc["total_uptime_s"] = totalUptimeSeconds;
        doc["downtime_s"] = totalDowntimeSeconds;
        doc["l_low"]      = String(vLowCut, 1);
        doc["l_res"]      = String(vResume, 1);
        doc["l_high"]     = String(vHighCut, 1);
        doc["connected_ssid"] = (WiFi.status() == WL_CONNECTED) ? wifiSSID : "Offline AP Mode";
        
        doc["current_outage_start"] = isAcOnline ? "" : getFormattedTime(sessionUptime - currentOutageStartSecs);
        doc["current_outage_s"] = isAcOnline ? 0 : (sessionUptime - currentOutageStartSecs);
        
        doc["last_full_epoch"] = lastFullChargeEpoch;
        doc["birth_epoch"] = systemBirthEpoch; 
        
        doc["tg_token"] = (tgToken == "null") ? "" : tgToken;
        doc["tg_chat"] = (tgChat == "null") ? "" : tgChat;
        doc["tg_enable"] = tgEnable;
        
        doc["sch_start"] = schStart;
        doc["sch_end"] = schEnd;
        doc["sch_enable"] = schEnable;
        doc["mute_buzzer"] = isBuzzerMuted;
        
        doc["sys_epoch"] = String(baseTimeEpoch + sessionUptime);
        doc["time_synced"] = timeSynced;

        if (LittleFS.exists("/outages.json")) {
            File file = LittleFS.open("/outages.json", "r");
            if (file) {
                DynamicJsonDocument outageDoc(4096);
                deserializeJson(outageDoc, file);
                doc["outages"] = outageDoc;
                file.close();
            }
        }
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/sync_time", HTTP_POST, [](AsyncWebServerRequest *request){
        if (WiFi.status() == WL_CONNECTED) {
            configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
            tryHttpTimeSync();
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(200, "application/json", "{\"success\":false}");
        }
    });
    
    server.on("/api/disable_timer", HTTP_POST, [](AsyncWebServerRequest *request){
        schEnable = false; saveSettings(); request->send(200, "application/json", "{\"success\":true}");
    });
    
    server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Restarting"); delay(500); ESP.restart();
    });

    server.on("/api/buzzer", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("mute")) {
            isBuzzerMuted = (request->getParam("mute")->value() == "1");
            if(isBuzzerMuted) digitalWrite(PIN_BUZZER, LOW); 
            saveSettings(); request->send(200, "text/plain", "Buzzer Updated");
        } else request->send(400, "text/plain", "Bad Request");
    });

    server.on("/api/mode", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("mode")) {
            chargerMode = request->getParam("mode")->value().toInt();
            saveSettings(); request->send(200, "text/plain", "OK");
        } else request->send(400, "text/plain", "Bad Request");
    });

    server.on("/api/calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("v")) {
            float v = request->getParam("v")->value().toFloat();
            if (v > 0 && rawAnalogAvg > 0) {
                calibrationFactor = v / rawAnalogAvg; saveSettings();
                request->send(200, "text/plain", "Calibrated"); return;
            }
        }
        request->send(400, "text/plain", "Invalid Input");
    });

    server.on("/api/limits", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("low") && request->hasParam("res") && request->hasParam("high")) {
            vLowCut = request->getParam("low")->value().toFloat();
            vResume = request->getParam("res")->value().toFloat();
            vHighCut = request->getParam("high")->value().toFloat();
            saveSettings(); request->send(200, "text/plain", "Limits Updated");
        } else request->send(400, "text/plain", "Invalid Data");
    });

    server.on("/api/download_csv", HTTP_GET, [](AsyncWebServerRequest *request){
        String csv = "Start Time,Restored At,Duration (Seconds)\n";
        if (LittleFS.exists("/outages.json")) {
            File file = LittleFS.open("/outages.json", "r");
            if (file) {
                DynamicJsonDocument doc(4096); deserializeJson(doc, file);
                JsonArray arr = doc.as<JsonArray>();
                for (JsonVariant v : arr) {
                    csv += v["start"].as<String>() + "," + v["end"].as<String>() + "," + v["dur_s"].as<String>() + "\n";
                }
                file.close();
            }
        }
        AsyncWebServerResponse *response = request->beginResponse(200, "text/csv", csv);
        response->addHeader("Content-Disposition", "attachment; filename=\"AeonGrid_Logs.csv\"");
        request->send(response);
    });

    server.on("/save-telegram", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("tg_token", true)) tgToken = request->getParam("tg_token", true)->value();
        if (request->hasParam("tg_chat", true)) tgChat = request->getParam("tg_chat", true)->value();
        tgEnable = request->hasParam("tg_enable", true); 
        saveSettings();
        request->send(200, "text/plain", "OK");
    });
    
    server.on("/save-timer", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("sch_start", true)) schStart = request->getParam("sch_start", true)->value();
        if (request->hasParam("sch_end", true)) schEnd = request->getParam("sch_end", true)->value();
        schEnable = true; 
        saveSettings();
        request->send(200, "text/plain", "OK");
    });

    server.on("/api/clear-logs", HTTP_POST, [](AsyncWebServerRequest *request){
        if (LittleFS.exists("/outages.json")) LittleFS.remove("/outages.json");
        if (LittleFS.exists("/sys_totals.json")) LittleFS.remove("/sys_totals.json");
        totalUptimeSeconds = 0; totalDowntimeSeconds = 0;
        lastFullChargeEpoch = 0;
        systemBirthEpoch = 0;
        request->send(200, "text/plain", "Logs Cleared");
    });

    server.on("/save-wifi", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
            DynamicJsonDocument doc(256);
            doc["ssid"] = request->getParam("ssid", true)->value();
            doc["pass"] = request->getParam("pass", true)->value();
            File f = LittleFS.open("/wifi.json", "w");
            if (f) { serializeJson(doc, f); f.close(); }
            request->send(200, "text/html", "<h3>WiFi Saved! Restarting Device...</h3>");
            delay(2000); ESP.restart();
        } else request->send(400, "text/html", "Invalid Data");
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
        bool success = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", success ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
        
        if (success) {
            shouldRebootAfterOTA = true; 
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if(!index){
            isOTAUpdating = true;
            Update.runAsync(true);
            if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){ }
        }
        if(!Update.hasError()){
            Update.write(data, len);
            digitalWrite(PIN_INBUILT_LED, !digitalRead(PIN_INBUILT_LED)); 
        }
        if(final){
            if(Update.end(true)){ } 
            isOTAUpdating = false; 
        }
    });

    server.begin();
}

void loop() {
    if (shouldRebootAfterOTA) {
        delay(1000);
        ESP.restart();
    }

    MDNS.update(); 
    handleBuzzer();

    unsigned long currentMillis = millis();
    
    if (!isOTAUpdating) {
        if (WiFi.status() == WL_CONNECTED) {
            time_t now = time(nullptr);
            
            // Periodic 6-hour resync or initial boot sync
            if (!timeSynced || (currentMillis - lastNtpPeriodicSyncMillis >= NTP_PERIODIC_INTERVAL)) {
                if (now > 1700000000) { 
                    baseTimeEpoch = now - sessionUptime; 
                    File f = LittleFS.open("/last_epoch.txt", "w");
                    if (f) { f.print(baseTimeEpoch + sessionUptime); f.close(); }
                    timeSynced = true; 
                    lastNtpPeriodicSyncMillis = currentMillis;

                    if (systemBirthEpoch == 0) {
                        systemBirthEpoch = baseTimeEpoch + sessionUptime;
                        saveTotals();
                    }
                }
                else if (currentMillis - lastNtpRetryMillis >= 10000 || lastNtpRetryMillis == 0) { 
                    lastNtpRetryMillis = currentMillis;
                    if (!tryHttpTimeSync()) {
                        configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
                    }
                }
            }
        }

        handleTelegramCommands();

        if (currentMillis - prevMillis500 >= 500) {
            prevMillis500 = currentMillis;
            processLocalAutomation();
            updateLedStatus();
        }
    }
    
    unsigned long elapsedMillis = currentMillis - prevMillis1000;
    if (elapsedMillis >= 1000) {
        unsigned long elapsedSecs = elapsedMillis / 1000;
        sessionUptime += elapsedSecs; 
        totalUptimeSeconds += elapsedSecs;
        
        if (!isAcOnline) totalDowntimeSeconds += elapsedSecs;
        
        prevMillis1000 = currentMillis - (elapsedMillis % 1000); 
        
        if (!isOTAUpdating && (sessionUptime - lastSaveUptime >= 3600)) {
             saveTotals();
             if (timeSynced || baseTimeEpoch > 0) {
                 File f = LittleFS.open("/last_epoch.txt", "w");
                 if (f) { f.print(baseTimeEpoch + sessionUptime); f.close(); }
             }
             lastSaveUptime = sessionUptime;
        }
    }
    
    if (!isOTAUpdating) {
        handleWiFiRecovery();
    }
}
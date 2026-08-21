# AeonGrid
AeonGrid is a standalone Smart Charge Controller and Grid Telemetry dashboard for ESP8266. It monitors voltage, logs grid outages to a 7-day heatmap, and automates charger relays. Featuring a premium UI, Telegram bot alerts, smart scheduling, and OTA updates, it runs 100% locally with zero cloud dependency. Perfect for smart home backups!


# ⚡ AeonGrid: Smart Charge Controller & Telemetry

![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-blue.svg)
![Platform](https://img.shields.io/badge/Platform-ESP8266-success)
![UI](https://img.shields.io/badge/UI-JetBrains%20Mono-black)

AeonGrid is a high-performance, completely standalone Smart Charge Controller and Grid Telemetry dashboard built for the ESP8266. It monitors battery voltage, tracks local grid power outages in real-time, and controls a battery charger via an aerospace-grade, cyberpunk-inspired web UI.

It operates with **Zero Cloud Dependency**. All analytics, heatmaps, and telemetry data are saved directly to the ESP8266's onboard LittleFS flash memory.

<img width="1326" height="937" alt="image" src="https://github.com/user-attachments/assets/3a5324f0-1a0e-4e65-a3db-b1869c5fafcd" />

<img width="1329" height="957" alt="image" src="https://github.com/user-attachments/assets/54f589f7-57df-4a4c-839c-9a3a283fce25" />



## ✨ Key Features

*   **Premium Local UI:** A stunning, frosted-glass dashboard featuring a dynamic roaming 3-orb mesh gradient, pure `JetBrains Mono` typography, and smooth CSS keyframe animations (like the Comet Trail charging indicator).
*   **7-Day Grid Heatmap:** Automatically logs power cuts and displays grid stability on a GitHub-style frequency matrix, calibrated to the system's exact installation "Birth Date".
*   **Smart Charger Control:** Automated charging based on highly configurable Low/Resume/High voltage limits, with a manual override via a sleek sliding toggle.
*   **Telegram Bot Integration:** Sends instant alerts for power cuts, low battery, and full charge. Reply to your bot with `/stats` or `/auto` to command the system remotely.
*   **Smart Scheduling Timer:** Restrict charger activation to specific hours of the day (e.g., off-peak electricity hours).
*   **OTA Updates:** Flash new `.bin` firmware files directly through the web interface without touching the hardware.

## 🔔 System Alerts & Audio Events

AeonGrid keeps you actively informed of critical power changes through both physical buzzer patterns and remote Telegram push notifications.

### 📱 Telegram Push Alerts
If enabled in the dashboard settings, the bot will automatically notify you when:
*   **Grid Drops:** 🔴 Power cut detected.
*   **Grid Restored:** 🟢 Power restored (includes the total outage duration).
*   **Battery at 50%:** 🔋 Warning that battery is at half capacity.
*   **Battery at 30%:** 🔋 Warning that battery is getting low.
*   **Battery Critical:** ⚠️ Low-Cut limit reached, auto-cut imminent.
*   **Charge Complete:** ✅ High-Cut limit reached, charger turned off safely.

### 🔊 Buzzer Beep Codes
The onboard Piezo buzzer uses specific beep patterns to indicate hardware states (this can be easily muted via the dashboard UI):
*   **1 Short Beep:** System Booted / Charger toggled OFF.
*   **2 Short Beeps:** Charger toggled ON.
*   **3 Quick Beeps:** Grid Power Cut detected!
*   **5 Quick Beeps:** Battery dropped to 30% capacity.
*   **1 Long Beep (5 Seconds):** CRITICAL - Battery empty (Low-Cut limit reached).












## 🛠️ Hardware Requirements & Custom Modules

* **Microcontroller:** ESP8266 NodeMCU V3
* **Battery System:** 24V setup (2x 12V Drycells in series) supported by default.
* **Relay Module:** Standard 2-Channel 5V Relay Module (Active Low).
* **Piezo Buzzer:** Standard 3.3V or 5V passive/active buzzer for audible alerts.

### 1. Custom Battery Voltage Sensor (A0)

To safely measure a 24V battery bank (which can reach up to ~29V when charging) on the ESP8266's A0 pin (which has a 3.3V limit), you need a simple resistor voltage divider network.

* **R1 (100kΩ Resistor):** Connect one end to the **Battery Positive (+)** terminal, and the other end to the NodeMCU **A0** pin.
* **R2 (10kΩ Resistor):** Connect one end to the NodeMCU **A0** pin, and the other end to **Ground (GND)**.
* *Note: Connect the Battery Negative (-) terminal to the NodeMCU GND.*

### 2. AC Grid Sensing (5V USB Charger)

Instead of dealing with dangerous high-voltage AC wiring, we use a simple, isolated 5V smartphone USB charger.

* Plug a standard 5V USB phone charger directly into the grid/wall socket.
* Take an old USB cable, cut the end off, and strip the wires.
* Connect the **USB Ground (Black wire)** to the NodeMCU **GND**.
* Connect the **USB 5V (Red wire)** to NodeMCU **D1 (GPIO 5)**.
* *Safety Note: ESP8266 pins are 5V tolerant, but adding a 1kΩ to 10kΩ resistor in series on the 5V line is recommended for best practice.*

---

## 🔌 Wiring & Pinout Summary

| ESP8266 Pin | Component / Function | Wiring Connection |
| --- | --- | --- |
| **`A0`** | Dc Voltage  | Middle of R1 (100kΩ) and R2 (10kΩ) from Battery. |
| **`D1` (GPIO 5)** | AC Grid Sensor | 5V+ line from the Grid-plugged USB charger. |
| **`D6` (GPIO 12)** | Charger Relay | `IN1` or `IN2` on the 2-Channel Relay Module. |
| **`D7` (GPIO 13)** | Piezo Buzzer | Positive pin of the Piezo buzzer. |
| **`D4` (GPIO 2)** | Inbuilt LED | (Internal) Visual indicator for Wi-Fi connection. |
| **`GND`** | Common Ground | Tie USB GND, Battery GND, and Relay GND here. |

---

## 🚀 Installation & Setup

You can install AeonGrid by either flashing the pre-compiled binary or building it from the source code.

### Option A: Flash the Pre-Compiled `.bin` (Easiest)

1. Download the latest `AeonGrid_vX.X.bin` release from the **[Releases](https://www.google.com/search?q=%23)** page.
2. Download an ESP8266 flashing tool like [NodeMCU PyFlasher](https://www.google.com/search?q=https://github.com/marcelstoer/nodemcu-pyflasher) or the [ESP Web Tools](https://www.google.com/search?q=https://esp.huhn.me/).
3. Connect your NodeMCU to your PC via USB.
4. Select your COM port, choose the `.bin` file, and hit **Flash**.

### Option B: Compile from Source (For Developers)

1. Open `AeonGrid.ino` in the Arduino IDE.
2. Install the required libraries: `ESPAsyncWebServer`, `ArduinoJson`, etc.
3. Ensure your flash size is set to include at least **1MB or 2MB FS (LittleFS)** in the Tools menu.
4. Compile and Upload.

### Initial Configuration

1. **Connect to AP:** Upon first boot, the device will host its own Wi-Fi network named `AeonGrid_AP` (Password: `12345678`).
2. **Configure Network:** Navigate to `[http://192.168.4.1](http://192.168.4.1)` in your browser. Enter your home Wi-Fi SSID and Password. The device will reboot and join your network.
3. **Calibrate:** Access the dashboard via its local IP or `[http://aeongrid.local](http://aeongrid.local)`. Go to **Settings**, measure your battery with a real multimeter, and enter that exact voltage into the calibration field to permanently sync the A0 pin accuracy.







## 🤖 Telegram Bot Setup

1. Message `@BotFather` on Telegram to create a new bot and get your **HTTP API Token**.
2. Get your personal **Chat ID** (using a bot like `@userinfobot`).
3. Enter both into the AeonGrid settings panel to enable instant push notifications and remote commands.

## 📜 License

**CC BY-NC 4.0 (Creative Commons Attribution-NonCommercial 4.0 International)**

You are completely free to download, modify, and build this project for your own personal use, home setups, or educational purposes. 

You **may not** use this material for commercial purposes (you cannot sell the software, the dashboard, or hardware pre-loaded with this code). If you remix or share this project, you must provide appropriate credit to the original author.

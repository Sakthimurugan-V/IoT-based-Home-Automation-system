/*
 * ============================================================
 *   ESP32-Based Smart Home Automation System
 *   Platform  : Arduino IDE + Blynk IoT
 *   Board     : ESP32 30-pin Development Module
 *
 *   SIMPLIFIED VERSION – MANUAL MODE ONLY
 *   - Relays controlled exclusively via Blynk app (manual)
 *   - WiFi lost → all 4 relays turn ON automatically
 *   - Buzzer triggers when any object is within 20 cm
 *     (works in BOTH online and offline mode)
 * ============================================================
 *
 * LIBRARY REQUIREMENTS (install via Arduino Library Manager):
 *   1. Blynk   – search "Blynk" by Volodymyr Shymanskyy  (v1.3.x)
 *   2. NewPing – search "NewPing" by Tim Eckel
 *
 * BOARD SETUP (Arduino IDE):
 *   File → Preferences → Additional Boards Manager URLs:
 *     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *   Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
 *   Tools → Port  → Select your COM port
 *   Upload Speed → 115200
 *
 * BLYNK SETUP:
 *   1. Create a free account at https://blynk.cloud
 *   2. Create a new Template → Device: ESP32
 *   3. Add Datastreams for V0–V11 (Integer as needed)
 *   4. Copy BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME, BLYNK_AUTH_TOKEN
 *      and paste them below.
 *
 * VIRTUAL PIN SUMMARY:
 *   V0  – Relay 1 control          (Button widget, Switch mode)
 *   V1  – Relay 2 control          (Button widget, Switch mode)
 *   V2  – Relay 3 control          (Button widget, Switch mode)
 *   V3  – Relay 4 control          (Button widget, Switch mode)
 *   V5  – Temperature °C           (Gauge / SuperChart)
 *   V6  – Distance cm              (Gauge / SuperChart)
 *   V7  – Relay 1 Status LED       (LED widget)
 *   V8  – Relay 2 Status LED       (LED widget)
 *   V9  – Relay 3 Status LED       (LED widget)
 *   V10 – Relay 4 Status LED       (LED widget)
 *   V11 – Buzzer Alarm Indicator   (LED widget)
 *
 * RELAY FLICKER FIX:
 *   All relay writes go through setRelay(), which checks the current
 *   state before calling digitalWrite(). If the requested state is
 *   already set, the GPIO is NOT touched — eliminating relay chatter.
 *
 * BUZZER RULE (unified for online & offline):
 *   ANY object detected within 20 cm → buzzer ON
 *   Object moves beyond 20 cm        → buzzer OFF
 *   V11 is updated on Blynk only when connected.
 * ============================================================
 */

// ── Blynk credentials ────────────────────────────────────────
#define BLYNK_TEMPLATE_ID   "TMPL3TXi3DnbV"
#define BLYNK_TEMPLATE_NAME "SmartHome"
#define BLYNK_AUTH_TOKEN    "VfKaKcwwAyPOwZW2fbQQwKRHxgvIznUw"
#define BLYNK_PRINT Serial

// ── Core libraries ───────────────────────────────────────────
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <NewPing.h>

// ── WiFi credentials ─────────────────────────────────────────
const char* WIFI_SSID     = "projectap";
const char* WIFI_PASSWORD = "12345678";

// ── GPIO Pin Definitions ─────────────────────────────────────
// Relay module (active-LOW: LOW = relay ON, HIGH = relay OFF)
#define RELAY1_PIN        25
#define RELAY2_PIN        13
#define RELAY3_PIN        14
#define RELAY4_PIN        27

// LM35 temperature sensor (analog input)
#define LM35_PIN          36

// Passive buzzer – driven via LEDC PWM
#define BUZZER_PIN        12
#define BUZZER_FREQ_HZ  3000    // 3 kHz tone
#define BUZZER_RESOLUTION  8    // 8-bit resolution → duty range 0–255
#define BUZZER_DUTY_ON   128    // 50% duty cycle → audible tone
#define BUZZER_DUTY_OFF    0    // 0% duty cycle  → silent

// HC-SR04 ultrasonic sensor
#define TRIG_PIN          19
#define ECHO_PIN          18

// WiFi status indicator LED
#define WIFI_LED_PIN       4

// ── Ultrasonic sensor configuration ──────────────────────────
#define MAX_DISTANCE_CM   400   // Maximum measurable range (cm)
#define ALARM_DISTANCE_CM  20   // Buzzer trigger threshold (cm) — online & offline

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE_CM);

// ── Blynk timer ──────────────────────────────────────────────
BlynkTimer timer;

// ── System state ─────────────────────────────────────────────
bool wifiConnected  = false;   // mirrors actual WiFi connection state
bool blynkConnected = false;   // mirrors Blynk connection state

// Relay states – true = relay energised (load ON)
// Stored here so setRelay() can skip redundant GPIO writes (flicker fix).
bool relayState[4] = { false, false, false, false };

// Buzzer state – prevents redundant ledcWrite / Blynk V11 writes
bool buzzerActive = false;

// ── Timing constants ─────────────────────────────────────────
#define SENSOR_INTERVAL_MS  1000   // Sensor → Blynk update period
#define ALARM_INTERVAL_MS    200   // Ultrasonic alarm poll period
#define WIFI_CHECK_INTERVAL 5000   // WiFi reconnect check period
#define SERIAL_BAUD        115200

// ── Forward declarations ──────────────────────────────────────
void setRelay(uint8_t index, bool on);
void setAllRelays(bool on);
void updateBlynkRelayLEDs();
void sendSensorData();
void checkUltrasonicAlarm();
void buzzerOn();
void buzzerOff();
float readTemperatureCelsius();
unsigned int readDistanceCM();
void handleWiFiReconnect();
void printStatus();

// ─────────────────────────────────────────────────────────────
//  BLYNK VIRTUAL PIN HANDLERS
// ─────────────────────────────────────────────────────────────

// V0 → Relay 1
BLYNK_WRITE(V0) {
    bool state = param.asInt();
    setRelay(0, state);
    Serial.printf("[Blynk] V0 → Relay 1 = %s\n", state ? "ON" : "OFF");
}

// V1 → Relay 2
BLYNK_WRITE(V1) {
    bool state = param.asInt();
    setRelay(1, state);
    Serial.printf("[Blynk] V1 → Relay 2 = %s\n", state ? "ON" : "OFF");
}

// V2 → Relay 3
BLYNK_WRITE(V2) {
    bool state = param.asInt();
    setRelay(2, state);
    Serial.printf("[Blynk] V2 → Relay 3 = %s\n", state ? "ON" : "OFF");
}

// V3 → Relay 4
BLYNK_WRITE(V3) {
    bool state = param.asInt();
    setRelay(3, state);
    Serial.printf("[Blynk] V3 → Relay 4 = %s\n", state ? "ON" : "OFF");
}

// Fires every time Blynk (re)connects – sync widget states with device
BLYNK_CONNECTED() {
    Serial.println("[Blynk] Connected – syncing widget states");
    Blynk.syncVirtual(V0, V1, V2, V3);
    updateBlynkRelayLEDs();
    // Reflect real buzzer state on V11 after reconnect
    Blynk.virtualWrite(V11, buzzerActive ? 255 : 0);
    blynkConnected = true;
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Relay control  (FLICKER FIX)
// ─────────────────────────────────────────────────────────────

/**
 * Set a relay ON or OFF.
 * Only calls digitalWrite() if the requested state differs from
 * the currently stored state — prevents relay chatter.
 *
 * index : 0–3  →  Relay 1–4
 * on    : true = energise relay (load ON)
 *
 * Active-LOW module:
 *   GPIO LOW  → relay ON
 *   GPIO HIGH → relay OFF
 */
void setRelay(uint8_t index, bool on) {
    if (index > 3) return;
    if (relayState[index] == on) return;   // already in correct state — skip

    const uint8_t pins[4] = { RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN };
    relayState[index] = on;
    digitalWrite(pins[index], on ? LOW : HIGH);   // active-LOW
}

/** Set all four relays to the same state at once. */
void setAllRelays(bool on) {
    for (uint8_t i = 0; i < 4; i++) {
        setRelay(i, on);
    }
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Buzzer control via LEDC PWM
// ─────────────────────────────────────────────────────────────

/** Sound the passive buzzer. No-op if already ON. */
void buzzerOn() {
    if (buzzerActive) return;
    ledcWrite(BUZZER_PIN, BUZZER_DUTY_ON);
    buzzerActive = true;
}

/** Silence the buzzer. No-op if already OFF. */
void buzzerOff() {
    if (!buzzerActive) return;
    ledcWrite(BUZZER_PIN, BUZZER_DUTY_OFF);
    buzzerActive = false;
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Sensor readings
// ─────────────────────────────────────────────────────────────

/**
 * Read LM35 and return temperature in °C.
 * 10 samples averaged to reduce ADC noise.
 */
float readTemperatureCelsius() {
    const uint8_t SAMPLES = 10;
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLES; i++) {
        sum += analogRead(LM35_PIN);
        delayMicroseconds(100);
    }
    float avgADC  = (float)sum / SAMPLES;
    float voltage = (avgADC / 4095.0f) * 3300.0f;   // mV
    float tempC   = voltage / 10.0f;                  // °C
    return tempC;
}

/**
 * Ping HC-SR04 and return distance in cm.
 * Returns MAX_DISTANCE_CM when no echo received (no object).
 */
unsigned int readDistanceCM() {
    unsigned int dist = sonar.ping_cm();
    if (dist == 0) dist = MAX_DISTANCE_CM;
    return dist;
}

// ─────────────────────────────────────────────────────────────
//  TIMER CALLBACK: Send sensor data to Blynk (every 1 s)
// ─────────────────────────────────────────────────────────────
void sendSensorData() {
    if (!Blynk.connected()) return;

    float        tempC = readTemperatureCelsius();
    unsigned int dist  = readDistanceCM();

    Blynk.virtualWrite(V5, tempC);
    Blynk.virtualWrite(V6, dist);
    updateBlynkRelayLEDs();

    Serial.printf("[Sensors] Temp: %.1f °C | Distance: %u cm\n", tempC, dist);
}

// ─────────────────────────────────────────────────────────────
//  TIMER CALLBACK: Ultrasonic alarm (every 200 ms)
//
//  UNIFIED RULE (online AND offline):
//    Object within 20 cm → buzzer ON
//    Object beyond 20 cm → buzzer OFF
//
//  V11 LED widget on Blynk is updated only when:
//    (a) connected to Blynk, AND
//    (b) the alarm state has actually changed (avoids write flood)
// ─────────────────────────────────────────────────────────────
void checkUltrasonicAlarm() {
    unsigned int dist        = readDistanceCM();
    bool         alarmActive = (dist < ALARM_DISTANCE_CM);

    if (alarmActive) {
        buzzerOn();
        Serial.printf("[Alarm] Object at %u cm – BUZZER ON\n", dist);
    } else {
        if (buzzerActive) {
            buzzerOff();
            Serial.printf("[Alarm] Object at %u cm – BUZZER OFF\n", dist);
        }
    }

    // Push V11 to Blynk only on state change, not every 200 ms
    static bool lastAlarmState = false;
    if (alarmActive != lastAlarmState) {
        lastAlarmState = alarmActive;
        if (Blynk.connected()) {
            Blynk.virtualWrite(V11, alarmActive ? 255 : 0);
            Serial.printf("[V11] Buzzer alarm indicator = %s\n",
                          alarmActive ? "ON" : "OFF");
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Push relay states to Blynk LED widgets (V7–V10)
// ─────────────────────────────────────────────────────────────
void updateBlynkRelayLEDs() {
    if (!Blynk.connected()) return;
    Blynk.virtualWrite(V7,  relayState[0] ? 255 : 0);
    Blynk.virtualWrite(V8,  relayState[1] ? 255 : 0);
    Blynk.virtualWrite(V9,  relayState[2] ? 255 : 0);
    Blynk.virtualWrite(V10, relayState[3] ? 255 : 0);
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Non-blocking WiFi / Blynk reconnect (every 5 s)
// ─────────────────────────────────────────────────────────────
void handleWiFiReconnect() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < WIFI_CHECK_INTERVAL) return;
    lastCheck = millis();

    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiConnected) {
            // Just reconnected
            wifiConnected = true;
            digitalWrite(WIFI_LED_PIN, HIGH);
            Serial.println("[WiFi] Reconnected → " + WiFi.localIP().toString());
            // NOTE: Relays are NOT reset here on reconnect.
            // They were all ON in offline mode; Blynk will sync
            // their correct states via BLYNK_CONNECTED() → syncVirtual.
        }
    } else {
        if (wifiConnected) {
            // Just lost connection
            wifiConnected  = false;
            blynkConnected = false;
            digitalWrite(WIFI_LED_PIN, LOW);
            Serial.println("[WiFi] Connection lost – entering Offline Mode");
            setAllRelays(true);   // Offline rule: all relays ON
        }
        Serial.println("[WiFi] Retrying connection…");
        WiFi.reconnect();
    }
}

// ─────────────────────────────────────────────────────────────
//  HELPER: Serial status printout (every 5 s via timer)
// ─────────────────────────────────────────────────────────────
void printStatus() {
    Serial.println("──────────────────────────────────");
    Serial.printf("  WiFi   : %s\n", wifiConnected     ? "Connected"    : "Offline");
    Serial.printf("  Blynk  : %s\n", Blynk.connected() ? "Connected"    : "Disconnected");
    Serial.printf("  Mode   : Manual only\n");
    Serial.printf("  Buzzer : %s\n", buzzerActive      ? "ON"           : "OFF");
    Serial.printf("  Relays : R1=%s R2=%s R3=%s R4=%s\n",
                  relayState[0] ? "ON" : "OFF",
                  relayState[1] ? "ON" : "OFF",
                  relayState[2] ? "ON" : "OFF",
                  relayState[3] ? "ON" : "OFF");
    Serial.println("──────────────────────────────────");
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);
    Serial.println("\n[Boot] ESP32 Smart Home System starting… (Manual Mode Only)");

    // ── Relay GPIO ────────────────────────────────────────────
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);
    pinMode(RELAY4_PIN, OUTPUT);
    // Active-LOW: drive HIGH so all relays start OFF
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);
    digitalWrite(RELAY3_PIN, HIGH);
    digitalWrite(RELAY4_PIN, HIGH);

    // ── WiFi LED ──────────────────────────────────────────────
    pinMode(WIFI_LED_PIN, OUTPUT);
    digitalWrite(WIFI_LED_PIN, LOW);

    // ── Buzzer: LEDC PWM setup ────────────────────────────────
    ledcAttach(BUZZER_PIN, BUZZER_FREQ_HZ, BUZZER_RESOLUTION);
    ledcWrite(BUZZER_PIN, BUZZER_DUTY_OFF);   // start silent

    // ── ADC for LM35 ─────────────────────────────────────────
    analogSetAttenuation(ADC_11db);   // full-scale ~3.3 V
    analogReadResolution(12);         // 12-bit: 0–4095

    // ── WiFi connection attempt ───────────────────────────────
    Serial.printf("[WiFi] Connecting to '%s'", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        // ── ONLINE: connect to Blynk ──────────────────────────
        wifiConnected = true;
        digitalWrite(WIFI_LED_PIN, HIGH);
        Serial.println("[WiFi] Connected! IP: " + WiFi.localIP().toString());

        Blynk.config(BLYNK_AUTH_TOKEN);
        Blynk.connect(3000);

        if (Blynk.connected()) {
            Serial.println("[Blynk] Connected to server");
        } else {
            Serial.println("[Blynk] Server unreachable – will retry in loop");
        }
    } else {
        // ── OFFLINE: turn all relays ON ───────────────────────
        wifiConnected = false;
        Serial.println("[WiFi] Could not connect – OFFLINE mode");
        Serial.println("[Offline] All relays turned ON");
        setAllRelays(true);
    }

    // ── BlynkTimer registrations ──────────────────────────────

    // Sensor data → Blynk every 1 second (temperature + distance display)
    timer.setInterval(SENSOR_INTERVAL_MS, sendSensorData);

    // Ultrasonic alarm check every 200 ms (responsive buzzer)
    timer.setInterval(ALARM_INTERVAL_MS, checkUltrasonicAlarm);

    // Serial status dump every 5 seconds
    timer.setInterval(5000L, printStatus);

    Serial.println("[Boot] Setup complete");
    printStatus();
}

// ─────────────────────────────────────────────────────────────
//  LOOP
//  Lean and non-blocking. All logic runs inside BlynkTimer
//  callbacks and BLYNK_WRITE() handlers. Relay manual control
//  comes entirely from Blynk virtual pin writes (V0–V3).
// ─────────────────────────────────────────────────────────────
void loop() {
    // Run Blynk comms (handles incoming V-pin writes, heartbeat, etc.)
    if (wifiConnected) {
        Blynk.run();
    }

    // Fire all registered BlynkTimer callbacks on schedule
    timer.run();

    // Non-blocking WiFi / Blynk reconnection check
    handleWiFiReconnect();

    // ── Offline relay guard ───────────────────────────────────
    // If WiFi is lost, keep all relays ON.
    // setRelay()'s state-guard makes this a no-op once already ON.
    if (!wifiConnected) {
        setAllRelays(true);
        return;
    }

    // ── Online: relays are controlled only by BLYNK_WRITE (V0–V3) ──
    // Nothing else belongs in loop().
}
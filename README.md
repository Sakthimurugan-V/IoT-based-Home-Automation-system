# 🏠 ESP32 Smart Home Automation System

An ESP32-based IoT Smart Home Automation system designed to control multiple electrical loads through a 4-channel relay module using the Blynk IoT platform, with temperature monitoring and automatic offline lighting control using an LDR sensor.

---

## 📌 Project Overview

The ESP32 Smart Home Automation System provides remote control of multiple electrical loads through a Blynk IoT dashboard.

The system uses an ESP32 development board as the main controller and a 4-channel relay module to control connected appliances such as lights, fan, and other electrical loads.

When Wi-Fi and Blynk are available, the relays can be controlled remotely through the Blynk application. When Wi-Fi is unavailable, the system continues operating locally using predefined offline control logic.

An LM35 sensor is used for temperature monitoring, while an LDR sensor provides automatic lighting control for Relay 2 during offline operation.

---

## 🎯 Objectives

- Develop an ESP32-based home automation system.
- Control multiple electrical loads using a relay module.
- Enable remote appliance control through Blynk IoT.
- Monitor temperature using an LM35 sensor.
- Provide automatic lighting control during Wi-Fi failure.
- Indicate Wi-Fi connection status using an LED.
- Implement reliable relay control with flicker prevention.
- Demonstrate IoT-based embedded system design.

---

## ✨ Key Features

- ✅ ESP32-based home automation
- ✅ 4-channel relay control
- ✅ Blynk IoT remote control
- ✅ Independent control of four loads
- ✅ LM35 temperature monitoring
- ✅ LDR-based automatic lighting control
- ✅ Offline operation during Wi-Fi failure
- ✅ Wi-Fi status LED indication
- ✅ Relay flicker prevention
- ✅ Non-blocking periodic sensor processing
- ✅ Serial Monitor system diagnostics

---

## 🧠 System Architecture

```text
                  ┌───────────────────────┐
                  │         ESP32         │
                  │    Main Controller    │
                  └───────────┬───────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
     LM35 Sensor          LDR Sensor         Wi-Fi LED
   Temperature Data     Light Detection     Status Indication
                              │
                              │
                              ▼
                     ┌─────────────────┐
                     │  4-Channel      │
                     │  Relay Module   │
                     └───────┬─────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
          Relay 1         Relay 2        Relay 3/4
          Load            Light          Loads
                             │
                             │
                    Offline LDR Control
                             │
                             ▼
                       Automatic Light

ESP32
  │
  ▼
 Wi-Fi
  │
  ▼
Blynk IoT Cloud
  │
  ▼
Mobile / Web Dashboard

🔧 Hardware Components
Component                    	Quantity                 	Purpose
ESP32 Development Board	         1	                Main controller
4-Channel Relay Module	         1	                Electrical load control
LM35 Temperature Sensor	         1	                Temperature measurement
LDR Sensor	                     1	                Ambient light detection
Wi-Fi Status LED	             1	                Wi-Fi connection indication
AC Loads	                As required	            Home appliance control
Power Supply	            As required	            System power
Resistors	                As required	            Sensor/interface circuits
Connecting Wires	        As required	            Hardware connections

📍 Pin Configuration
Component	        Function	          ESP32 GPIO
Relay 1	       Load 1 Control        	 GPIO 15
Relay 2	       Load 2 Control	         GPIO 13
Relay 3	       Load 3 Control	         GPIO 14
Relay 4	       Load 4 Control	         GPIO 27
LM35	   Analog Temperature Input      GPIO 36
LDR	         Analog Light Input	         GPIO 39
LED	        Wifi Status Indicator	     GPIO 4

GPIO Notes

GPIO 36 and GPIO 39 are used as analog input pins.

The ESP32 ADC is configured for 12-bit resolution:

ADC Resolution = 12 bits
ADC Range = 0–4095
🔌 Relay Configuration

The relay module used in this project is configured as active LOW.

GPIO LOW  → Relay ON
GPIO HIGH → Relay OFF

The four relays are used to control different electrical loads.

Example:

Relay 1 → Load 1
Relay 2 → Light
Relay 3 → Load 3
Relay 4 → Load 4

The actual loads can be changed according to the trainer hardware.

🌡️ Temperature Monitoring

An LM35 temperature sensor is connected to the ESP32 analog input.

The LM35 provides an analog voltage proportional to temperature.

The software:

LM35
  ↓
Analog ADC Reading
  ↓
Voltage Calculation
  ↓
Temperature Calculation
  ↓
Temperature in °C
  ↓
Blynk Dashboard

The temperature value is updated periodically and transmitted to Blynk.

💡 LDR-Based Offline Lighting Control

The LDR is used to automatically control Relay 2 when Wi-Fi is unavailable.

The configured threshold is:

#define LDR_THRESHOLD 2000

The operating logic is:

LDR > Threshold
      ↓
Bright
      ↓
Relay 2 OFF

and:

LDR < Threshold
      ↓
Dark
      ↓
Relay 2 ON

The threshold can be adjusted according to the actual LDR circuit and environmental lighting conditions.

📱 Blynk IoT

Blynk IoT is used for remote monitoring and relay control.

The ESP32 communicates with the Blynk Cloud through Wi-Fi.

ESP32
  ↓
Wi-Fi
  ↓
Blynk Cloud
  ↓
Blynk Dashboard
📊 Blynk Virtual Pins
Virtual Pin	   Function       	Direction
V0	        Relay 1 Control   	Blynk → ESP32
V1	        Relay 2 Control	    Blynk → ESP32
V2	        Relay 3 Control	    Blynk → ESP32
V3	        Relay 4 Control	    Blynk → ESP32
V5	          Temperature	      ESP32 → Blynk
V6	          LDR Reading	      ESP32 → Blynk
V7	        Relay 1 Status	    ESP32 → Blynk
V8	        Relay 2 Status	    ESP32 → Blynk
V9	        Relay 3 Status	    ESP32 → Blynk
V10	        Relay 4 Status	    ESP32 → Blynk

🎛️ Blynk Dashboard

Recommended widgets:

Control Widgets
Relay 1 Button → V0
Relay 2 Button → V1
Relay 3 Button → V2
Relay 4 Button → V3

Monitoring Widgets
Temperature Gauge → V5
LDR Gauge → V6
Relay 1 LED → V7
Relay 2 LED → V8
Relay 3 LED → V9
Relay 4 LED → V10

🌐 Operating Modes
Online Mode

When Wi-Fi is connected:

ESP32
  ↓
Wi-Fi Connected
  ↓
Blynk Connected
  ↓
Blynk Controls Relays

All four relays can be controlled from the Blynk dashboard.

Offline Mode

When Wi-Fi is disconnected:

Wi-Fi Disconnected
        ↓
   Offline Mode
        ↓
┌───────┼────────┐
│       │        │
▼       ▼        ▼
R1      R3       R4
ON      ON       ON

Relay 2 is controlled automatically according to the LDR reading:

Bright → Relay 2 OFF
Dark   → Relay 2 ON

This allows the system to continue performing local automation even without an Internet connection.

🔄 System Operating Flow
Power ON
   ↓
ESP32 Initialization
   ↓
Initialize GPIO
   ↓
Initialize ADC
   ↓
Attempt Wi-Fi Connection
   ↓
Wi-Fi Connected?
   ├───────────────┐
   │ YES           │ NO
   ▼               ▼
Blynk Control    Offline Mode
   │               │
   │          R1, R3, R4 → ON
   │               │
   │          LDR controls R2
   │
   ▼
Blynk Controls Relays
   │
   ▼
Monitor Temperature
   │
   ▼
Monitor LDR
   │
   ▼
Update Blynk
   │
   ▼
Continue Monitoring

🛡️ Relay Flicker Prevention

The project implements a relay state-checking mechanism to prevent unnecessary relay switching.

Before changing a relay:

Requested State
      ↓
Compare with Current State
      ↓
Same?
 ┌────┴────┐
YES        NO
 ↓          ↓
No GPIO    Change
Write      Relay State

This reduces unnecessary GPIO switching and helps prevent relay chatter or flickering.

⏱️ Timing and Software Architecture

The project uses periodic timers for sensor and system tasks.

Task	Interval
Temperature/LDR data update	1 second
Offline LDR control	200 ms
Wi-Fi connection check	5 seconds
Serial status information	5 seconds

The main loop remains lightweight and uses:

Blynk.run();
timer.run();

for communication and scheduled tasks.

💻 Software Requirements
Development Environment
Arduino IDE
ESP32 Arduino Core
Blynk IoT Library
Programming Language
C/C++
Required Libraries
WiFi.h
BlynkSimpleEsp32.h

The standard ESP32 Wi-Fi library is included with the ESP32 Arduino board package.

🚀 Installation
1. Install Arduino IDE

Download and install Arduino IDE.

2. Install ESP32 Board Package

Add the ESP32 board package URL to Arduino IDE:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Then install the ESP32 Arduino platform through Boards Manager.

3. Select Board

Select:

ESP32 Arduino
→ ESP32 Dev Module
4. Install Blynk Library

Open:

Sketch
→ Include Library
→ Manage Libraries

Search for:

Blynk

and install the Blynk library.

5. Configure Credentials

Update the following values in the source code:

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
6. Select COM Port

Connect the ESP32 to the computer and select the appropriate COM port.

7. Upload

Compile and upload the firmware to the ESP32.

8. Serial Monitor

Open Serial Monitor at:

115200 baud

to monitor system status and sensor readings.

⚙️ Calibration
LDR Calibration

The LDR threshold can be modified:

#define LDR_THRESHOLD 2000

Measure the LDR ADC value under:

Bright lighting
Normal lighting
Dark conditions

Then select a suitable threshold.

LM35 Calibration

The LM35 conversion is based on its analog output.

The program calculates:

ADC Reading
     ↓
Voltage
     ↓
Temperature

If required, the conversion can be adjusted according to the actual ESP32 ADC characteristics and sensor circuit.

🧪 Testing

Recommended test cases:

Test	                Expected Result
Power ON	            ESP32 starts successfully
Wi-Fi connected	      Wi-Fi LED ON
Wi-Fi disconnected	  Wi-Fi LED OFF
Relay 1 ON	          Load 1 activates
Relay 1 OFF	          Load 1 deactivates
Relay 2 ON	          Load 2 activates
Relay 2 OFF	          Load 2 deactivates
Relay 3 ON	          Load 3 activates
Relay 3 OFF	          Load 3 deactivates
Relay 4 ON	          Load 4 activates
Bright environment	  Relay 2 OFF in offline mode
Dark environment	    Relay 2 ON in offline mode
Temperature measurement	Temperature displayed in Blynk
LDR measurement	      LDR value displayed in Blynk
Wi-Fi failure	        Offline automation continues

⚠️ Safety Precautions

This project can be used to control AC electrical loads through a relay module.

Do not connect AC mains directly to ESP32 GPIO pins.
ESP32 GPIO pins must only control the relay input.
Use a properly rated relay module for the connected load.
Use suitable insulation and electrical protection.
Ensure proper enclosure and wiring for AC connections.
Disconnect power before modifying high-voltage wiring.
Use appropriate fuses and circuit protection.
Keep low-voltage ESP32 circuitry isolated from hazardous mains wiring.
Testing of mains-powered circuits should be performed by a qualified person.
📸 Project Gallery

Add your actual project photographs here.

Complete Home Automation Trainer

Relay Module

ESP32 Controller

Blynk Dashboard

🔮 Future Enhancements
Mobile-based appliance scheduling
Voice-controlled appliances
Energy consumption monitoring
Current and voltage monitoring
Additional environmental sensors
Automatic appliance scheduling
Web-based monitoring
Real-time power consumption analytics
Improved offline automation logic

👨‍💻 Author

Sakthimurugan V

Electronics and Communication Engineering

Areas of Interest
Embedded Systems
IoT
ESP32
Arduino
Sensor Interfacing
Hardware Design
Home Automation
Embedded Programming
📄 License

This project is licensed under the MIT License.

See the LICENSE file for details.

⭐ Support

If you find this project useful, consider giving the repository a ⭐ on GitHub.


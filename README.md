# Smart Mobility Cane 
A smart navigation aid designed to enhance the safety and independence of visually impaired individuals by integrating real-time obstacle detection and automated location tracking.

##  Key Features
*   **Obstacle Detection:** Utilizes an ultrasonic sensor to detect objects within a specific range.
*   **Audiovisual & Haptic Alerts:** Provides dynamic feedback based on object proximity:
    *   `< 40 cm:` Continuous buzzer beep + Vibration motor activated.
    *   `40 cm - 100 cm:` Normal buzzer beep.
    *   `> 100 cm:` System remains in standby.
*   **Real-Time Location Tracking:** Integrates a GPS module to retrieve satellite location data.
*   **Guardian Notifications:** Automatically sends live location coordinates to a guardian's smartphone via the Telegram API every 5 minutes.
*   **User-Friendly Design:** Features single-button operation, lightweight construction, and reflective stickers for external visibility.
*   **Rechargeable Power:** Powered by a 3.7V Li-Ion battery with a Type-C charging module.

##  Components & Hardware
*   **Microcontroller:** ESP32 DevKit (Handles sensor data processing, GPS parsing, and Wi-Fi/Telegram communication).
*   **Sensors & Modules:** 
    *   HC-SR04 Ultrasonic Sensor
    *   GPS Module
*   **Actuators:** Buzzer, Vibration Motor
*   **Power Management:** 
    *   3.7V Li-Ion Battery
    *   MT3608 Boost Converter (Steps up voltage to 5V for components)
    *   Type-C Battery Charging Module
*   **Other:** D400 Transistor (Motor driving), 1k Resistor, Power Switch

##  System Architecture
The system operates on an ESP32 microcontroller powered by a 3.7V Li-Ion battery, stepped up to 5V using an MT3608 boost converter. 
1.  **Sensing:** The HC-SR04 ultrasonic sensor continuously monitors the path ahead.
2.  **Processing & Alerting:** The ESP32 evaluates the distance; if a threshold is breached, it triggers the buzzer directly and the vibration motor via a D400 transistor circuit.
3.  **Connectivity:** Simultaneously, the GPS module tracks satellite coordinates, which the ESP32 transmits over Wi-Fi to a predefined Telegram chat via the Telegram API.

##  Cost Efficiency
The prototype was built with a strong focus on affordability and accessibility, bringing the total estimated component cost to approximately **3,300 LKR**.

##  Contributors
Developed by Team OrionX:
*   Siluni Pasqual
*   Ranuga Edirisinghe
*   Lakshitha Sahabandu

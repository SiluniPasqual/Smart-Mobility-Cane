// === Ultrasonic + Buzzer ===
#define TRIG_PIN 19
#define ECHO_PIN 18
#define BUZZER_PIN 4
#define DISTANCE_THRESHOLD_CM 80
#define VIBRATION_PIN 2  // Change to your actual pin

float lastDistance = 0;

// === GPS + WiFi + Telegram ===
#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>

// Wi-Fi credentials
const char* ssid = "Siluni";
const char* password = "123456789a";

// Telegram credentials
const char* botToken = "7341188142:AAG8kMbb6_9-TEHnvuw-MgfPuUxLck4JmTw";
const int64_t chatID = 848564756;

// Static IP config


// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // RX=16, TX=17

// Web server
WebServer server(80);

// HTTPS client
WiFiClientSecure client;

// Coordinates
double latitude = 0.0;
double longitude = 0.0;

// Telegram live tracking
bool liveLocationSent = false;
int messageID = -1;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000;

// Distance check timing
unsigned long lastDistanceCheck = 0;

// ========== HTML Page ==========
String generateHTML() {
  String html = "<!DOCTYPE html><html><head><title>Live GPS Map</title>";
  html += "<style>body { font-family: Arial; text-align: center; } iframe { width: 90%; height: 500px; border: none; }</style>";
  html += "<script>";
  html += "function updateLocation() {";
  html += "fetch('/location').then(res => res.json()).then(data => {";
  html += "document.getElementById('lat').textContent = data.lat.toFixed(6);";
  html += "document.getElementById('lng').textContent = data.lng.toFixed(6);";
  html += "document.getElementById('map').src = 'https://maps.google.com/maps?q=' + data.lat + ',' + data.lng + '&z=15&output=embed';";
  html += "document.getElementById('lastUpdated').textContent = Math.floor(Date.now() / 1000);";
  html += "});";
  html += "}";
  html += "setInterval(updateLocation, 5000);";
  html += "window.onload = updateLocation;";
  html += "</script>";
  html += "</head><body>";
  html += "<h2>Live GPS Location</h2>";
  html += "<p><strong>Latitude:</strong> <span id='lat'>...</span><br>";
  html += "<strong>Longitude:</strong> <span id='lng'>...</span></p>";
  html += "<iframe id='map' src=''></iframe>";
  html += "<p>Last updated: <span id='lastUpdated'>...</span></p>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", generateHTML());
}

void handleLocation() {
  String json = "{";
  json += "\"lat\":" + String(latitude, 6) + ",";
  json += "\"lng\":" + String(longitude, 6);
  json += "}";
  server.send(200, "application/json", json);
}

int extractMessageID(String response) {
  int idx = response.indexOf("\"message_id\":");
  if (idx == -1) return -1;
  int start = idx + 13;
  int end = response.indexOf(",", start);
  return response.substring(start, end).toInt();
}

void sendInitialLiveLocation(double lat, double lng) {
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Connection failed");
    return;
  }

  String payload = "chat_id=" + String(chatID);
  payload += "&latitude=" + String(lat, 6);
  payload += "&longitude=" + String(lng, 6);
  payload += "&live_period=86400";

  String request = "POST /bot" + String(botToken) + "/sendLocation HTTP/1.1\r\n";
  request += "Host: api.telegram.org\r\n";
  request += "Content-Type: application/x-www-form-urlencoded\r\n";
  request += "Content-Length: " + String(payload.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += payload;

  client.print(request);

  String response = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    response += line + "\n";
    if (line == "\r") break;
  }

  messageID = extractMessageID(response);
  Serial.print("📤 Sent initial live location. message_id = ");
  Serial.println(messageID);
}

void updateLiveLocation(double lat, double lng) {
  if (messageID == -1) return;

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Failed to connect for update");
    return;
  }

  String payload = "chat_id=" + String(chatID);
  payload += "&message_id=" + String(messageID);
  payload += "&latitude=" + String(lat, 6);
  payload += "&longitude=" + String(lng, 6);

  String request = "POST /bot" + String(botToken) + "/editMessageLiveLocation HTTP/1.1\r\n";
  request += "Host: api.telegram.org\r\n";
  request += "Content-Type: application/x-www-form-urlencoded\r\n";
  request += "Content-Length: " + String(payload.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += payload;

  client.print(request);
  Serial.println("Updated live location");
}

float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

// ==================== setup() ====================
void setup() {
  Serial.begin(115200);

  // Ultrasonic + Buzzer
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);


  // GPS
GPS_Serial.begin(9600, SERIAL_8N1, 13, 14);  // RX=14 (D14), TX=13 (D13)


  // WiFi
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Wi-Fi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();

  // Web server
  server.on("/", handleRoot);
  server.on("/location", handleLocation);
  server.begin();
  Serial.println("🌐 Web server started");
}

// ==================== loop() ====================
void loop() {
  // --- GPS updates ---
  while (GPS_Serial.available()) {
    gps.encode(GPS_Serial.read());
  }

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    Serial.print("📍 Lat: ");
    Serial.print(latitude, 6);
    Serial.print(" | Lng: ");
    Serial.println(longitude, 6);

    if (!liveLocationSent && gps.location.isValid()) {
      sendInitialLiveLocation(latitude, longitude);
      liveLocationSent = true;
    }
  }

  if (liveLocationSent && millis() - lastUpdate > updateInterval && gps.location.isValid()) {
    lastUpdate = millis();
    updateLiveLocation(latitude, longitude);
  }

  // --- Ultrasonic check every 200ms ---
if (millis() - lastDistanceCheck > 200) {
  lastDistanceCheck = millis();
  lastDistance = measureDistanceCm();
  Serial.print("Distance: ");
  Serial.print(lastDistance);
  Serial.println(" cm");

  if (lastDistance > 0 && lastDistance < 20) {
    // Close object: warning beep + vibration motor ON
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  } else if (lastDistance >= 20 && lastDistance <= 30) {
    // Moderate distance: normal beep only
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(300);
  } else {
    // Object too far: no alert
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATION_PIN, LOW);
  }
}
}


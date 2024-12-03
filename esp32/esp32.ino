#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFiUdp.h>

// Include AWS credentials from external header file
#include "aws_credentials.h"

// Wi-Fi credentials
Preferences preferences;
char ssid[32] = "";
char password[64] = "";

String roku_ip;
WiFiUDP udp;

// MQTT client setup
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Roku TV details
const int roku_port = 8060;
const int ledPin = 2; // Pin for LED (built-in LED or external)

// Captive portal HTML form
const char* htmlForm = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 WiFi Setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f9;
      color: #333;
      text-align: center;
    }
    h1 {
      margin-top: 20px;
      font-size: 24px;
      color: #444;
    }
    form {
      background-color: #fff;
      max-width: 400px;
      margin: 20px auto;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }
    label {
      font-size: 16px;
      margin-bottom: 10px;
      display: block;
      text-align: left;
    }
    input[type="text"],
    input[type="password"] {
      width: calc(100% - 20px);
      padding: 10px;
      margin: 10px 0 20px 0;
      font-size: 16px;
      border: 1px solid #ccc;
      border-radius: 5px;
      box-sizing: border-box;
    }
    button {
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      padding: 10px 20px;
      cursor: pointer;
      width: 100%;
    }
    button:hover {
      background-color: #45a049;
    }
    @media screen and (max-width: 600px) {
      h1 {
        font-size: 20px;
      }
      form {
        padding: 15px;
      }
      input[type="text"],
      input[type="password"] {
        font-size: 14px;
        padding: 8px;
      }
      button {
        font-size: 14px;
        padding: 8px;
      }
    }
  </style>
</head>
<body>
  <h1>ESP32 WiFi Configuration</h1>
  <form action="/connect" method="post">
    <label for="ssid">WiFi SSID:</label>
    <input type="text" id="ssid" name="ssid" placeholder="Enter your WiFi SSID" required>
    <label for="password">WiFi Password:</label>
    <input type="password" id="password" name="password" placeholder="Enter your WiFi password" required>
    <button type="submit">Connect</button>
  </form>
</body>
</html>
)rawliteral";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

unsigned long previousMillis = 0;
bool ledState = LOW;

void handleConnect() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String inputSSID = server.arg("ssid");
    String inputPassword = server.arg("password");

    inputSSID.toCharArray(ssid, sizeof(ssid));
    inputPassword.toCharArray(password, sizeof(password));

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    server.send(200, "text/html", "Credentials saved! Restarting ESP32 to connect.");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "SSID and Password are required.");
  }
}

void setupAPMode() {
  Serial.println("Starting Access Point mode...");
  WiFi.softAP("ESP32_Setup", "12345678"); // Set AP credentials
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.begin();
  Serial.println("Web server started.");

  pinMode(ledPin, OUTPUT);

  while (true) {
    server.handleClient(); // Handle incoming HTTP requests

    // Non-blocking LED blinking
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      ledState = !ledState; // Toggle LED state
      digitalWrite(ledPin, ledState);
    }
  }
}

String discoverRoku() {
  const char* ssdpRequest = 
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "MX: 1\r\n"
    "ST: roku:ecp\r\n\r\n";

  IPAddress multicastAddress(239, 255, 255, 250); // SSDP multicast address
  const int multicastPort = 1900;

  udp.begin(multicastPort); // Start listening on the multicast port
  udp.beginPacket(multicastAddress, multicastPort);
  udp.write((uint8_t*)ssdpRequest, strlen(ssdpRequest)); // Explicit cast and length
  udp.endPacket();

  unsigned long startTime = millis();
  while (millis() - startTime < 3000) { // Wait up to 3 seconds for a response
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
      char response[512];
      int len = udp.read(response, sizeof(response) - 1);
      response[len] = '\0';

      String responseStr = String(response);
      int locationIndex = responseStr.indexOf("LOCATION: ");
      if (locationIndex != -1) {
        int start = locationIndex + 10; // Skip "LOCATION: "
        int end = responseStr.indexOf("\r\n", start);
        String location = responseStr.substring(start, end);

        // Parse IP address from the location URL
        int ipStart = location.indexOf("//") + 2;
        int ipEnd = location.indexOf(":", ipStart);
        String ipAddress = location.substring(ipStart, ipEnd);
        udp.stop();
        return ipAddress; // Return the Roku IP address
      }
    }
  }

  udp.stop();
  return ""; // Return an empty string if no Roku is found
}

void setupWiFi() {
  pinMode(ledPin, OUTPUT); // Set LED pin as output
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    // Blink rapidly (twice per second) while connecting
    digitalWrite(ledPin, HIGH);
    delay(250);
    digitalWrite(ledPin, LOW);
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(ledPin, HIGH); // Keep LED ON to indicate successful connection
  } else {
    Serial.println("\nFailed to connect. Switching to AP mode...");
    setupAPMode();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* command = doc["command"];
  if (command != nullptr) {
    Serial.print("Parsed command: ");
    Serial.println(command);
    sendECPCommand(command);
  } else {
    Serial.println("Command field not found in JSON payload.");
  }
}

void sendECPCommand(const String& command) {
    if (WiFi.status() == WL_CONNECTED && !roku_ip.isEmpty()) {
        HTTPClient http;
        String url;

        if (command == "Netflix") {
            url = "http://" + roku_ip + ":" + String(roku_port) + "/launch/12";
        } else if (command == "Disney plus") {
            url = "http://" + roku_ip + ":" + String(roku_port) + "/launch/291097";
        } else
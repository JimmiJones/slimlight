/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-relay-module-ac-web-server/

/Modified by J.Case and ChatGPT/
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebSrv.h"

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO true

// Assign GPIO to the relay
const int relayGPIO = 27;

// Replace with your network credentials
const char* ssid = "MYSSID";
const char* password = "MYPASSWORD";

const char* PARAM_INPUT_1 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Slims Light</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    .timer {font-size: 2.0rem;}
  </style>
</head>
<body>
  <h2>Slims Light</h2>
  <h4>Relay - GPIO 27</h4>
  <label class="switch">
    <input type="checkbox" onchange="startRelay(this)" id="startButton" %RELAY_STATE%>
    <span class="slider"></span>
  </label>
  <button onclick="stopRelay()">Stop</button>
  <p class="timer" id="timer">Time Remaining: --:--</p>
  <script>
    var timerInterval;

    function startRelay(element) {
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        xhr.open("GET", "/start?state=1", true);
        timerInterval = setInterval(updateTimer, 1000);
      } else {
        xhr.open("GET", "/stop", true);
        clearInterval(timerInterval);
        document.getElementById("timer").innerText = "Time Remaining: --:--";
      }
      xhr.send();
    }

    function stopRelay() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/stop", true);
      clearInterval(timerInterval);
      document.getElementById("timer").innerText = "Time Remaining: --:--";
      xhr.send();
    }

    function updateTimer() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (xhr.readyState == XMLHttpRequest.DONE) {
          var remainingTime = xhr.responseText;
          document.getElementById("timer").innerText = "Time Remaining: " + remainingTime;
        }
      }
      xhr.open("GET", "/timer", true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

String relayState() {
  return (digitalRead(relayGPIO) == HIGH) ? "" : "checked";
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Set relay pin as output
  pinMode(relayGPIO, OUTPUT);

  // Set relay to off (Normally Open) when the program starts
  digitalWrite(relayGPIO, HIGH);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String relayStateValue = relayState();
    request->send_P(200, "text/html", index_html, [&](const String &var) {
      return (var == "RELAY_STATE") ? relayStateValue : String();
    });
  });

  // Send a GET request to <ESP_IP>/start?state=<inputMessage>
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(relayGPIO, LOW);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/stop
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(relayGPIO, HIGH);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/timer
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
    int remainingTime = 8 * 60; // 8 minutes in seconds
    request->send_P(200, "text/plain", String(remainingTime / 60) + ":" + String(remainingTime % 60));
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing to do here
}


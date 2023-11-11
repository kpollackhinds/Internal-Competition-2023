#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

const char* ssid = "SBRT";
const char* password = "Robotic$3";

// Define motor pins
// const int lmotor_in1 = 12; 
// const int lmotor_in2 = 33;
// const int rmotor_in1 = 13;
// const int rmotor_in2 = 32;

const int lmotor_in1 = 12;
const int lmotor_in2 = 14;
const int rmotor_in1 = 32;
const int rmotor_in2 = 33;

Servo myservo1;
Servo myservo2;
const int servo1pin = 25;
const int servo2pin = 26;

bool up = false;


AsyncWebServer server(80);
WebSocketsServer webSocket(81);

const char indexHtml[]  = R"RAW(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>ESP32 Direction Controller</title>
  </head>
  <body>
    <h1>ESP32 Direction Controller</h1>
    <p>Use WASD keys to control the direction:</p>
    <p>W - Forward, A - Left, S - Backward, D - Right</p>
    <div>
      <figure style="height: 49vh">
        <div id="stream-container" class="image-container">
          <img id="stream" src="" />
        </div>
      </figure>
    </div>

    <script>
      const view = document.getElementById("stream");
      var gateway = `ws://${window.location.hostname}:81`; // WebSocket URL
      var websocket;

      function initWebSocket() {
        console.log("Trying to open a WebSocket connection...");
        websocket = new WebSocket(gateway);
        websocket.onopen = function (evt) {
          console.log("WebSocket connection opened");
        };
        websocket.onclose = function (evt) {
          console.log("WebSocket connection closed");
        };
        websocket.onerror = function (evt) {
          console.log("WebSocket error: " + evt.data);
        };
        websocket.onmessage = function (evt) {
          if (evt.data instanceof Blob) {
            var urlObject = URL.createObjectURL(evt.data);
            view.src = urlObject;
          }
        };
      }

      window.addEventListener("load", initWebSocket);

      function sendCommand(command) {
        if (websocket.readyState === WebSocket.OPEN) {
          websocket.send(command);
        }
      }

      document.addEventListener("keydown", function (event) {
        if (event.key === "w" || event.key === "W" || event.key === "ArrowUp") {
          sendCommand("forward");
        } else if (event.key === "a" || event.key === "A"|| event.key === "ArrowLeft") {
          sendCommand("left");
        } else if (event.key === "s" || event.key === "S"|| event.key === "ArrowDown") {
          sendCommand("backward");
        } else if (event.key === "d" || event.key === "D"|| event.key === "ArrowRight") {
          sendCommand("right");
        }
        else if (event.key === "MediaPlayPause") {
          sendCommand("moveservo");
          }
        console.log(event.key);
      });

      // Event listener for keyup to stop movement
      document.addEventListener("keyup", function (event) {
        if (["w", "W", "ArrowUp", "a", "A", "ArrowLeft", "s", "S","ArrowDown", "d", "D", "ArrowRight", "MediaPlayPause"].includes(event.key)) {
          sendCommand("stop"); 
        }
      });
    </script>
  </body>
</html>
)RAW";

void moveForward() {
  digitalWrite(lmotor_in1, HIGH);
  digitalWrite(lmotor_in2, LOW);
  digitalWrite(rmotor_in1, HIGH);
  digitalWrite(rmotor_in2, LOW);
}

void moveBackward() {
  digitalWrite(lmotor_in1, LOW);
  digitalWrite(lmotor_in2, HIGH);
  digitalWrite(rmotor_in1, LOW);
  digitalWrite(rmotor_in2, HIGH);
}

void turnLeft() {
  digitalWrite(lmotor_in1, LOW);
  digitalWrite(lmotor_in2, LOW);
  digitalWrite(rmotor_in1, HIGH);
  digitalWrite(rmotor_in2, LOW);
}

void turnRight() {
  digitalWrite(lmotor_in1, HIGH);
  digitalWrite(lmotor_in2, LOW);
  digitalWrite(rmotor_in1, LOW);
  digitalWrite(rmotor_in2, LOW);
}

void stopCar() {
  digitalWrite(lmotor_in1, LOW);
  digitalWrite(lmotor_in2, LOW);
  digitalWrite(rmotor_in1, LOW);
  digitalWrite(rmotor_in2, LOW);
}

//assuming that bo
void moveServo(){
  if (up){
    Serial.println("down");
    myservo1.write(10);
    myservo2.write(170);
  }
  else{
    Serial.println("up");
    myservo1.write(50);
    myservo2.write(130);
  }

  up = !up;
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  String message = String((char *)data).substring(0, len);
  Serial.println(message);

  if (message == "forward") {
    moveForward();
  } else if (message == "backward") { 
    moveBackward();
  } else if (message == "left") { 
    turnLeft();
  } else if (message == "right") { 
    turnRight();
  } else if (message == "moveservo") {
    Serial.println("got message");
    moveServo();
  } else if (message == "stop") {
    stopCar();
  }
}

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", client_num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      handleWebSocketMessage(nullptr, payload, length);
      break;
  }
}

void setup() {
  pinMode(lmotor_in1, OUTPUT);
  pinMode(lmotor_in2, OUTPUT);
  pinMode(rmotor_in1, OUTPUT);
  pinMode(rmotor_in2, OUTPUT);

  myservo1.attach(servo1pin);
  myservo2.attach(servo2pin);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  stopCar();

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Camera Ready! Use 'http://" + WiFi.localIP().toString() + "' to connect");

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", indexHtml);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
});


  server.begin();
  Serial.println("HTTP server started");
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started");
}

void loop() {
  webSocket.loop();
}
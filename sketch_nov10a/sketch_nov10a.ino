#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

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
const int rmotot_in2 = 33;

Servo myservo;
const int servopin = 25;

bool up = false;

AsyncWebServer server(80);


String html2 = R"RAW(
<!DOCTYPE html>
<head>
<title>ESP32 Direction Controller</title>
</head>
<body>
<h1>ESP32 Direction Controller</h1>
<p>Use WASD keys to control the direction:</p>
<p>W - Forward, A - Left, S - Backward, D - Right</p>

<script>
// Function to send the request to the ESP32
function sendDirection(direction) {
  // Replace with the IP address of your ESP32
  var ip = location.host;
  var espUrl = 'http://192.168.1.25/control';

  // Assemble the full URL with the direction parameter
  var fullUrl = espUrl + '?dir=' + direction;

  // Send the request
  fetch(fullUrl)
    .then(response => response.text())
    .then(data => console.log(data))
    .catch(error => console.error('Error:', error));
}

// Event listener for keydown to start movement
document.addEventListener('keydown', function(event) {
  if(event.key === 'w' || event.key === 'W') {
    sendDirection('forward');
  } else if(event.key === 'a' || event.key === 'A') {
    sendDirection('left');
  } else if(event.key === 's' || event.key === 'S') {
    sendDirection('backward');
  } else if(event.key === 'd' || event.key === 'D') {
    sendDirection('right');
  }
  else(console.log(event.key);)
});

// Event listener for keyup to stop movement
document.addEventListener('keyup', function(event) {
  if(['w', 'W', 'a', 'A', 's', 'S', 'd', 'D'].includes(event.key)) {
    sendDirection('stop');
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
    myservo.write(10);
  }
  else{
    myservo.write(90);
  }
}

void setup() {
  myservo.attach(servopin);
  pinMode(lmotor_in1, OUTPUT);
  pinMode(lmotor_in2, OUTPUT);
  pinMode(rmotor_in1, OUTPUT);
  pinMode(rmotor_in2, OUTPUT);

  stopCar();  // Ensure car is stopped on startup

  Serial.begin(115200);
  // WiFi.softAP(ssid, password);
  // //   while (WiFi.status() != WL_CONNECTED) {
  // //   delay(1000);
  // //   Serial.println("Connecting to WiFi...");
  // // }
  // Serial.println(WiFi.localIP());
    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", html2);
  });

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    String direction = request->getParam("dir")->value();
    if (direction == "forward") {
      moveForward();
    } else if (direction == "backward") {
      moveBackward();
    } else if (direction == "left") {
      turnLeft();
    } else if (direction == "right") {
      turnRight();
    } else if (direction == "stop") {
      stopCar();
    }
    request->send(200);
  });

  server.begin();
}

void loop() {
  
  
}


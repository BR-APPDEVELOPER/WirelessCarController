#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

//network credentials
const char *ssid = "realme 2 Pro";
const char *password = "BoopathyRaj8D";

//AsyncWebServer object on port 80
AsyncWebServer server(80);

//define pins for 4 motors
const int leftPairForward_pin = 0;
const int leftPairBackward_pin = 4;
const int rightPairForward_pin = 16;
const int rightPairBackward_pin = 17;

//for ultrasonuc sensor
const int ultrasonicTrig_pin = 5;
const int ultrasonicEcho_pin = 18;
const int builtInLed = 2;

const int vccForRgbLed = 23;
#define SOUND_SPEED 0.034
#define LED_PIN 22
#define NUM_LEDS 8
#define BRIGHTNESS 50
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
long duration;
float distanceCm;


//decalre variable for to check weather the motor is on or off conditionLeft
bool forward_check = false;
bool backward_check = false;
bool left_check = false;
bool right_check = false;
bool emergencyStop = false;
bool startCar = false;
bool conditionLeft = false;
bool conditionRight = false;

void setup() {
  Serial.begin(115200);
  //decalre pins for I/O
  pinMode(leftPairForward_pin, OUTPUT);
  pinMode(leftPairBackward_pin, OUTPUT);
  pinMode(rightPairForward_pin, OUTPUT);
  pinMode(rightPairBackward_pin, OUTPUT);

  pinMode(ultrasonicTrig_pin, OUTPUT);
  pinMode(ultrasonicEcho_pin, INPUT);
  pinMode(builtInLed, OUTPUT);
  pinMode(vccForRgbLed, OUTPUT);

  //by default all motor in stop state
  //stopRunningAllMotor();
  digitalWrite(leftPairForward_pin, LOW);
  digitalWrite(leftPairBackward_pin, LOW);
  digitalWrite(rightPairForward_pin, LOW);
  digitalWrite(rightPairBackward_pin, LOW);
  digitalWrite(vccForRgbLed, HIGH);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //connect to the wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi");
  }

  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  /*// Route to handle HTTP POST requests
  // Route to handle HTTP POST requests
  server.on("/startcar", HTTP_POST, [](AsyncWebServerRequest *request){
    int params = request->params();
    String message = "";
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        Serial.print("POST parameter: ");
        Serial.print(p->name());
        Serial.print(" = ");
        Serial.println(p->value());
        message += p->name() + " = " + p->value() + "\n";
      }
    }
    request->send(200, "text/plain", "Data received:\n" + message);
  });

  server.on("/startcar", HTTP_POST, [](AsyncWebServerRequest *request) {
    String key;
    if (request->hasParam("data", true)) {
      key = request->getParam("data", true)->value();
      if (key == "start") startCar = true;
      else startCar = false;
      Serial.println("Received data: " + key);
    } else {
      startCar = false;
      key = "No message sent";
      Serial.println("No data received");
    }
    request->send(200, "text/plain", "Received data: " + key);
  });*/

  //For all motors to rotate in forward direction
  server.on("/motor/forward", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (checkEmergencyStop(request)) return;  // check for emergency stop is active ot not

    if (!forward_check) {  //All 4 Motor will run in Forward direction
      digitalWrite(leftPairForward_pin, HIGH);
      digitalWrite(leftPairBackward_pin, LOW);
      digitalWrite(rightPairForward_pin, HIGH);
      digitalWrite(rightPairBackward_pin, LOW);
      request->send(200, "text/plain", "Motor is running Forward.");
      forward_check = true;

    } else {  //All 4 Motor will not run in any direction
      stopRunningAllMotor();
      request->send(200, "text/plain", "Motor is not running.");
      forward_check = false;
    }
  });

  //For all motors to rotate in backward direction
  server.on("/motor/backward", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (checkEmergencyStop(request)) return;  // check for emergency stop is active ot not

    if (!backward_check) {  //All 4 Motor will run in backward direction
      digitalWrite(leftPairForward_pin, LOW);
      digitalWrite(leftPairBackward_pin, HIGH);
      digitalWrite(rightPairForward_pin, LOW);
      digitalWrite(rightPairBackward_pin, HIGH);
      dimBrightRedColor(200, 0, 0);  //bright red color
      request->send(200, "text/plain", "Motor is running Backward.");
      backward_check = true;

    } else {  //All 4 Motor will not run in any direction
      stopRunningAllMotor();
      dimBrightRedColor(50, 0, 0);
      request->send(200, "text/plain", "Motor is not running.");
      backward_check = false;
    }
  });


  //To trun the car direction to left side
  server.on("/motor/turnleft", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (checkEmergencyStop(request)) return;  // check for emergency stop is active ot not

    if (!left_check) {  //left pair Motor will run in backward direction and right pair Motor will run in forward direction
      digitalWrite(leftPairForward_pin, LOW);
      digitalWrite(rightPairBackward_pin, LOW);
      digitalWrite(leftPairBackward_pin, HIGH);
      digitalWrite(rightPairForward_pin, HIGH);

      conditionLeft = true;

      request->send(200, "text/plain", "Motor is turning left side.");
      left_check = true;

    } else {  //All 4 Motor will not run in any direction
      stopRunningAllMotor();
      dimBrightRedColor(50, 0, 0);
      conditionLeft = false;
      request->send(200, "text/plain", "Motor is not turning.");
      left_check = false;
    }
  });

  //To trun the car direction to right side
  server.on("/motor/turnright", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (checkEmergencyStop(request)) return;  // check for emergency stop is active ot not

    if (!right_check) {  //left pair Motor will run in forward direction and right pair Motor will run in backward direction
      digitalWrite(leftPairBackward_pin, LOW);
      digitalWrite(rightPairForward_pin, LOW);
      digitalWrite(leftPairForward_pin, HIGH);
      digitalWrite(rightPairBackward_pin, HIGH);
      conditionRight = true;
      request->send(200, "text/plain", "Motor is turning right side.");
      right_check = true;

    } else {  //All 4 Motor will not run in any direction
      stopRunningAllMotor();
      dimBrightRedColor(50, 0, 0);
      conditionRight = false;
      request->send(200, "text/plain", "Motor is not turning.");
      right_check = false;
    }
  });

  //to turn on/off the car front light
  server.on("/motor/frontlight/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(builtInLed, HIGH);
    request->send(200, "text/plain", "Car front light is On");
  });

  server.on("/motor/frontlight/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(builtInLed, LOW);
    request->send(200, "text/plain", "Car front light is Off");
  });
  server.begin();
}

void loop() {
  //clears the trig pin
  digitalWrite(ultrasonicTrig_pin, LOW);
  delayMicroseconds(2);

  //sets the trig pin on HIGH state for 10 micro seconds
  digitalWrite(ultrasonicTrig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrig_pin, LOW);

  //reads the echo pin returns the sound wave travel time travel time in microseconds
  duration = pulseIn(ultrasonicEcho_pin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  if (distanceCm < 30) {
    stopRunningAllMotor();
    emergencyStop = true;
    forward_check = false;
    backward_check = false;
    left_check = false;
    right_check = false;
  } else {
    emergencyStop = false;
  }
  Serial.println("Distance in CM: " + String(distanceCm));
  if (conditionLeft) leftIndicater();
  else if (conditionRight) rightIndicater();
  delay(1000);
}

void stopRunningAllMotor() {
  digitalWrite(leftPairForward_pin, LOW);
  digitalWrite(leftPairBackward_pin, LOW);
  digitalWrite(rightPairForward_pin, LOW);
  digitalWrite(rightPairBackward_pin, LOW);
}

bool checkEmergencyStop(AsyncWebServerRequest *request) {
  if (emergencyStop) {
    request->send(200, "text/plain", "Emergency stop is enabled, Motor cannot run.");
    return true;
  }
  return false;
}

void dimBrightRedColor(int R, int G, int B) {
  CRGB color = CRGB(R, G, B);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void leftIndicater() {
  CRGB colorOff = CRGB(0, 0, 0);
  CRGB color = CRGB(255, 69, 0);
  //while (conditionLeft){
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i > 1) {
      leds[i] = colorOff;
    } else {
      leds[i] = color;
    }
  }
  FastLED.show();
  delay(150);
  for (int i = 0; i < 2; i++) {
    leds[i] = colorOff;
  }
  FastLED.show();

  //}
}

void rightIndicater() {
  CRGB colorOff = CRGB(0, 0, 0);
  CRGB color = CRGB(255, 69, 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < 6) {
      leds[i] = colorOff;
    } else {
      leds[i] = color;
    }
  }
  FastLED.show();
  delay(150);
  for (int i = 6; i < 8 ; i++) {
    leds[i] = colorOff;
  }
  FastLED.show();
}
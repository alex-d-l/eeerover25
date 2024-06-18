#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>
#include <Arduino.h>

// SSID & Password so Rover can connect to WiFi/Hotspot
const char ssid[] = "Alex";
const char pass[] = "alex4palace";

// Constant Group number to make IP address constant (Only for EEERover network)
const int groupNumber = 25; 

// Global Variables:
int currentSpeed = 0; // this variable determines the speed of the rover's movement
float null_point = 1.45; // this variable is used in the magnetic code

// Variables to store the data that will be sent to the webpage
String receivedName = ""; 
String receivedPolarity = "";
String receivedInfraredFreq = "";
String receivedRadioFreq = "";
String receivedSpecies = "";

// Variables to store time measurements of rising edges to calculate frequency
volatile unsigned long lastRisingEdgeTimeRadio = 0; 
volatile unsigned long currentRisingEdgeTimeRadio = 0;
volatile unsigned long edgeCountRadio = 0;
float frequencyRadio = 0;

volatile unsigned long lastRisingEdgeTimeInfra = 0;
volatile unsigned long currentRisingEdgeTimeInfra = 0;
volatile unsigned long edgeCountInfra = 0;
float frequencyInfra = 0;

// Creates web server instance to listen to HTTP requests on port 80
WiFiWebServer server(80);

// Function declarations
void handleRoot();
void handleNotFound();
void handleCommand();
void handleConsolidatedData();

void readName();
bool isLetter(char c);
void readPolarity();
void readInfrared();
void handleRisingEdgeInfra();
void readRadio();
void handleRisingEdge();
void species();


void setup() {
  // N.B. Pins 5, 7, 10 used by WiFi Shield

  // Movement and Speed Pins:
  pinMode(2, OUTPUT); // Direction: right
  pinMode(3, OUTPUT); // Direction: left

  pinMode(6, OUTPUT); // Speed: right
  pinMode(4, OUTPUT); // Speed: left

  // Analogue Pin for Polarity
  pinMode(A2, INPUT);

  // Digital Pin for Infrared
  pinMode(9, INPUT);

  // Digital Pin for Radio
  pinMode(8, INPUT);

  Serial.begin(9600);

  // Wait 10s for the serial connection before proceeding
  // This ensures you can see messages from startup on the monitor
  // Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);  

  Serial.println(F("\nStarting Web Server"));

  // Check WiFi shield is present
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    while (true);
  }

  // Attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  // Register the callbacks to respond to HTTP requests
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.on("/consolidated", handleConsolidatedData);

  server.onNotFound(handleNotFound);
  
  server.begin();

  Serial.print(F("HTTP server started @ "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));

  // Initialize the UART port to receive the signal at 600 bps
  Serial1.begin(600);
  Serial.println("UART Connection is ready!");
  
  attachInterrupt(digitalPinToInterrupt(8), handleRisingEdge, RISING);
  attachInterrupt(digitalPinToInterrupt(9), handleRisingEdgeInfra, RISING);
}


// Call the server polling function in the main loop
void loop() {
  server.handleClient();
  readName();
  readPolarity();
  readInfrared();
  readRadio();
  species();
}


void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  // 172.20.10.4 is mobile hotspot ip - change this accordingly
  server.sendHeader("Location", "http://172.20.10.4/index.html", true);
  server.send(302, "text/plain", "");
}


// Handles the Movement & Speed Commands from Webpage
void handleCommand() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  String cmd = server.arg("cmd");

  // For debugging:
  Serial.println("Received Command: '" + cmd + "'");

  if(cmd.startsWith("SPEED")) {
    int speed = cmd.substring(5).toInt();
    currentSpeed = speed * 2.55;
  }

  else if(cmd == "F") {
    digitalWrite(2, HIGH); // Right ON
    digitalWrite(3, HIGH); // Left ON

    analogWrite(6, currentSpeed); // Right
    analogWrite(4, currentSpeed); // Left
  }
  
  else if(cmd == "B") {
    digitalWrite(2, LOW); // Right OFF
    digitalWrite(3, LOW); // Left OFF

    analogWrite(6, currentSpeed); // Right
    analogWrite(4, currentSpeed); // Left
  }

  else if(cmd == "R") {
    digitalWrite(2, LOW); // Right OFF
    digitalWrite(3, HIGH); // Left ON

    analogWrite(6, currentSpeed); // Right
    analogWrite(4, currentSpeed); // Left
  }

  else if(cmd == "L") {
    digitalWrite(2, HIGH); // Right ON
    digitalWrite(3, LOW); // Left OFF

    analogWrite(6, currentSpeed); // Right
    analogWrite(4, currentSpeed); // Left
  }

  else if(cmd == "S") {
    analogWrite(6, 0); // Right
    analogWrite(4, 0); // Left
  }

  else if(cmd == "C") {
    float temp = 0;
    for(int i = 0; i < 20; i++) {
      temp = analogRead(A2);
      null_point += temp;
      delay(2);
    }
    null_point = 3.3 * null_point / 1023;
    null_point = null_point / 20;
    Serial.print("NULL point: ");
    Serial.println(null_point);
  }

  server.send(200, F("text/plain"), cmd);

  // For debugging:
  Serial.println("Sent response to server.");

}

// Generate a 404 response with details of the failed request
void handleNotFound() {
  String message = F("File Not Found\n\n"); 
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, F("text/plain"), message);
}


void handleConsolidatedData() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  String response = "{";
  response += "\"name\": \"" + receivedName + "\", ";
  response += "\"infrared\": \"" + receivedInfraredFreq + "\", ";
  response += "\"radio\": \"" + receivedRadioFreq + "\", ";
  response += "\"polarity\": \"" + receivedPolarity + "\", ";
  response += "\"species\": \"" + receivedSpecies + "\"";
  response += "}";

  server.send(200, "application/json", response);
}


void readName() {
  static String name_received = "";
  static bool receivingName = false;

  while (Serial1.available() > 0) {
    // for debugging:
    Serial.println("Serial1 is available!");

    char incomingChar = Serial1.read();
    if (incomingChar == '#') {
      receivingName = true;
      name_received = "#";
    } 
    else if (receivingName && isLetter(incomingChar)) {
      name_received += incomingChar;
      if (name_received.length() == 4) {
        receivingName = false;
        receivedName = name_received;
      }
    }
  }
  // for debugging:
  Serial.println("Received Name: " + receivedName);
}



bool isLetter(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); // Checks if the character is within the range of alphabet in HEX
}


void readPolarity() {
  // float null_point = 1.45; // might need calibration (POLARITY)
  float displacement = 0.03; // this value will need calibration as well (POLARITY)
  float mag_read = (3.3 * analogRead(A2)) / 1023;
  Serial.println(mag_read);

  if(mag_read > null_point + displacement) {
    receivedPolarity = "North";
  }

  else if(mag_read < null_point - displacement) {
    receivedPolarity = "South";
  }

  else {
    receivedPolarity = "Not Detected";
  }

  // for debugging:
  Serial.println("Received Polarity: " + receivedPolarity);
}


void handleRisingEdge() {
  lastRisingEdgeTimeRadio = currentRisingEdgeTimeRadio;
  currentRisingEdgeTimeRadio = micros();
  edgeCountRadio++;
}


void readRadio() {
  static unsigned long lastAttemptTimeRadio = 0;
  const unsigned long timeout = 50; // 50 ms timeout  

  if (millis() - lastAttemptTimeRadio >= timeout) {
    if (edgeCountRadio >= 10) {
      unsigned long period = currentRisingEdgeTimeRadio - lastRisingEdgeTimeRadio;
      frequencyRadio = 1000000.0 / period;
      if((frequencyRadio > 95) && (frequencyRadio < 145)) {
        receivedRadioFreq = "120 Hz";
      } 
      else if((frequencyRadio > 175) && (frequencyRadio < 225)) {
        receivedRadioFreq = "200 Hz";
      } 
      else {
        // For debugging
        receivedRadioFreq = String(frequencyRadio) + " - Not Within Range"; 
      }
      edgeCountRadio = 0;
    } 
    else {
      receivedRadioFreq = "Not Detected";
    }

    // Reset the timing for the next measurement
    lastAttemptTimeRadio = millis();

    // For debugging
    Serial.println("Received Radio: " + receivedRadioFreq);
  }
}


void handleRisingEdgeInfra() {
  lastRisingEdgeTimeInfra = currentRisingEdgeTimeInfra;
  currentRisingEdgeTimeInfra = micros();
  edgeCountInfra++;
}


void readInfrared() {
  static unsigned long lastAttemptTimeInfra = 0;
  const unsigned long timeout = 50; // 50 ms timeout

  if (millis() - lastAttemptTimeInfra >= timeout) {
    if (edgeCountInfra >= 10) {
      unsigned long period = currentRisingEdgeTimeInfra - lastRisingEdgeTimeInfra;
      frequencyInfra = 1000000.0 / period;
      if((frequencyInfra > 328) && (frequencyInfra < 378)) {
        receivedInfraredFreq = "353 Hz";
      } 
      else if((frequencyInfra > 546) && (frequencyInfra < 596)) {
        receivedInfraredFreq = "571 Hz";
      } 
      else {
        // For debugging
        receivedInfraredFreq = String(frequencyInfra) + " - Not Within Range"; 
      }
      edgeCountInfra = 0;
    } 
    else {
      receivedInfraredFreq = "Not Detected";
    }

    // Reset the timing for the next measurement
    lastAttemptTimeInfra = millis();

    // For debugging
    Serial.println("Received Infrared: " + receivedInfraredFreq);
  }
}

void species() {
  if((receivedInfraredFreq == "571 Hz") && (receivedPolarity == "North")) {
    receivedSpecies = "Abronia";
  }
  else if((receivedInfraredFreq == "353 Hz") && (receivedPolarity == "South")) {
    receivedSpecies = "Dixonius";
  }
  else if((receivedRadioFreq == "120 Hz") && (receivedPolarity == "North")) {
    receivedSpecies = "Elgaria";
  }
  else if((receivedRadioFreq == "200 Hz") && (receivedPolarity == "South")) {
    receivedSpecies = "Cophotis";
  }
  else {
    receivedSpecies = "Not Detected";
  }
  Serial.println(receivedSpecies);
}
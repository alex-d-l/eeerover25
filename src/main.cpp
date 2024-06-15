#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>
#include <Arduino.h>

// SSID & Password so Rover can connect to WiFi/Hotspot
const char ssid[] = "Alex";
const char pass[] = "alex4palace";

// Set your group number to make the IP address constant - only do this on the EEERover network
const int groupNumber = 25; 

// Global Variables:
int currentSpeed = 0;
float null_point;
String receivedName = "";
String receivedPolarity = "";
String receivedInfraredFreq = "";
String receivedRadioFreq = "";

WiFiWebServer server(80);

bool isLetter(char c);

void handleRoot();
void handleNotFound();
void handleCommand();
void handleConsolidatedData();

void readName();
void readPolarity();
void readInfrared();
void readRadio();
void handleRisingEdge();

// Variables to store time measurements
volatile unsigned long lastRisingEdgeTimeRadio = 0;
volatile unsigned long currentRisingEdgeTimeRadio = 0;
volatile unsigned long edgeCountRadio = 0;
float frequencyRadio = 0;

unsigned long lastRisingEdgeTimeInfrared = 0;
unsigned long currentRisingEdgeTimeInfrared = 0;
unsigned long edgeCountInfrared = 0;
float frequencyInfrared = 0;

int threshold;  // Initial threshold value
unsigned long lastThresholdUpdateTime = 0;
const unsigned long thresholdUpdateInterval = 1000;  // Update threshold every 1000 ms


void setup() {
  // N.B. Pins 5, 7, 10 used by WiFi Shield

  // Movement and Speed Pins:
  pinMode(2, OUTPUT); // Direction: right
  pinMode(3, OUTPUT); // Direction: left

  pinMode(6, OUTPUT); // Speed: right
  pinMode(4, OUTPUT); // Speed: left

  // Analogue Pin for Polarity
  pinMode(A2, INPUT);

  // Analogue Pin for Infrared
  pinMode(A1, INPUT);

  // Digital Pin for Radio
  pinMode(8, INPUT);

  // Initial Setup values
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  analogWrite(6, 0);
  analogWrite(4, 0);

  Serial.begin(9600);

  float temp = 0;
  for(int i = 0; i < 10; i++) {
    temp = (3.3 * analogRead(A2)) / 1023;
    null_point += temp;
    delay(5);
  }
  null_point = null_point / 10;
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

  // Configure the static IP address if group number is set
  // if (groupNumber)
  //   WiFi.config(IPAddress(192,168,0,groupNumber+1));

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

}


// Call the server polling function in the main loop
void loop() {
  server.handleClient();
  readName();
  readPolarity();
  readInfrared();
  readRadio();
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
  response += "\"polarity\": \"" + receivedPolarity + "\"";
  response += "}";

  server.send(200, "application/json", response);
}


void readName() {
  static String name_received = "";
  static bool receivingName = false;

  while (Serial1.available() > 0) {
    // for debugging:
    Serial.println("Serial1 is available! (readName())");

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
  //float null_point = 1.70; // might need calibration (POLARITY)
  float displacement = 0.05; // this value will need calibration as well (POLARITY)
  
  float mag_read = (3.3 * analogRead(A2)) / 1023;

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
      if (((frequencyRadio > 95) && (frequencyRadio < 145)) || ((frequencyRadio > 175) && (frequencyRadio < 225))) {
        receivedRadioFreq = String(frequencyRadio) + " Hz";
      } 
      else {
        // For debugging
        receivedRadioFreq = String(frequencyRadio) + " Hz"; 
        // receivedRadioFreq = "Not Detected (Not within range)"; // UNCOMMENT
      }
      edgeCountRadio = 0;
    } 
    else {
      receivedRadioFreq = "Not Detected (Not enough rising edges)";
    }

    // Reset the timing for the next measurement
    lastAttemptTimeRadio = millis();

    // For debugging
    Serial.println("Received Radio: " + receivedRadioFreq);
  }
}


// TEST WITH DIGITAL PIN
// void readInfrared() {
//   unsigned long startTime = 0;
//   unsigned long currentTime = 0;

//   // Measure max and min - this way we find amplitude and can find threshold
//   unsigned long start = millis();
//   int maxRead = 0;
//   int minRead = 1023;
//   int currentRead;

//   // Measure for 10 milliseconds
//   while (millis() - start < 10) {
//     currentRead = analogRead(A1);
//     if (currentRead > maxRead) {
//       maxRead = currentRead;
//     }
//     if (currentRead < minRead) {
//       minRead = currentRead;
//     }
//   }

//   int threshold = minRead + (maxRead - minRead) / 2; // Average to find midpoint

//   Serial.print("Threshold: ");
//   Serial.print(threshold);

//   // Wait for the first rising edge
//   while (analogRead(A1) < threshold);
//   startTime = micros();

//   // Initialize variables to store the time intervals
//   unsigned long intervals[5];
//   int count = 0;

//   while (count < 5) {
//     // Wait for the signal to go above the threshold
//     while (analogRead(A1) < threshold);
//     // Record the current time when the signal rises
//     currentTime = micros();
//     // Store the interval
//     intervals[count] = currentTime - startTime;
//     // Update the start time
//     startTime = currentTime;
//     count++;
//   }

//   // Calculate the average period
//   unsigned long totalPeriod = 0;
//   for (int i = 0; i < 5; i++) {
//     totalPeriod += intervals[i];
//   }
//   unsigned long averagePeriod = totalPeriod / 5;
//   float frequency = 1000000.0 / averagePeriod;

//   // Print the frequency
//   Serial.print(", Frequency: ");
//   Serial.print(frequency);
//   Serial.println(" Hz");
// }

void readInfrared() {
  unsigned long startTime = 0;
  unsigned long currentTime = 0;

  // Measure max and min  - this way we find amplitude and can find threshold
  unsigned long start = millis();
  int maxRead = 0;
  int minRead = 1023;
  int currentRead;

  // Measure for 100 milliseconds
  while (millis() - start < 50) {
    currentRead = analogRead(A1);
    if (currentRead > maxRead) {
      maxRead = currentRead;
    }
    if (currentRead < minRead) {
      minRead = currentRead;
    }
  }

  int threshold = minRead + (maxRead - minRead) / 2; // Average to find midpoint

  // Wait for the first rising edge
  while (analogRead(A1) < threshold);
  startTime = micros();

  // wait for 10 waves
  //might be bugs due to not leaving the loop
  if(threshold > 50){//if statement might fix this bug
    int count = 0;
    while (count < 5) {
      while (analogRead(A1) >= threshold); // Wait for the signal to drop below the threshold
      while (analogRead(A1) < threshold);  // Wait for the signal to go above the threshold
      count++;
    }
  }
  currentTime = micros();

  // Calculate the average period of one wavelength
  unsigned long period = (currentTime - startTime) / 5;
  float frequency = 1000000.0 / period;

  // Print the frequency
  Serial.print("Infrared: ");
  Serial.println(frequency);

  receivedInfraredFreq = String(frequency);

  // can print only when the output is in range - for example 350 - 400 hz and 550 to 600 HZ
}
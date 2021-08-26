
/*
 * This is the Arduino Sketch for the SmartRelief project. It hosts a webserver and controls a RGB Matrix based on the input from the websocket from a connected client
 * A lot of this code was inspired by https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
 * 
 * @author Marco Hirsbrunner
 * @version 1.0
 * @license MIT
 */



 /*------------- Load libraries --------------- */
#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>

/*------------------- Defines --------------- */
#define USE_SERIAL Serial
#define pinMatrix D6

/* ---------------------------------------------------------------------------------------------------------------
 * ----------------------------------------- Benutzerdefinierte Werte --------------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/
const int nLED = 64; // Wie viele LEDs sind vorhanden (Anzahl Matrixen x 2)
const int maxNLED = 64; // Wie viele LEDs könne gleichzeitig angeschalten sein
const int LEDBrightness = 250; // Wie hell sind die LED (von 0 bis 255)
unsigned long timerInterval = 60 * 1000; //Nach wie langer inaktiver Zeit die LED abgeschaltet werden (*1000 da in Millisekunden -> 60 * 1000 = 60 Sekunden)
const char *ssid = "SmartRelief"; // Der Name des Netzwerkes
const char *password = "";   // Das Passwort für das Netzwerk (leer -> "" für kein Passwort)



/* ---------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------Global variables --------------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/
bool LEDState[nLED]; //Saves wheter an LED is turned on or off (true for on, false for off)
/*
 * Saves the last time this LED was turned on. If only a limited number of LEDs may be turned on at the same time,
 * the application will start by turning off the ones who have been on the longest. This lookup is done
 * via this array.
 */
unsigned long LEDTime[nLED]; //Saves last time the LED was turned on

unsigned long lastCall = 0; //Saves the last time something happened, is needed if the LEDs should turn off after nothing happens
boolean TimerBasedOff = false; //Saves wheter the LED got turned off because of timer (So it doesnt keep turning them off)


/* ------------------- Webserver and Wifi variables -------------- */
ESP8266WiFiMulti wifiMulti;

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81

const char* mdnsName = "smartrelief";


/* ------------------- Strip variable --------------- */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(nLED, pinMatrix, NEO_GRB + NEO_KHZ800); 


/*
 * Gets executed once on startup
 */
void setup() {
  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10); 
  Serial.println("\r\n");
  delay(2000); // wait for everything to get ready
      WiFi.disconnect(); //disconnect from the current Network, program not made for multiple network connections
  strip.begin();
  strip.show(); // set all pixels to 'off'
  strip.setBrightness(LEDBrightness);

  startWiFi();                 // Start a Wi-Fi access point

  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server

  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  Serial.println("Finished Setup");
}


/*
 * Keeps looping while the device is running
 */
void loop() {
  webSocket.loop();
  server.handleClient();
  checkTimer();
}

/* ---------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------- Websocket Handlers --------------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/ 
 void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED: // When disconnected, just log it
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: { // When new client connected, log it and send them the current LED status.
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        String json = parseLedDataToJson();
        
        webSocket.sendTXT(num, json);
      }
      break;
    case WStype_TEXT: //When recieving new data parse it, adjust LEDs, send Update to Client.
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String LED = String((char *) &payload[0]);
      if(LED[0] != 'u') { // Clients will ask for an update every 3 seconds with simply a u
        turnOn(LED); //call function to turn on the requested LED. Example for LED String: "255,255,255,5,6,7,8,9". the first 3 Values define the Color of the LED
      }
      // After adjusting LEDs or simply for updating the client, parse the LED data and send it via the websocket.
      String json = parseLedDataToJson();
      webSocket.sendTXT(num, json);
      break;
  }
}

/*
 * Send the actual data about the LED to the all WebSockets (In this case Clients connected via. Javascript websocket)
 */
void resendDataForWebsocket(){
   // String json = parseLedDataToJson();
   // For some reason this crashes the webserver, i have not been able to figure out why this happens so far.
   // It does not create an endless loop of sending and recieving, so i don't know why it crashes.
   // What is not possible like this right now, is that data gets live updated on other devices
   // webSocket.broadcastTXT(json);
}

/*
 * Converts the LEDStates to a JSON Object to send via the websocket.
 */
String parseLedDataToJson() {
  String json = "{ ";
  for(int i = 0; i<nLED; i++){
    json += "\"" + String(i) + "\"";
    json += ":";
    json += "\"" + String(LEDState[i]) + "\"";
    if(!(i == nLED -1)){ json += ",";}
  }
  json += "}";
  Serial.print("Sending data to client: ");
  Serial.println(json);
  return json;
}

/* ---------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------- LED Helper functions ---------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/ 

 /*
 * checks wheter all LED should be turned off based on the timer Interval
 */
void checkTimer() {
  if (!TimerBasedOff) { //if it is not already off because of the timer
    if (lastCall + timerInterval < millis()) { //if the timerInterval has passed since the last action then turn off (millis() returns current time)
      Serial.print("Timer based turn off, no action since ");
      Serial.print(timerInterval / 1000);
      Serial.println(" seconds");
      turnOffAllLed();
      TimerBasedOff = true;
    }
  }
}

/*
 * turns off all LED
 */
void turnOffAllLed() {
  Serial.println("Turning all LED off");
  for (int i = 0; i < nLED ; i++) {
    turnOnLed(0, 0, 0, i);
  }
  strip.show();
}

/*
 * counts the amount of LED turned on
 */
int nLEDOn() {
  int counter = 0;
  for (int i = 0; i < sizeof(LEDState); i++) {
    if (LEDState[i] != NULL && LEDState[i]) {
      counter++;
    }
  }
  return counter;
}

/*
 * turns on single led (used in ledOn() )
 * 
 * Parameters: 
 *  int r: red Value (0-255)
 *  int g: green Value (0-255)
 *  int b: blue Value (0-255)
 *  int led: led to be turned on
 */
void turnOnLed(int r, int g, int b, int led) {
  if (r == 0 && b == 0 && g == 0) {
    Serial.print("Turning off LED");
    Serial.println(led);
    LEDState[led] = false;
    strip.setPixelColor(led, strip.Color(0, 0, 0));
  }
  else {
    if (nLEDOn() <= maxNLED) { // check wheter the amount of active LEDs exceeds the allowed maximum
      Serial.print("The counter equals: ");
      Serial.println(nLEDOn());
      Serial.println("accepted To turn on LED");
      Serial.print("Turning on LED: ");
      Serial.println(led);
      LEDState[led] = true;
      LEDTime[led] = millis();
      strip.setPixelColor(led, strip.Color(r, g, b)); //set the Color to the first 3 number that were sent
    }
    else {
      Serial.println("Turning off oldest LED because there are too many on");
      turnOnLed(0, 0, 0, getOldestLED());
      turnOnLed(r, g, b, led);
    }
  }
}

/*
 * handels the given LED String, turns on, off. 
 * 
 * parameters:
 *  String LED: Infos about LED in format: " redValue as int, greenValue as int, blueValue as int, firstLED, secondLED" -> requires at least 4 entries (3 for colors)
 */
void turnOn(String LED) {
  Serial.print("LED String: ");
  Serial.println(LED);
  TimerBasedOff = false;
  lastCall = millis();
  int nPositionLED = 0;
  if (LED[0] == 'x') { //if the Websocket recievs a "x" from the JS then the client requests to close the AP
    WiFi.softAPdisconnect(true);
  }
  else {
    /*
     * Parse the recieved comma seperated value string into the values.
     */
    int ledArrayLength = countArgs(LED);
    int ledArray[ledArrayLength]; //Create a Array large enough to store all the numbers from LED String
    String charBuf = ""; //Empty all charBuf Saves,
    int ledArrayPos = 0; // remember the Position in the ledArray Array
    for (int i = 0 ; i < LED.length(); i++) {
      if (LED.charAt(i) == ',') { //when there comes a comma, save the joined Number of charBuf in ledArray
        ledArray[ledArrayPos] = charBuf.toInt();
        ledArrayPos++; //use next number in array next time
        charBuf = ""; //empty the charPos for next Number
      }
      else {
        charBuf += LED.charAt(i);
      }
    }
    ledArray[ledArrayPos] = charBuf.toInt();
    Serial.println("Colors for next LED are: ");
    Serial.print("RED:"); //print out the color and all the LED that were sent.
    Serial.println(ledArray[0]);
    Serial.print("GREEN:");
    Serial.println(ledArray[1]);
    Serial.print("BLUE:");
    Serial.println(ledArray[2]);
    Serial.println("LEDs: ");
    for (int i = 3; i < ledArrayLength; i++) {
      Serial.print(ledArray[i]);
      Serial.print(", ");
    }
    Serial.println("");
    for (int i = 3; i < ledArrayLength; i++) {
      turnOnLed(ledArray[0], ledArray[1], ledArray[2], ledArray[i]);
    } 
    // resendDataForWebsocket();
    strip.show();
   }
}

/*
 * counts the Amount of Arguments seperated by a comma (,) in the Stirng that was given
 * parameters:
 *  String argsString: Der String von dem die Anzahl Argumente bestimmt werden soll
 * returns int nArgs: Anzahl Argumente
 */
int countArgs(String argsString) {
  int nArgs = 0;
  for (int i = 0; i < argsString.length(); i++) {
    if (argsString.charAt(i) == ',') {
      nArgs++;
    }
  }
  return nArgs + 1;
}

/*
 * gets the LED that was turned on the longest time ago
 */
int getOldestLED() {
  int oldestLED = 0;
  unsigned long oldestTime = millis();
  for (int i = 0; i < nLED; i++) {
    if (LEDState[i]) {
      if (LEDTime[i] < oldestTime) {
        oldestTime = LEDTime[i];
        oldestLED = i;
      }
    }
  }
  return oldestLED;
}


/* ---------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------- Startup function  ------------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/ 
void startServer() {

  server.on("/connect", HTTP_POST, handleConnect); // this method is used to connect to an existing network
  server.on("/ipinfo", handleIPInfo); // this method is used to get the ip adress of the board in the home network
  server.onNotFound(handleNotFound); // if someone requests any other file or page, go to function 'handleNotFound'
               
  server.begin(); // start the HTTP server and check if the file exists
  Serial.println("HTTP server started.");
}

void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.mode(WIFI_AP_STA); // Set the mode so that it works as a station and connects to an access point 
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");
  Serial.println("\r\n");
  
  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP address for own network ");
  Serial.print(" : ");
  Serial.println(WiFi.localIP());
  Serial.print("IP address for home network ");
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());

}

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      Serial.printf("\tFS File: %s\r\n", fileName.c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  MDNS.addService("http", "tcp", 80); //set port for http server to 80
  MDNS.addService("ws", "tcp", 81); //set port for websocket to 81
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}


/* ---------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------- Server Handlers --------------------------------------------
 ----------------------------------------------------------------------------------------------------------------*/ 
/*
 * handels the connect to a other network, reads the Arguments that were given (ssid and password) from the server.args map. if no connection after ~7sec break out.
 */
void handleConnect() { //connect the http Server with the sent Network
  if (server.args() > 1) { //only try to connect if there are both values sent (ssid and password)
    int breaker = 0;
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    WiFi.disconnect(); //disconnect from the current Network, program not made for multiple network connections
    WiFi.begin(ssid, password); //start the connection (c_str() changes a String to a char array)
     // Connecting to WiFi...
    Serial.print("Connecting to ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED)
    { 
      delay(100);
      Serial.print(".");
    }
    handleIPInfo();
  }
}

/*
 * send the ip of the http server in the local network (not of the acces point hosted on the Wemos)
 */
void handleIPInfo() {
  server.send(200, "text/plain", WiFi.localIP().toString());
}

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}


bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  if (SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal                                      // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  return "text/plain";
}

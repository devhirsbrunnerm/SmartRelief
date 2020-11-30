/*
 * This is the Arduino Sketch for the SmartRelief project. It hosts a webserver and controls a RGB Matrix based on the input from the websocket from a connected client
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
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>

/*------------------- Defines --------------- */
#define USE_SERIAL Serial
#define pinMatrix D6

/*------------------- Adjustable values ------------*/
const int nLED = 64;
const int maxNLED = 25;
const int LEDBrightness = 20;
const int maxNumberOfClients = 20;



/* ----------------- Global variables ----------------- */
bool LEDState[nLED]; //Saves wheter an LED is turned on or off (true for on, false for off)
unsigned long LEDTime[nLED]; //Saves last time the LED was turned on

unsigned long lastCall = 0; //Saves the last time something happened
boolean TimerBasedOff = false; //Saves wheter the LED got turned off because of timer (So it doesnt keep turning them off)
unsigned long timerInterval = 60 * 1000; //Nach wie langer inaktiver Zeit die LED abgeschaltet werden (*1000 da in Millisekunden -> 60 * 1000 = 60 Sekunden)


/* ------------------- Webserver and Wifi variables -------------- */
ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
const char *ssid = "SmartRelief"; // The name of the Wi-Fi network that will be created
const char *password = "";   // The password required to connect to it, leave blank for an open network
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
  strip.begin();
  strip.show(); // set all pixels to 'off'
  strip.setBrightness(LEDBrightness);

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

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
  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();  
  checkTimer();
}

/*
 * -------------------------------- Websocket Handlers -------------------------- 
 */

uint8_t clients[maxNumberOfClients];
int numberOfClients = 0;
 void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      numberOfClients--;
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        String json = parseLedDataToJson();
        
        webSocket.sendTXT(num, json);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String LED = String((char *) &payload[0]);
      if(LED[0] != 'u') {
        turnOn(LED); //call function to turn on the requested LED. Example for LED String: "255,255,255,5,6,7,8,9". the first 3 Values define the Color of the LED
      }
      String json = parseLedDataToJson();
      webSocket.sendTXT(num, json);
      break;
  }
}

/*
 * Send the actual data about the LED to the all WebSockets (In this case Clients connected via. Javascript websocket)
 */
void resendDataForWebsocket(){
   String json = parseLedDataToJson();
   // For some reason this crashes the webserver, i have not been able to figure out why this happens so far.
   // It does not create an endless loop of sending and recieving, so i don't know why it crashes.
   // What is not possible like this right now, is that data gets live updated on other devices
   // webSocket.broadcastTXT(json);
}

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

/* 
 *  ------------------------------------------ LED HELPER FUNCTIONS --------------------------------- 
 */

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
 * turns on single led (usen in ledOn() )
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
    if (nLEDOn() <= maxNLED) {
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


/*
 * ---------------------------------------------- Startup Helpers ------------------------------------- 
 */

void startServer() { // Start a HTTP server with a file read handler and an upload handler                     // go to 'handleFileUpload'

  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/ipinfo", handleIPInfo);
  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'

  server.serveStatic("/", SPIFFS, "/index.html"); //on no adress request send the index page stored in SPIFFS
  server.serveStatic("/main.css", SPIFFS, "/main.css"); //on http server request send the files from SPIFFS
  server.serveStatic("/preview_icon.png", SPIFFS, "/preview_icon.png");
  server.serveStatic("/main.js", SPIFFS, "/main.js");
  server.serveStatic("/variables.js", SPIFFS, "/variables.js");
  server.serveStatic("/bootstrap.min.js", SPIFFS, "/bootstrap.min.js");
  server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css");
  server.serveStatic("/jquery.min.js", SPIFFS, "/jquery.min.js");
               
  server.begin(); // start the HTTP server and check if the file exists
  Serial.println("HTTP server started.");
}

void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP address for network ");
  Serial.print(" : ");
  Serial.println(WiFi.localIP());
  Serial.print("IP address for network ");
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());
  
  WiFi.mode(WIFI_AP_STA);  
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  Serial.println("Connecting");
  if(WiFi.softAPgetStationNum() == 0) {      // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");
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


/* --------------------------------------------------- Server Handlers --------------------------------- */
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
  String pathWithGz = path + ".gz";
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
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".png")) return "image/png";
  return "text/plain";
}

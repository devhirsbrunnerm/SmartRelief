# SmartRelief
An ESP8266 project to control a RGB-Matrix via a website hosted on the device. 
It was only tested with the WEMOS D1 R2 V2.1.0 Board.

The idea behind this project is to have an RGB Matrix with optical fibres that are connected to a specific point in a mountain relief. Via a website the specific RGB Lights are controlled, allowing a user to view where a specific point in this relief is (for example a mountain, a lake and so on).

The standard IP Adress in the "SmartRelief" Network is 192.168.4.1
## Setup
I used a WEMOS D1 R2 V2.1.0 Board.

To load this software the board, I used the Arduino IDE. I added the following URL to the board managers:
* http://arduino.esp8266.com/stable/package_esp8266com_index.json

I then installed the "esp8266 by ESP8266 Community version 2.7.4" in Boards Manager, since this adds support for the WEMOS D1 R2 (found as LOLIN(WEMOS) D1 R2).
A more elaborate guide can be found [here](https://www.instructables.com/Programming-the-WeMos-Using-Arduino-SoftwareIDE/).

The libraries i installed via the libaray manager are:
* Adafruit NeoPixel by Adafruit Version 1.7.0
* WebSockets by Markus Sattler Version 2.3.1


## Load the Code
To run the code on the WEMOS, 2 steps are needed:
1. run Tools -> ESP8266 Sketch Data Upload (This uploads the files in the data folder in the SPIFFS of the WEMOS).
2. run Sketch -> Upload (This uploads the Code for the WEMOS).


## Helpful Links
* [Setting up the Arduino IDE with the WEMOS](https://www.instructables.com/Programming-the-WeMos-Using-Arduino-SoftwareIDE/)
* [Getting started with ESP8266 and Websockets (a complete example)](https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html)
* [Explanation of SPIFFS and serving it as webserver](https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/)
* [Explanation of the different WIFI Modes](https://nodemcu.readthedocs.io/en/release/modules/wifi/)


# Overview of code
1. Wemos starts it's network, webserver, websocket, SPIFFS and so on
2. Client requests the webpage via the webserver which runs on port 80
3. The Webserver servces the files that are stored in the SPIFFS
4. The Client activates or deactivates LEDs via the Websocket
5. The Websocket server recieves the event, parses the payload and processes the information. The Server sends the updated state to the client

## Live update for multiple clients
To support live updates for multiple clients, the clients ask for updated data every 3 seconds. As reaction the server sends an update to the client, who can then adjust the UI.

## Steps to add WEMOS to Home network
To add the WEMOS to the home network, the client does the following:
* Load the webpage
* Fill out the form to add the WEMOS to the network (requires ssid and password)
* Send the form data as HTTP_POST to /connect
* The server then tries to connect to the network
* Upon successful connection, the user gets the new IP Address as response
  - Alternativley the client can go to /ipinfo to get the IP Address

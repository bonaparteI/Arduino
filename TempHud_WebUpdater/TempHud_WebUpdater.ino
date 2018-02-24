/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" myhome.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include "dht11.h"

/* Global variables area */
const char* host = "myhome";
const char* ssid = "W04_14A51AEF9DAE";
const char* password = "abh3r2y1mtig36e";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

const char DHT11_PIN = 2;

/* Support functions area */
////////////////////////////////////////////////////////////////////////////////
void readDhtData(dht11 *Dht) {
  int chk;
  chk = Dht->read(DHT11_PIN);

  switch (chk) {
    case DHTLIB_OK:
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
    break; default:
      Serial.print("Unknown error,\t");
      break;
  }

  Serial.print("DHT11, \t");
  Serial.print("OK,\t");
  Serial.print(Dht->humidity, 1);
  Serial.print(",\t");
  Serial.println(Dht->temperature, 1);
  delay(1000);
}

////////////////////////////////////////////////////////////////////////////////
void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  dht11 DHT;
  readDhtData(&DHT);

  snprintf ( temp, 400,

             "<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>My Home</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Welcome to TungDepTrai's home!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Refresh interval: 10sec</p>\
    <h2>Temperature: %02d oC</h2>\
    <h2>Humidity: %02d %</h2>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

             hr, min % 60, sec % 60, DHT.temperature, DHT.humidity
           );
  httpServer.send ( 200, "text/html", temp );
}

////////////////////////////////////////////////////////////////////////////////
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += ( httpServer.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
    message += " " + httpServer.argName ( i ) + ": " + httpServer.arg ( i ) + "\n";
  }

  httpServer.send ( 404, "text/plain", message );
}

////////////////////////////////////////////////////////////////////////////////
void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  httpServer.send ( 200, "image/svg+xml", out);
}

////////////////////////////////////////////////////////////////////////////////
void connectWifi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
}

////////////////////////////////////////////////////////////////////////////////
void startWebUpdater() {
  /* For Web Updater */
  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n\n", host);
}

////////////////////////////////////////////////////////////////////////////////
void startHttpServer() {
  httpServer.on ( "/", handleRoot );
  httpServer.on ( "/test.svg", drawGraph );
  httpServer.on ( "/inline", []() {
    httpServer.send ( 200, "text/plain", "this works as well" );
  } );
  httpServer.onNotFound ( handleNotFound );
  httpServer.begin();
  Serial.println ( "HTTP server started" );
}

/* Main functions area */
////////////////////////////////////////////////////////////////////////////////
void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");

  connectWifi();

  startWebUpdater();

  startHttpServer();
}

////////////////////////////////////////////////////////////////////////////////
void loop(void) {
  httpServer.handleClient();
}



#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "Maker-office";
const char *password = "hulurobot423";
MDNSResponder mdns;

ESP8266WebServer server ( 80 );

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 400,
"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Serial Camera</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",hr, min % 60, sec % 60);
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x+= 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send ( 200, "image/svg+xml", out);
}
int isRequestMode = -1;
int bufferIndex = 0;
union{
  byte bytes[2];
  unsigned short s;
}bytes2short;
unsigned char imgBuffer[1024];
void setup ( void ) {
  Serial.begin ( 115200 );
  WiFi.begin ( ssid, password );
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on ( "/", handleRoot );
  server.on ( "/test.svg", drawGraph );
  server.on ( "/request/length", []() {
    bufferIndex = 0;
    isRequestMode = 0;
    String s = "length:";
    s += bytes2short.bytes[0];
    s += " ";
    s += bytes2short.bytes[1];
    s += " ";
    s += bytes2short.s;
    s += "\n";
    server.send ( 200, "text/html", s );
  } );
  server.on ( "/request/buffer", []() {
    bufferIndex = 0;
    isRequestMode = 2;
    server.send ( 200, "text/html", "/request/buffer ok" );
    Serial.print("/request/buffer\n");
  } );
  server.on ( "/capture", []() {
    server.send ( 200, "text/html", "/capture ok" );
    isRequestMode = 0;
    Serial.print("/clear\n");
    delay(500);
    Serial.print("/capture\n");
    delay(500);
    bufferIndex = 0;
    isRequestMode = 1;
    Serial.print("/request/length\n");
  } );
  server.on ( "/clear", []() {
    isRequestMode = 0;
    server.send ( 200, "text/html", "/clear ok" );
    Serial.print("/clear\n");
  } );
  server.on ( "/reset", []() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send ( 200, "text/html", "/reset\n" );
    Serial.print("/reset\n");
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}
char ok[3];
void loop ( void ) {
  if(isRequestMode<=0){
    mdns.update();
    server.handleClient();
  }else{
    if(Serial.available()){
      unsigned char c = Serial.read();
      if(isRequestMode==1){
        if(bufferIndex>=7&&bufferIndex<=8){
          bytes2short.bytes[bufferIndex-7] = c;
          if(bufferIndex==8){
            isRequestMode = 0;
          }
        }
      }
      if(isRequestMode==2){
        imgBuffer[bufferIndex]=c;
      }
      bufferIndex++;
      if(ok[0]=='\n'&&ok[1]=='k'&&ok[2]=='o'){    
      }
      ok[2] = ok[1];
      ok[1] = ok[0];
      ok[0] = c;
    }
  }
}

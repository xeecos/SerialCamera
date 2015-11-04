#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
}

int value = 0;
bool isCapture = false;
unsigned char bytes[10240];
int bytesSize = 0;
int bytesIndex = 0;
String buffer = "";
void loop() {
   if(Serial.available()){
    if(isCapture){
      bytes[bytesIndex] = Serial.read();
      bytesIndex++;
      if(bufferIndex>=bytesSize){
        isCapture = false;
      }
    }else{
      char c = Serial.read();
      if(c=='\n'){
        parseBuffer();
      }else{
        buffer+=c;
      }
    }
  }
}
void parseBuffer(){
  buffer = buffer+"/";
  int count = 0;
  int startIndex = 0;
  int endIndex = 0;
  int len = buffer.length();
  if(len<1){
    return;
  }
  String tmp;
  String values[10];
  while(true) {
    startIndex = buffer.indexOf("/", endIndex);
    endIndex = buffer.indexOf("/", startIndex + 1);
    tmp = buffer.substring(startIndex+1, endIndex);
    values[count] = tmp;
    count++;
    if(endIndex==len-1) break;
  }
  if(values[0].equals("setupwifi")){
    setupWIFI(values[1],values[2]);
  }
  if(values[0].equals("wifistatus")){
    wifiStatus();
  }
  if(values[0].equals("capture")){
    captureMode(values[1]);
  }
  Serial.println(buffer);
  buffer = "";
}
void setupWIFI(String ssid, String password){

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void wifiStatus(){
  Serial.print("/wifistatus/");
  Serial.print(WiFi.status());
}
void captureMode(String len){
  bytesSize = len.toInt();
  isCapture = true;
}
void sendRequest(String host,String port,String url){
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host.c_str(), port.toInt())) {
    Serial.println("connection failed");
    return;
  }
  url.replace("%2F","/");
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}


void urlencode(String str)
{
  str.replace("/","%2F");
}
void urldecode(String str)
{
  str.replace("%2F","/");
}


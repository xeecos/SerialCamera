#include <MeBaseBoard.h>
#include <SoftwareSerial.h>

SoftwareSerial sw(A10,A9);
String buffer = "";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  sw.begin(19200);
}

  // We now create a URI for the request
double lastTime = 0;
bool isCapture = false;
int bytesSize = 0;
int bytesIndex = 0;
void loop() {
  if(Serial1.available()){
    char c = Serial1.read();
    if(c=='\n'){
      parseBuffer();
    }else{
      buffer+=c;
    }
  }
  if(sw.available()){
    unsigned char c = sw.read();
    Serial.write(c);
    Serial1.write(c);
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
  if(values[0].equals("resolution")){
    cameraResolution(values[1].toInt());
  }else if(values[0].equals("clear")){
    cameraClear();
  }else if(values[0].equals("reset")){
    cameraReset();
  }else if(values[0].equals("capture")){
    cameraCapture();
  }else if(values[0].equals("quality")){
    cameraQuality(values[1].toInt());
  }else if(values[0].equals("request")){
    if(values[1].equals("length")){
      cameraGetLength();
    }else if(values[1].equals("buffer")){
      cameraGetBuffer(values[2].toInt(),values[3].toInt(),values[4].toInt(),values[5].toInt());
    }
  }else if(values[0].equals("baudrate")){
    cameraBaudrate(values[1].toInt());
  }
  buffer = "";
  Serial.print(values[0]);
  Serial.print(" ok\n");
}
void cameraReset(){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x26);
  sw.write((unsigned char)0x0);
}
void cameraClear(){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x36);
  sw.write(0x01);
  sw.write(0x03);
}
void cameraResolution(unsigned char resolution){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x31);
  sw.write(0x05);
  sw.write(0x04);
  sw.write(0x01);
  sw.write((unsigned char)0x00);
  sw.write(0x19);
  if(resolution==0){
    sw.write(0x11);
  }else if(resolution==1){
    sw.write((unsigned char)0x00);
  }else if(resolution==2){
    sw.write(0x22);
  }
}
void cameraQuality(unsigned char quality){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x31);
  sw.write(0x05);
  sw.write(0x01);
  sw.write(0x01);
  sw.write(0x12);
  sw.write(0x04);
  sw.write(quality);
}
void cameraCapture(){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x36);
  sw.write(0x01);
  sw.write((unsigned char)0x0);
}
void cameraGetLength(){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x34);
  sw.write(0x01);
  sw.write((unsigned char)0x0);
}
void cameraGetBuffer(unsigned char s1, unsigned char s2, unsigned char e1, unsigned char e2){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x32);
  sw.write(0x0C);
  sw.write((unsigned char)0x00);
  sw.write(0x0A);
  sw.write((unsigned char)0x00);
  sw.write((unsigned char)0x00);
  sw.write(s1);
  sw.write(s2);
  sw.write((unsigned char)0x00);
  sw.write((unsigned char)0x00);
  sw.write(e1);
  sw.write(e2);
  sw.write((unsigned char)0x00);
  sw.write(0xFF);
}
void cameraBaudrate(unsigned char baudrate){
  sw.write(0x56);
  sw.write((unsigned char)0x0);
  sw.write(0x31);
  sw.write(0x06);
  sw.write(0x04);
  sw.write(0x02);
  sw.write((unsigned char)0x00);
  sw.write(0x08);
  if(baudrate==0){
    sw.write(0xAE);
    sw.write(0xC8);//9600
  }else if(baudrate==1){
    sw.write(0x56);
    sw.write(0xE4);//19200
  }else if(baudrate==2){
    sw.write(0x2A);
    sw.write(0xF2);//38400
  }else if(baudrate==3){
    sw.write(0x1C);
    sw.write(0x4C);//57600
  }else if(baudrate==4){
    sw.write(0x0D);
    sw.write(0xA6);//115200
  }
}
void urlencode(String input)
{
  input.replace("/","%2F");
}
void urldecode(String input)
{
  input.replace("%2F","/");
}

#include <Arduino.h>
 
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>
#include <pgmspace.h>
 #include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "tar.h"
#include <HTTPClient.h>
 
const char *ssid = "RedOne";
const char *password = "qweasdzxc";
const char *serverLoc = "http://127.0.0.1:8080";
//const uint16_t screenBuf1[26*320];
//const uint16_t screenBuf2[26*320];
//const uint16_t screenBuf3[48*320];
 
 
TFT_eSPI tft = TFT_eSPI();
 
void drawImageByPixel(const unsigned short *data1, int32_t h, int32_t w, int32_t x, int32_t y)
{
  for(int i = 0;i<h; i++)
  {
    for(int j = 0;j<w; j++)
    {
      tft.drawPixel(j+x,i+y,data1[i*w+j]);
    }
  }
}
 
//此函数的用处是从总的背景中截取一小块逐像素绘制在屏幕上，用于在时钟或者是日期的某几位重绘之前
//覆盖之前的字符
void printPart(const uint16_t* data1, int32_t h, int32_t w, int x, int y)
{
  for(int i = 0;i<h;i++)
  {
    for(int j = 0;j<w;j++)
    {
      uint16_t pixel = data1[(y+i)*240+x+j];
      tft.drawPixel(j+x,i+y,pixel);
    }
  }
}
 
void setup() {
  //Initialize Screen
  tft.init();
  tft.fillScreen(TFT_SKYBLUE);
  tft.setRotation(1);
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 5);
  tft.println("Booting...");
  // put your setup code here, to run once:
  Serial.begin(115200);
  tft.setCursor(10, 36);
  tft.println("Connecting");
  tft.setCursor(10, 67);
  tft.println(ssid);
  tft.setCursor(10, 98);
  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);
  int restartCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    tft.print(".");
    if(restartCount>=5)
    {
      //WiFi.end();
      delay(1000);
      WiFi.begin(ssid, password);
      restartCount = 0;
    }
    restartCount++;
  }
  tft.println("");
  tft.setCursor(10, 129);
  tft.println("WiFi Ready");
  delay(500);
  pinMode(0, INPUT);
  drawImageByPixel(imgArray, imgHeight, imgWidth, 0,0);
  //tft.pushImage(0,0,imgWidth,imgHeight,imgArray);
  tft.setTextFont(4);
}
 
String legend1 = "          ";
String legend2 = "        ";
int isCon = 0;
void loop() {
  // put your main code here, to run repeatedly:
  HTTPClient http;
  //Serial.println("try");
  http.begin(serverLoc);
  
  String dataToSend;
  if(!digitalRead(0))
  {
    dataToSend = "Button pressed!";
  }
  else
  {
    dataToSend = "Button not pressed!";
  }
  int httpCode = http.POST(dataToSend);
  if (httpCode == HTTP_CODE_OK) 
  {
      delay(100);
      String line = http.getString(); 
      if(line.indexOf("@") != -1)
      {
        String strToPrint = line.substring(2, line.length()-1);
        int splitLoc = strToPrint.indexOf('!');
        String strToPrint1 = strToPrint.substring(0, splitLoc);
        String strToPrint2 = strToPrint.substring(splitLoc + 1, strToPrint.length());
        tft.setTextColor(tft.color565(234,153,46));
        tft.setTextFont(4);
        if(isCon)
        {
          isCon = 0;
          drawImageByPixel(imgArray, 36,320,0,0);
        }
        int x = (320 - tft.textWidth("Current Time is"))/2;
        tft.setCursor(x, 10);
        tft.println("Current Time is");
        x = (320-tft.textWidth(strToPrint1))/2;
        tft.setCursor(x,41);
        for(int i = 0;i<strToPrint1.length();i++)
        {
          if(strToPrint1[i] == legend1[i])
          {
            tft.print(strToPrint1[i]);
          }
          else
          {
            int x,y;
            x = tft.getCursorX();
            y = tft.getCursorY();
            //此处仅仅重绘了对应的改变过的字符下面的背景图案，节省时间
            printPart(imgArray, 26, tft.textWidth(&strToPrint1[i]), x,y);
            tft.print(strToPrint1[i]);
          }
        }
        tft.setTextFont(7);
        x = (320-tft.textWidth(strToPrint2))/2;
        tft.setCursor(x,72);
        for(int i = 0;i<strToPrint2.length();i++)
        {
          if(strToPrint2[i] == legend2[i])
          {
            tft.print(strToPrint2[i]);
          }
          else
          {
            int x,y;
            x = tft.getCursorX();
            y = tft.getCursorY();
            //此处仅仅重绘了对应的改变过的字符下面的背景图案，节省时间
            printPart(imgArray, 48, tft.textWidth(&strToPrint2[i]), x,y);
            tft.print(strToPrint2[i]);
          }
        }
        //tft.println(strToPrint2);
        legend1 = strToPrint1;
        legend2 = strToPrint2;
      }
      http.end();
      delay(400);
  }
  else
  {
    //tft.fillScreen(TFT_SILVER);
    tft.fillRect(0,10,320,26, TFT_YELLOW);
    tft.setCursor(5,10);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(4);
    tft.println("Connection failed!");
    http.end(); //关闭当前连接
    isCon = 1;
    delay(1000);
  }
  //delay(500);
 
}
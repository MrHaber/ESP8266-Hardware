#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <string>
#include <time.h>

#define TFT_RST   D0
#define TFT_CS    D1
#define TFT_DC    D2
#define countof(a) (sizeof(a) / sizeof(a[0]))
#define INPUT_SIZE 30

using namespace std;

RtcDS3231<TwoWire> Rtc(Wire);
const uint16_t GLCD_Color_Black = 0x0000;
const uint16_t GLCD_Color_Blue = 0x001F;
const uint16_t GLCD_Color_Red = 0xF800;
const uint16_t GLCD_Color_Green = 0x07E0;
const uint16_t GLCD_Color_Cyan = 0x07FF;
const uint16_t GLCD_Color_Magenta = 0xF81F;
const uint16_t GLCD_Color_Yellow = 0xFFE0;
const uint16_t GLCD_Color_White = 0xFFFF;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#ifndef STASSID
#define STASSID "HOME ID" // Filling require
#define STAPSK  "PASSWORD" // Filling require
#endif
const char* ssidAP     = "ESP8266-Access-Point";
const char* passwordAP = "123456789";
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", SendHTML());
  digitalWrite(led, 0);
}

String SendHTML()
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Timer Test</title>\n";
  ptr +="</head>\n";
  ptr += "<p>Date/Time: <span id=\"datetime\"></span></p>\n";
  ptr += "<script>";
  ptr += "var dt = new Date();";
  ptr += "document.getElementById(\"datetime\").innerHTML = dt.toLocaleString();";
  ptr += "</script>";
  ptr += "<meta http-equiv=\"refresh\" content=\"3\" />";
  ptr +="</html>\n";
  return ptr;
}
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

String weekDays[7]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
 
//Month names
String months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void initTime(){
      if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }
}

const long utcOffsetInSeconds = 7200;
bool loading = false;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
void setup(void) {

  Wire.begin(12,0);

  Rtc.Begin();
  unsigned int iterator = 0;
  
  unsigned short maxValue = 81;

  String dayValue;

  String timeValue;
  
  tft.initR(INITR_BLACKTAB);
  
  tft.fillScreen(GLCD_Color_Black);
  delay(300);
  
  loadingProc();
  
  loading = true;
  bool foundConnection = true;
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Trying connect to ");
  Serial.print(ssid);
  // Wait for connection
 while (WiFi.status() != WL_CONNECTED && (iterator <= 80)) {
   iterator+=1;
   delay(500);
   Serial.print("Trying reconnect: ");
   Serial.print((maxValue - iterator));
   Serial.print("\n");
   if(iterator == 50) {
    Serial.print("Connection may be refused");
    Serial.print("\n");
    delay(1000);
    errorMsg(GLCD_Color_Red);
   }
  }
  if (iterator >= 80) {
    foundConnection = false;
    WiFi.disconnect();
  }
  delay(100);
  if(!foundConnection) {

 // HttpClient client;

  Serial.print("\n");
  Serial.println("Connection not found or password is incorrect. Trying create Access point...");
  Serial.println("Setting AP (Access Point)…");
  WiFi.softAP(ssidAP, passwordAP);
  Serial.println(WiFi.localIP());
  IPAddress IP = WiFi.softAPIP();
  String IpValue = IpAddress2String(IP);
// client.get(IpValue);
  delay(100);
  /*while (client.available()) {

    char c = client.read();

    Serial.print(c);

  }*/
  
  Serial.println("AP IP address: ");
  Serial.println(IP);
  delay(100);
  Serial.println("\n");
  
  RtcDateTime dt = RtcDateTime(__DATE__, __TIME__);
      char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year() );
  dayValue = datestring;
  
      char timeString[20];

      snprintf_P(timeString, 
            countof(timeString),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
            
  timeValue = timeString;
  }else{
  Serial.print("Connected to ");
  Serial.println(ssid);
  timeClient.begin();
  timeClient.setTimeOffset(10800);
  
  delay(100);
  
  timeClient.update();
  
  unsigned long epochTime = timeClient.getEpochTime();
  
 // struct tm *ptm = ctime ((time_t *)&epochTime);  // Sun Sep 19 15:48:43 2021
  struct tm ts, te, t_now;
  time_t tims = (time_t(epochTime));
  ts = te = t_now = *localtime(&tims);
  
  string bags;
  String bag = String(tims);
  Serial.print("HOURS: ");
  Serial.print(ts.tm_hour);
  char spliterator[100] = " ";
  char c2[100];
  String split = " ";
  strcpy(c2, spliterator);
  
  String monthDay = getValue(bag, split, 0);

  String currentMonth = getValue(bag,split, 1);
  String currentYear = getValue(bag,split, 4);

  String currentDate = getFullFormattedTime(timeClient);
  
  //int splitT = formattedDate.indexOf("T");
 // dayStamp = formattedDate.substring(0, splitT);
 // timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print(currentDate);
  dayValue = currentDate;
  timeValue = timeClient.getFormattedTime();
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  loading = false;
  delay(100);
  
  //bootstrap();

  updateTime(dayValue, timeValue);
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

 /* server.on("/gif", []() {
    static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
    };*/
  /*  char gif_colored[sizeof(gif)];
    memcpy_P(gif_colored, gif, sizeof(gif));
    // Set the background to a random set of colors
    gif_colored[16] = millis() % 256;
    gif_colored[17] = millis() % 256;
    gif_colored[18] = millis() % 256;
    server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
  });*/

  server.onNotFound(handleNotFound);

  /////////////////////////////////////////////////////////
  // Hook examples

  server.addHook([](const String & method, const String & url, WiFiClient * client, ESP8266WebServer::ContentTypeFunction contentType) {
    (void)method;      // GET, PUT, ...
    (void)url;         // example: /root/myfile.html
    (void)client;      // the webserver tcp client connection
    (void)contentType; // contentType(".html") => "text/html"
    Serial.printf("A useless web hook has passed\n");
    Serial.printf("(this hook is in 0x%08x area (401x=IRAM 402x=FLASH))\n", esp_get_program_counter());
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  server.addHook([](const String&, const String & url, WiFiClient*, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/fail")) {
      Serial.printf("An always failing web hook has been triggered\n");
      return ESP8266WebServer::CLIENT_MUST_STOP;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  server.addHook([](const String&, const String & url, WiFiClient * client, ESP8266WebServer::ContentTypeFunction) {
    if (url.startsWith("/dump")) {
      Serial.printf("The dumper web hook is on the run\n");

      // Here the request is not interpreted, so we cannot for sure
      // swallow the exact amount matching the full request+content,
      // hence the tcp connection cannot be handled anymore by the
      // webserver.
#ifdef STREAMSEND_API
      // we are lucky
      client->sendAll(Serial, 500);
#else
      auto last = millis();
      while ((millis() - last) < 500) {
        char buf[32];
        size_t len = client->read((uint8_t*)buf, sizeof(buf));
        if (len > 0) {
          Serial.printf("(<%d> chars)", (int)len);
          Serial.write(buf, len);
          last = millis();
        }
      }
#endif
      // Two choices: return MUST STOP and webserver will close it
      //                       (we already have the example with '/fail' hook)
      // or                  IS GIVEN and webserver will forget it
      // trying with IS GIVEN and storing it on a dumb WiFiClient.
      // check the client connection: it should not immediately be closed
      // (make another '/dump' one to close the first)
      Serial.printf("\nTelling server to forget this connection\n");
      static WiFiClient forgetme = *client; // stop previous one if present and transfer client refcounter
      return ESP8266WebServer::CLIENT_IS_GIVEN;
    }
    return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
  });

  // Hook examples
  /////////////////////////////////////////////////////////

  server.begin();
  Serial.println("HTTP server started");
}

String getFullFormattedTime(NTPClient client) {
   time_t rawtime = client.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return dayStr + "/" + monthStr + "/" + yearStr+ " ";
}

String getValue(String data, String separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == ' ' || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]); 
}
void loadingProc() {
   tft.setTextWrap(false);
  tft.fillScreen(GLCD_Color_Black);
  tft.setRotation(1);
  tft.setTextColor(GLCD_Color_Blue);
  tft.setTextSize(2);
  tft.setCursor(20, 25);
  tft.print("Loading.... ");
}
void updateTime(String dateText, String timeText) {
  tft.fillScreen(GLCD_Color_Black);
  tft.setTextColor(GLCD_Color_Green);
  tft.setTextSize(2);
  tft.setCursor(40, 15);
  tft.print("Time");
  tft.setCursor(20, 40);
  tft.print(dateText);
  tft.setCursor(20, 65);
  tft.print(timeText);
}
void bootstrap() {
    // Screen init
  tft.fillScreen(GLCD_Color_Black);
  tft.setTextColor(GLCD_Color_Green);
  tft.setTextSize(2);
  tft.setCursor(15, 15);
  tft.print("Time: ");
}
void errorMsg(uint16_t color) {
  
    tft.setCursor(10, 60);
    tft.setTextColor(color);
    tft.setTextSize(1);
    tft.print("We have trouble with wifi");
    tft.setCursor(10, 70);
    tft.print("We will use access point");
}
void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}
void loop(void) {
  server.handleClient();
  MDNS.update();
  delay(100);
}

String printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return (datestring);
}

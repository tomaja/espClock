#include <ArduinoJson.h>

#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <ESP8266HTTPClient.h>

#include "Display.h"
#include "Util.h"

enum Mode { TIME, IP, TEMP };
Mode mode = Mode::IP;

const char *ssid     = "tomaje";
const char *password = "kol1kopter123";

char szTemp[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
std::vector<std::string> vParts;  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

ESP8266WebServer server(80);

void handleWeather()
{
  String strWeather;

  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, "http://api.openweathermap.org/data/2.5/weather?id=792937&APPID=e33b2c54bc45a503418168a99d682fe0"))
  {
    // start connection and send HTTP header
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
        strWeather = payload;
        
        StaticJsonBuffer<2000> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(strWeather);
        if (root.success())
        {
          JsonObject& last = root["main"];
          double temp = last["temp"];
          temp -= 273.15;
          Serial.println(temp);
          sprintf(szTemp, "  %.2f     ", temp);
          mode = Mode::TEMP;
        }
        else
        {
          Serial.println("parseObject() failed");
          Serial.println(payload);
          mode = Mode::TIME;
        }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  server.send(200, "text/plain", szTemp);
}

void handleIP()
{
  mode = Mode::IP;
}
void setup()
{
  ESP.wdtDisable();
  pinMode(BUFFER_DATA, OUTPUT);
  pinMode(BUFFER_CLK, OUTPUT);
  pinMode(BUFFER_LATCH, OUTPUT);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  int count = 0;
  std::string strInit = "";
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay ( 40 );
    Serial.print ( "." );
    strInit += '0';
    PrintForMS(strInit, 200);
    if(strInit == "00000000")
    {
      strInit = "";
    }
  }
  IPAddress ip;
  ip = WiFi.localIP();
  Serial.println(ip);
  ESP.wdtFeed();
  vParts = Split(std::string(ip.toString().c_str()), '.');
  
  server.on("/w", handleWeather);
  server.on("/t", [&](){ mode == Mode::TIME; return true; });
  server.on("/i", handleIP);
  server.begin();
  timeClient.begin();
}

void loop()
{
  timeClient.update();
  server.handleClient();

  if(true)
  {
    if(mode == Mode::IP)
    {
      PrintForMS(vParts[0].c_str(), 350);
      PrintForMS(vParts[1].c_str(), 350);
      PrintForMS(vParts[2].c_str(), 350);
      PrintForMS(vParts[3].c_str(), 350);
      mode = Mode::TIME;
    }
    else if(mode == Mode::TIME)
    {
      String strTime = timeClient.getFormattedTime();
      PrintString(strTime);
      delayMicroseconds(10);
    }
    else if(mode == Mode::TEMP)
    {
      PrintForMS(szTemp, 1000);
      delayMicroseconds(10);
      mode = Mode::TIME;
    }
  }
  else // TEST
  {
    PrintChar('0', 0, 1);
    PrintChar('1', 1, 0);
    PrintChar('2', 2, 1);
    PrintChar('3', 3, 0);
    PrintChar('4', 4, 1);
    PrintChar('5', 5, 0);
    PrintChar('6', 6, 1);
    PrintChar('7', 7, 0);
    PrintChar('8', 8, 1);
  }
}

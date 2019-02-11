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

const char *ssid     = "tomaje";
const char *password = "kol1kopter123";

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
        }
        else
        {
          Serial.println("parseObject() failed");
          Serial.println(payload);
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
  server.send(200, "text/plain", strWeather);
}

void setup()
{
  pinMode(BUFFER_DATA, OUTPUT);
  pinMode(BUFFER_CLK, OUTPUT);
  pinMode(BUFFER_LATCH, OUTPUT);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  server.on("/w", handleWeather);
  server.begin();
  timeClient.begin();
}

void loop() {
  timeClient.update();
  server.handleClient();
  String strTime = timeClient.getFormattedTime();
  // Serial.println(strTime);
  char szTime[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  strTime.substring(0, 9).toCharArray(szTime, 9);
  PrintString(szTime);
  Serial.println(szTime);
  delay(1000);
}

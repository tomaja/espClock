#include <ArduinoJson.h>

#include "EEPROM.h"
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

char *ssid     = "tomaje";
char *password = "kol1kopter123";

char zsSsid[32] = { 0 };
char zsPass[32] = { 0 };

char szTemp[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
std::vector<std::string> vParts;  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);

ESP8266WebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>WiFi Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/wifi">
    SSID: <input type="text" name="ssid"><br />
    Password: <input type="text" name="password"><br />
    <input type="submit" value="Save">
  </form>
</body></html>)rawliteral";

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

void ResetSettingsFlash()
{
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0xCD);
  }
}

void ReadStringFromFlash(int& nPos, char* zsString)
{
  char ch;
  int i = 0;
  while (ch = EEPROM.read(nPos++))
  {
    zsString[i++] = ch;
  }
  zsString[i] = 0;
}

void ReadSettingsFlash()
{
  EEPROM.begin(512);
  int nPosition = 0;
  ReadStringFromFlash(nPosition, zsSsid);
  ReadStringFromFlash(nPosition, zsPass);
  EEPROM.end();
}

void WriteStringToFlash(const char* strString, int& nPointer)
{
  Serial.print ( "Writing string: " );
  char ch;
  int nLen = strlen(strString);
  for (int i = 0; i < nLen; i++)
  {
    Serial.print ( strString[i] );
    EEPROM.write(nPointer++, strString[i]);
  }
  Serial.print ( "\n" );
  EEPROM.write(nPointer++, 0);
}

void WriteSettingsFlash()
{
  EEPROM.begin(512);
  int nPosition = 0;
  WriteStringToFlash(zsSsid, nPosition);
  WriteStringToFlash(zsPass, nPosition);
  EEPROM.end();
}

bool bAPMode = true;

void setup()
{
  ESP.wdtDisable();
  pinMode(BUFFER_DATA, OUTPUT);
  pinMode(BUFFER_CLK, OUTPUT);
  pinMode(BUFFER_LATCH, OUTPUT);
  
  Serial.begin(115200);
  Serial.print ( "Loading settings\n" );
  
  EEPROM.begin(512);
  //ResetSettingsFlash();
  char ch = EEPROM.read(0);
  EEPROM.end();
  
  if(ch != 0xCD)
  {
    bAPMode = false;
    ReadSettingsFlash();   
    Serial.print ( zsSsid );
    Serial.print ( "\n" );
    Serial.print ( zsPass );
    Serial.print ( "\n" );
    WiFi.begin(zsSsid, zsPass);
    
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
      count++;
      if(count > 100)
      {
        break;
      }
    }

    if(WiFi.status() != WL_CONNECTED)
    {
      boolean result = WiFi.softAP("espSat", "12345678");
      if(result == true)
      {
        Serial.println("Ready");
      
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(myIP);
        server.on("/", [&](){ server.send(200, "text/html", index_html); });
        server.on("/wifi", [&](){
          char zsChars[32] = {0};
          server.arg("ssid").toCharArray(zsChars, 32);
          strcpy(zsSsid, zsChars);
          server.arg("password").toCharArray(zsChars, 32);
          strcpy(zsPass, zsChars);
          WriteSettingsFlash();
          delay(1000);
          ESP.restart();
          });
        server.begin();
        bAPMode = true;
        Serial.println("HTTP server started");
        return;        
      }
      else
      {
        Serial.println("Failed!");
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
    server.on("/setup", [&](){ server.send(200, "text/html", index_html); });
    server.on("/wifi", [&](){
      char zsChars[32] = {0};
      server.arg("ssid").toCharArray(zsChars, 32);
      strcpy(zsSsid, zsChars);
      Serial.println(zsSsid);
      server.arg("password").toCharArray(zsChars, 32);
      strcpy(zsPass, zsChars);
      Serial.println(zsPass);
      WriteSettingsFlash();
      ESP.restart();
      });
    server.begin();
    timeClient.begin();
  }
  else
  {
    bAPMode = true;
    boolean result = WiFi.softAP("espSat", "12345678");
    if(result == true)
    {
      Serial.println("Ready");
    
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      server.on("/", [&](){ server.send(200, "text/html", index_html); });
      server.on("/wifi", [&](){
        char zsChars[32] = {0};
        server.arg("ssid").toCharArray(zsChars, 32);
        strcpy(zsSsid, zsChars);
        server.arg("password").toCharArray(zsChars, 32);
        strcpy(zsPass, zsChars);
        WriteSettingsFlash();
        delay(1000);
        ESP.restart();
        });
      server.begin();
      Serial.println("HTTP server started");
      
    }
    else
    {
      Serial.println("Failed!");
    }
    strcpy(zsSsid, ssid);
    strcpy(zsPass, password);
    WriteSettingsFlash();
  }
}

void loop()
{
  timeClient.update();
  server.handleClient();

  if(!bAPMode)
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
    PrintChar('AP      ', 0, 1);
  }
}

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


#define AP_SSID "XXXXXXXX"
#define AP_PASSWD  "XXXXXXXX"
#define AP_CHANNEL 1
#define AP_HIDDEN false
#define AP_MAX_CONNECTION 4

#define HTTP_SERVER_PORT XXXX

#define MSG_MAX_CHARACTERS 100
#define SERIAL_BAUD 9600
#define STARTUP_DELAY 1000


IPAddress ip(XXX,XXX,XXX,XXX);
IPAddress gateway(XXX,XXX,XXX,XXX);
IPAddress subnet(XXX,XXX,XXX,XXX);




ESP8266WebServer server(HTTP_SERVER_PORT);



String construct_request(String client_ip, String msg)
{
  String request = "";

  request += "{";

  request += "\"client\": ";

  request += "\"";
  request += client_ip;
  request += "\"";

  request += ", ";

  request += "\"msg\": ";
  
  request += msg;

  request += "}";

  return request;
}


void handleData()
{ 
  if( server.hasArg("msg") )
  {    
    if(server.arg("msg").length() > MSG_MAX_CHARACTERS)
    {
      return;
    }

    String request = construct_request(server.client().remoteIP().toString(), server.arg("msg"));

    Serial.println(request);

    server.send(200);
  }
}


void setup()
{  
  delay(STARTUP_DELAY);
  Serial.begin(SERIAL_BAUD);

  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTION);
  
  server.on("/data", handleData);
  server.begin();
}



void loop()
{
  server.handleClient();
}

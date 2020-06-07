#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>


#define SERVER "XXXXXXXXXXXXXXXXXXXX"
#define CHECK_CONNECT_ENDPOINT "XXXXXXXXXXXXXXXXXXXX"
#define SINK_ENDPOINT "XXXXXXXXXXXXXXXXXXX"
#define SERVER_PORT XXXX
#define AGENT "arduino-ethernet"
#define SERIAL_BAUD 9600
#define CHECK_CONNECT_TIMEOUT 5000

#define RX_PIN 6
#define TX_PIN 7
#define ACTIVE_LED_PIN 8

#define CHECK_CONNECT 0
#define ACTIVE 1


IPAddress ip(XXX, XXX, XXX, XXX);
byte mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

SoftwareSerial mySerial(RX_PIN, TX_PIN);

EthernetClient client;
String data;
int state;
unsigned long prev = 0;
unsigned long now;


void http_post(char* endpoint, String data)
{ 
  client.stop();

  if( client.connect(SERVER, SERVER_PORT) )
  {    
    client.print("POST ");
    client.print(endpoint);
    client.println(" HTTP/1.1");
    
    client.print("Host: ");
    client.println(SERVER);

    client.print("User-Agent: ");
    client.println(AGENT);

    client.println("Content-Type: application/json");
    
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
  }
}


void http_get(char* endpoint, String data)
{
  client.stop();

  if( client.connect(SERVER, SERVER_PORT) )
  {    
    client.print("GET ");
    client.print(endpoint);
    client.print(data);
    client.println(" HTTP/1.1");
    
    client.print("Host: ");
    client.println(SERVER);
    
    client.print("User-Agent: ");
    client.println(AGENT);
    
    client.println("Connection: close");
    client.println();
  }
}


void signal_active()
{
  for(int i=0; i<10; i++)
  {
    digitalWrite(ACTIVE_LED_PIN, HIGH);
    delay(100);
    digitalWrite(ACTIVE_LED_PIN, LOW);
    delay(100);
  }
}


void check_connect()
{
  if(client.available() == 143)
  { 
    signal_active();
    
    state = ACTIVE;
    return;
  }
  
  now = millis();

  if( (now - prev) < CHECK_CONNECT_TIMEOUT)
  {
    return;
  }

  prev = now;
  http_get(CHECK_CONNECT_ENDPOINT, "");
}


void active()
{
  data = "";

  if(mySerial.available())
  {
    data = mySerial.readStringUntil('\n');

    if(data.length() >= 5)
    { 
      data = data.substring(0, data.length() - 1);
      
      http_post(SINK_ENDPOINT, data);
    }
  }
}



void setup()
{ 
  pinMode(ACTIVE_LED_PIN, OUTPUT);
  
  mySerial.begin(SERIAL_BAUD); 
  
  Ethernet.begin(mac, ip);

  state = CHECK_CONNECT;
}


void loop()
{
  switch(state)
  {
    case CHECK_CONNECT:
      check_connect();
      break;

    case ACTIVE:
      active();
      break;
  }
}

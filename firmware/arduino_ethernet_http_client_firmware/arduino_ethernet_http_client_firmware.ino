#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>


#define SERVER "XXXXXXXXXXXXXXXXXXXX"
#define CHECK_CONNECT_ENDPOINT "XXXXXXXXXXXXXXXXXXXX"
#define SINK_ENDPOINT "XXXXXXXXXXXXXXXXXXX"
#define SERVER_PORT XXXX
#define AGENT "arduino-ethernet"
#define SERIAL_BAUD 9600
#define STARTUP_DELAY 5000
#define CHECK_CONNECT_TIMEOUT 5000
#define RECEIVE_TIMEOUT 900000

#define RX_PIN 8
#define TX_PIN 9
#define ACTIVE_LED_PIN A2

#define CHECK_CONNECT 0
#define ACTIVE 1

byte mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

SoftwareSerial mySerial(RX_PIN, TX_PIN);

EthernetClient client;
String data;
int state;
unsigned long check_connect_prev = 0;
unsigned long receive_prev = 0;



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
  unsigned long now = millis();

  if( abs(now - check_connect_prev) < CHECK_CONNECT_TIMEOUT)
  {
    return;
  }

  if(Ethernet.begin(mac) == 0)
  {
    return;
  }

  if(client.available() > 0)
  { 
    signal_active();
    
    state = ACTIVE;
    return;
  }
  
  http_get(CHECK_CONNECT_ENDPOINT, "");

  check_connect_prev = now;
}


void active()
{
  unsigned long receive_now = millis();

  if( abs(receive_now - receive_prev) < RECEIVE_TIMEOUT)
  {
    digitalWrite(ACTIVE_LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(ACTIVE_LED_PIN, LOW);
  }

  data = "";

  if(mySerial.available())
  {
    data = mySerial.readStringUntil('\n');

    if(data.length() >= 5)
    { 
      data = data.substring(0, data.length() - 1);
      
      http_post(SINK_ENDPOINT, data);

      receive_prev = receive_now;
    }
  }
}



void setup()
{ 
  pinMode(ACTIVE_LED_PIN, OUTPUT);
  
  mySerial.begin(SERIAL_BAUD); 

  check_connect_prev = CHECK_CONNECT_TIMEOUT;
  receive_prev = RECEIVE_TIMEOUT;

  delay(STARTUP_DELAY);

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

/*
   Example for 6-digit TM1637 based segment Display
   The number of milliseconds after start will be displayed on the 6-digit screen

   The MIT License (MIT)

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "TM1637_6D.h"

#define CLK A0 //pins definitions for TM1637 and can be changed to other ports
#define DIO A4

const bool debug = false;
const unsigned int MAX_MESSAGE_LENGTH = 12;
//Create a place to hold the incoming message
static char message[MAX_MESSAGE_LENGTH];
static unsigned int message_pos = 0;

TM1637_6D tm1637_6D(CLK, DIO);

// Array for displaying digits, the first number in the array will be displayed on the right
uint8_t ListDisp[6] = {0, 1, 2, 3, 4, 5};
// Array for displaying points, the first point in the array will be displayed on the right
uint8_t ListDispPoint[6] = {POINT_ON, POINT_OFF, POINT_OFF, POINT_OFF, POINT_OFF, POINT_OFF};
// String for converting millis value to seperate characters in the string
String millisstring;

int numrasp = 0, count = 0, countsec = 0, mode = 0;
float temperature = 0;
char buffer[24];

EthernetClient ethClient;
PubSubClient client(ethClient);


// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
// Changement parrapport au réseau
IPAddress ip(192, 168, 1, 250);
IPAddress server(192, 168, 1, 201);


/********************************
   Connection avec le broker MQTT
*********************************/
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    //    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //      client.publish("/system/relais", "0000");
      // ... and resubscribe
      //      client.publish("/system/msg", 100);
      delay(100);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

/************************************************************
   Reception des messages de MQTT sur le réseau inerne
*************************************************************/
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length ; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  buffer[0] = payload[0];
  buffer[1] = 0;
  numrasp = atoi ((char*)buffer);
  Serial.print(numrasp);

  if (numrasp == 9)
  {
    buffer[0] = payload[1];
    buffer[1] = 0;
    mode = atoi ((char*)buffer);
    Serial.print(mode);
  }
  else
  {
    buffer[0] = payload[1];
    buffer[1] = payload[2];
    buffer[2] = payload[3];
    buffer[3] = payload[4];
    buffer[4] = 0;

    temperature = atof ((char*)buffer);
    Serial.println(temperature);
  }




}

/**********************************
   Reception des messages par Serie
***********************************/
void entry() {
  if (Serial.available() > 0) {
    //Read the next available byte in the serial receive buffer
    char inByte = Serial.read();
    //Message coming in (check not terminating character) and guard for over message size
    if ( inByte != '\r' )
    {
      //Add the incoming byte to our message
      message[message_pos] = inByte;
      message_pos++;
    }
    //Full message received...
    else
    {
      //Add null character to string
      message[message_pos] = '\0';
      //Print the message (or do other things)
      //Serial.println(message);
      //Reset for the next message
      message_pos = 0;
      buffer[0] = message[0];
      buffer[1] = 0;

      numrasp = atoi ((char*)buffer);


 if (numrasp == 9)
  {
    buffer[0] = message[1];
    buffer[1] = 0;
    mode = atoi ((char*)buffer);
    Serial.print(mode);
  }
  else
  {
      buffer[0] = message[1];
      buffer[1] = message[2];
      buffer[2] = message[3];
      buffer[3] = message[4];
     // buffer[4] = 0;

      temperature = atof (buffer);
      //Serial.print(num);
      Serial.println(temperature);
  }
    }
  }

}


/************************************
    Boucle base de temps a la seconde
*************************************/
void base_de_temps()
{
  delay(10);
  count += 1;
  if (count >= 100)
  {
    count = 0;
    countsec += 1;
    Serial.println(countsec);
  }
  /*
     Coupure de sécurité
     si pas de nouvelles commandes de marche
  */
  if (countsec >= 45) {
    countsec = 0;
  }

}

/****************************************************************
   Init Programme
*****************************************************************/
void setup()
{
  Serial.begin(57600);
  tm1637_6D.init();
  // You can set the brightness level from 0(darkest) till 7(brightest) or use one
  // of the predefined brightness levels below
  tm1637_6D.set(BRIGHT_DARKEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  uint8_t ListDispPoint[6] = {POINT_ON, POINT_OFF, POINT_OFF, POINT_OFF, POINT_OFF, POINT_OFF};

  Ethernet.begin(mac, ip);

  client.setServer(server, 1883); delay(500);
  client.setCallback(callback); delay(500);

  // client.subscribe("cluster/temp");  delay(1000);

}

/****************************************************************
   Boucle Programme
*****************************************************************/
void loop() {
  if (debug) {
    entry();
    }
  else {
    if (!client.connected()) {
      reconnect(); client.subscribe("cluster/temp");
    }
    client.loop();
      }

  base_de_temps();

 
  if (numrasp == 9)
    tm1637_6D.set(mode); tm1637_6D.displayFloat(temperature, numrasp);

}

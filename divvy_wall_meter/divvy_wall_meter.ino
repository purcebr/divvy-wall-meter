

/*
* Divvy Bike Availability Meter
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h> 

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Heroku uses name-based server routing, so we need DNS here
char server[] = "shrouded-beach-2183.herokuapp.com";    // name address for Google (using DNS)

// network specific settings
IPAddress ip(172,16,2,200);
IPAddress gateway(172,16,2,1);
IPAddress subnet(255, 255, 255, 0);

//Pin for LED
int ledPin = 7;
int servoPin = 2;
int updateDelay = 60000;

Servo servoMeter;
 
EthernetClient client;

void setup()
{
    Serial.begin(9600);

    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        Ethernet.begin(mac, ip);
      }
      // give the Ethernet shield a second to initialize:
      delay(1000);
      Serial.println("connecting...");
  
      pinMode(led, OUTPUT);    
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
       
}

void loop()
{
  ledblink();
  int dial_angle;
  int start_saving;
  String clientMsg ="";
  char c;
  int index=0;
  int bikes_num;  
  int total_bikes;
  int old_bikes = 0;

  while(true) {
        
      if (client.connect(server, 80)) {

        // if you get a connection, report back via serial:
        Serial.println("connected");
        // Make a HTTP request:
        client.println("GET /stations/31 HTTP/1.1"); //Change this to your desired station id.
        client.println("Host: shrouded-beach-2183.herokuapp.com");
        client.println("Connection: close");
        client.println();

        clientMsg ="";
        start_saving = 0;
        // if there are incoming bytes available 
        // from the server, read them and print them:
        if (client) {
          //int start_saving = 0;
          while (client.connected()) {
            if (client.available()) {
              c = client.read();
              
              //Strip out the headers
              if (c == '{') {
                start_saving =1;
              }
              
              if(start_saving == 1) {
                clientMsg+=c;//store the recieved chracters in a string
              }
              //if the character is an "end of line" the whole message is recieved
            }
          }
        }
        
        /* Get total number of available docks */
        
        int start = clientMsg.indexOf("availableDocks") + 17; //the length of the json key
        int end = start + 2;
        String docks = clientMsg.substring(start, end);
        docks.replace(","," "); //remove trailing comma for single digits
        docks.trim();

        int total_docks = docks.toInt();

        /* Get total number of available bikes */

        start = clientMsg.indexOf("availableBikes") + 17; //the length of the json key
        end = start + 2;
        String bikes = clientMsg.substring(start, end);
        bikes.replace(","," "); //remove trailing comma for single digits
        bikes.trim();
      
        int total_bikes = bikes.toInt();

        //If there are 'disabled' bikes the total number of slots will be reduced. Can't always count on 23 slots available.
        int availableSlots = total_bikes + total_docks;

        if(total_bikes != old_bikes) {
          servoMeter.attach(2);
          delay(2000);
          servoMeter.write(180 - total_bikes * (180 / availableSlots));
          ledblink();
          delay(2000);
          servoMeter.detach(); // Turn off the servo when we're waiting.
        } else {
          ledflash();
        }

        if(total_docks == 0 || total_bikes == 0) {
          ledsolid();
        }
        
        old_bikes = total_bikes;
 
        client.stop();
      } else {
        Serial.println("connection failed");
      }  
        delay(updateDelay);
    }
}

/* LED Patterns */

void ledblink() {
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);               // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(250);
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);               // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
}

void ledflash() {
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(400);               // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
}

void ledsolid() {
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
}

void ledoff() {
  digitalWrite(ledPin, LOW);   // turn the LED on (HIGH is the voltage level)
}




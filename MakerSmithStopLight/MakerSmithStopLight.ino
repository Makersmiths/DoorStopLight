#include <SPI.h>
#include <EthernetV2_0.h>
#include <HttpClient.h>

byte mac[] = {  0xDE, 0xAD, 0xBE, 0xBE, 0xFE, 0xED };
//IPAddress server(173,194,33,104); // Google

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
#define W5200_CS  10
#define SDCARD_CS 4

const int switchPin = 8;
const int relayPin = 2;
//const bool stateChanged = false;
bool startup = true;
int currentState  = 0;
#define LIB_DOMAIN "arduino-tweet.appspot.com"
const char* token = "4832247938-64qLNQms41xEyYrtYFMwINi7hYn7dU4kHL06UOG";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;


void setup() {
  // put your setup code here, to run once: 
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) 
  {
      Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      for(;;)
      ;
  }
    // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");


}

void loop() {
  // put your main code here, to run repeatedly:
  if(startup)
  {
    //Read the state of the switch and setup lights, but 
    //do not send a tweet since the light may have just 
    //unpluged
    if(digitalRead(switchPin) == 1)
    {
        currentState = 1;
        digitalWrite(relayPin, HIGH);
        Serial.println("Closed");
    }
    else
    {
        currentState = 0;
        digitalWrite(relayPin, LOW);
        Serial.println("Open");
    }
    startup = false;
  }  
  else
  {
    if(digitalRead(switchPin) != currentState)
    {
      if(digitalRead(switchPin) == 1)
      {
          currentState = 1;
          UpdateTwitter(currentState);
          digitalWrite(relayPin, HIGH);
          Serial.println("Space is Closed");
      }
      else
      {
          currentState = 0;
          UpdateTwitter(currentState);
          digitalWrite(relayPin, LOW);
          Serial.println("Space is Open");
      }    
    }  
  }
  delay(1000);

}


void UpdateTwitter(int state)
{
  int err =0;
  String response;
  String status;
  
  HttpClient http(client);
  err = http.get("www.timeapi.org", "/est/now");
  if (err == 0)
  {
    err = http.responseStatusCode();
    if (err >= 0)
    {
      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        unsigned long timeoutStart = millis();
        char c;
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                response += c;
                bodyLen--;
                timeoutStart = millis();
            }
            else
            {
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
    response = random(100000);
  }
  Serial.println(response);
  Serial.println("End request");
  
  http.stop();

    if(state == 0)
    {
      if(err == -1)
      {
        status = "Space Open - ";
      }
      else
      {
        status = "Space Open at ";
      }
    }
    else
    {
      if(err == -1)
      {
        status = "Space Closed - ";
      }
      else
      {
        status = "Space Closed at ";
      }
    }

    
    String msg = "/update?token=4832247938-64qLNQms41xEyYrtYFMwINi7hYn7dU4kHL06UOG&status=" + status + response;
    msg.replace(" ", "%20");
    Serial.println(msg.c_str());
    
  err = http.get("arduino-tweet.appspot.com", msg.c_str());
  if (err == 0)
  {
    err = http.responseStatusCode();
    if (err >= 0)
    {
      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        unsigned long timeoutStart = millis();
        char c;
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                response += c;
                bodyLen--;
                timeoutStart = millis();
            }
            else
            {
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
    response = random(100000);
  }
  Serial.println(response);
  Serial.println("End request");
  
  http.stop();
}

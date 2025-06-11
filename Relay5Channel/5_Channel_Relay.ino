#include <Arduino.h>

#define RELAY1_PIN 32  // ESP32 pin GPIO26 connected to Relay's pin
#define RELAY2_PIN 33  // ESP32 pin GPIO26 connected to Relay's pin
#define RELAY3_PIN 25  // ESP32 pin GPIO26 connected to Relay's pin
#define RELAY4_PIN 26  // ESP32 pin GPIO26 connected to Relay's pin
#define RELAY5_PIN 27  // ESP32 pin GPIO26 connected to Relay's pin


void setup() {  
  Serial.begin(115200);    
  pinMode(RELAY1_PIN, OUTPUT);        // set ESP32 pin to output mode
  pinMode(RELAY2_PIN, OUTPUT);        // set ESP32 pin to output mode
  pinMode(RELAY3_PIN, OUTPUT);        // set ESP32 pin to output mode
  pinMode(RELAY4_PIN, OUTPUT);        // set ESP32 pin to output mode
  pinMode(RELAY5_PIN, OUTPUT);        // set ESP32 pin to output mode
}

void loop(){

  if(Serial.available() > 0){
    int data = Serial.read();
    if(data == 'a'){
      digitalWrite(RELAY1_PIN, LOW);   // turn on Relay
      Serial.println("Relay 1 is OFF");
    }
    else if(data == 'A'){
      digitalWrite(RELAY1_PIN, HIGH);    // turn off Relay
      Serial.println("Relay 1 is ON");
    }
    else if(data == 'b'){
      digitalWrite(RELAY2_PIN, LOW);   // turn on Relay
      Serial.println("Relay 2 is OFF");
    }
    else if(data == 'B'){
      digitalWrite(RELAY2_PIN, HIGH);    // turn off Relay
      Serial.println("Relay 2 is ON");
    }
    else if(data == 'c'){
      digitalWrite(RELAY3_PIN, LOW);   // turn on Relay
      Serial.println("Relay 3 is OFF");
    }
    else if(data == 'C'){
      digitalWrite(RELAY3_PIN, HIGH);    // turn off Relay
      Serial.println("Relay 3 is ON");
    }
    else if(data == 'd'){
      digitalWrite(RELAY4_PIN, LOW);   // turn on Relay
      Serial.println("Relay 4 is OFF");
    }
    else if(data == 'D'){
      digitalWrite(RELAY4_PIN, HIGH);    // turn off Relay
      Serial.println("Relay 4 is ON");
    }
    else if(data == 'e'){
      digitalWrite(RELAY5_PIN, LOW);   // turn on Relay
      Serial.println("Relay 5 is OFF");
    }
    else if(data == 'E'){
      digitalWrite(RELAY5_PIN, HIGH);    // turn off Relay
      Serial.println("Relay 5 is ON");
    }
  }
 }

 

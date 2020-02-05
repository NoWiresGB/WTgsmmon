/*
 *    PINOUT: 
 *        _____________________________
 *       |  ARDUINO UNO >>>   SIM800L  |
 *        -----------------------------
 *            GND      >>>   GND
 *        RX  8       >>>   TX    
 *        TX  9       >>>   RX
 *      
 */
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <SHT1x.h>
// Specify data and clock connections and instantiate SHT1x object
#define dataPin  15
#define clockPin 14
SHT1x sht1x(dataPin, clockPin);

/*
 * SHT3x sensors are 1Wire and require alternative libararies
 * 
 * #include <Wire.h>
 * #include "SHTSensor.h"
 * SHTSensor sht;
 * Use SCL/SDA pins
 */
 
#include <BareBoneSim800.h>
BareBoneSim800 sim800("wireless.twilio.com");
const char* number = "2936";  // Twilio programmable wireless API
char message[25];  // vessel to hold the SMS message as it builds
float temp_c;
float humidity;
char str_temp[6];
char str_humid[6];

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect



void do_send(void){

    if (true) {  // we will eventually check we're network attached before bothering to proceed
// read my sensors and get data


// SHT 3x only!
//        if (sht.readSample()) {
//        Serial.print("SHT:");
//        Serial.print("  RH: ");
//        humidity = sht.getHumidity();
//        Serial.print(humidity, 2);
//        //Serial.print("\n");
//        Serial.print("  T:  ");
//        temp_c = sht.getTemperature();
//        Serial.print(temp_c, 2);
//        Serial.print("\n");
//        } else {
//          Serial.print("Error in readSample()\n");
//        }

  
        temp_c = sht1x.readTemperatureC();
        humidity = sht1x.readHumidity();
// Disable Serial output
//        Serial.print(temp_c);
//        Serial.print("C RH");
//        Serial.print(humidity);
//        Serial.println("%");

        // do GSM stuff
        
        //String time1 = "";
        //time1 = sim800.getTime();  // don't use this - takes time & power.  Timestamp in Twilio logs
        
        // Convert floats to char arrays as sprintf doesn't support floats on AVR Megas.
        dtostrf(temp_c, 4, 2, str_temp);
        dtostrf(humidity, 4, 2, str_humid);
        sprintf(message, "{temp:%s,humid:%s}", str_temp, str_humid);

// Suppress serial        
//        Serial.print(" Report is: ");
//        Serial.println(message);
       


        bool messageSent =  sim800.sendSMS(number, message);
//        if(messageSent) {
//           Serial.println("Message Sent");
//
//        } else
//           Serial.println("Not Sent, Something happened");


        
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void sleep_8(void){
// disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();  
}

void setup() {
  // put your setup code here, to run once:
//  Serial.begin(115200);
//  Serial.println(F("Starting"));
  sim800.begin();

  sleep_8(); // wait for GSM module to connect
  
// SHT 3x only
//    Wire.begin();
//    if (sht.init()) {
//      Serial.print("SHTinit(): success\n");
//  } else {
//      Serial.print("SHTinit(): failed\n");
//  }
//  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x

  bool deviceAttached = sim800.isAttached();
//  if (deviceAttached)
//    Serial.println("Device is Attached");
//  else
//    Serial.println("Not Attached");

}

void loop() {
  // put your main code here, to run repeatedly:
  do_send();

  // Enable Sleep mode on GSM module
  bool sleepActivated = sim800.enterSleepMode();
//  if(sleepActivated)
//    Serial.println("Sleep Mode/Low Power Activated");
//  else
//    Serial.println("Sleep not Activated");

  // NOW SLEEP for multiples of 8 seconds  (450 x = 1 hr approx)
  // N.B. 450 => 425 = just under 1hr. Rough re-cal for WDT timer innacuracy over long period
  for (int i = 0; i < 425; i++) { 
  sleep_8();
  }

  //WAKE UP GSM MODULE
    bool disableSleep = sim800.disableSleep();
//    if(disableSleep)
//    Serial.println("Sleep Mode/Low Power Disabled");
//  else
//    Serial.println("Sleep not Disbaled");
    
}

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
#include <BareBoneSim800.h>

BareBoneSim800 sim800("wireless.twilio.com");

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  15
#define clockPin 14

SHT1x sht1x(dataPin, clockPin);

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect



void do_send(void){
    // Check if there is not a current TX/RX job running
    if (true) {
      // read my sensors and get data
        float temp_c;
        float humidity;
        temp_c = sht1x.readTemperatureC();
        humidity = sht1x.readHumidity();
        Serial.print(temp_c);
        Serial.print("C RH");
        Serial.print(humidity);
        Serial.println("%");

        // do GSM stuff
        const char* number = "2936";  // Twilio programmable wireless API
        String time1 = "";
        time1 = sim800.getTime();
        static char c_temp[6];
        static char c_humidity[6];
        dtostrf(temp_c, 4, 2, c_temp);
        dtostrf(humidity, 4, 2, c_humidity);
        String report = "{temp:";
        report.concat(c_temp);
        report.concat( ",humid:" );
        report.concat(c_humidity);
        report.concat( "}" );
        Serial.print(time1);
        Serial.print(" Report is: ");
        Serial.println(report);
        char message[25];       
        report.toCharArray(message, 25);


        bool messageSent = sim800.sendSMS(number, message);
        if(messageSent) {
           Serial.println("Message Sent");
//           Serial.println(strlen(message));
//           for ( byte i = 0; i < strlen(message); i++) {
//            Serial.print(i, DEC);
//            Serial.print("=");
//            Serial.write(message[i]);
//            Serial.println();
//           }
        } else
           Serial.println("Not Sent, Something happened");

        
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
  Serial.begin(115200);
  Serial.println(F("Starting"));
  sim800.begin();
  sleep_8();
  //delay(8000); // wait for GSM module to connect
  bool deviceAttached = sim800.isAttached();
  if (deviceAttached)
    Serial.println("Device is Attached");
  else
    Serial.println("Not Attached");

}

void loop() {
  // put your main code here, to run repeatedly:
  do_send();

  // Enable Sleep mode on GSM module
  bool sleepActivated = sim800.enterSleepMode();
  if(sleepActivated)
    Serial.println("Sleep Mode/Low Power Activated");
  else
    Serial.println("Sleep not Activated");

  // NOW SLEEP for multiples of 8 seconds  (450 x = 1 hr approx)
  // N.B. 450 => 425 = just under 1hr. Rough re-cal for WDT timer innacuracy over long period
  for (int i = 0; i < 425; i++) { 
  sleep_8();
  }

  //WAKE UP GSM MODULE
    bool disableSleep = sim800.disableSleep();
    if(disableSleep)
    Serial.println("Sleep Mode/Low Power Disabled");
  else
    Serial.println("Sleep not Disbaled");
    
}

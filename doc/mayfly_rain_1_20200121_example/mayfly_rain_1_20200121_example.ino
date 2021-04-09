#include <Wire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>


//SODAQ  libraries
#include <RTCTimer.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_PcInt_Mod.h>

String targetURL;

#define READ_DELAY 1

//RTC Timer
RTCTimer timer;

String dataRec = "";   //a string to hold the data record
int currentminute, currenthour;
long currentepochtime = 0;
float boardtemp = 0.0;

int daily_tips = 0;          //the number of tips
volatile int TipFlag = 0;     //flag indicating that a tip has occurred
volatile int AlarmFlag = 0;   //alarm flag
int midnightflag = 0;         //flag showing that it's midnight

int batteryPin = A6;
int batterysenseValue = 0;
float batteryvoltage;

#define XBEE_SleepPin 23      //  sleep pin of the Xbee 900mhz radio module

//RTC Interrupt pin
#define RTC_PIN A7
#define RTC_INT_PERIOD EveryMinute

#define SD_SS_PIN 12

//The data log file
#define FILE_NAME "RainLog.txt"

//Data header
#define LOGGERNAME "Mayfly Rain Logger"
#define DATA_HEADER "DateTime_EST,TZ-Offset,Loggertime,BoardTemp,Battery_V,RainTips"


void setup()
{
  //Initialise the serial connection
  Serial.begin(57600);   //computer connection
  Serial1.begin(9600);   //xbee 900mhz radio module in bee header
  rtc.begin();
  delay(200);
  pinMode(8, OUTPUT);                 //green LED
  pinMode(9, OUTPUT);                 //red LED

  greenred4flash();   //blink the LEDs to show the board is on

  setupLogFile();

  //Setup timer events
  setupTimer();

  //Setup sleep mode
  setupSleep();

  //Make first call
  Serial.println("Power On, running: mayfly_rain_1.ino");


     //enable the D10 pullup jumper on Mayfly solder jumper SJ12 in order for the next 2 lines to work
     pinMode(10, INPUT);    //sets pin 10 as an input
     attachInterrupt(2, pin10interrupt, LOW);    //sets up an interrupt looking for a LOW trigger
     //connect one terminal of tipping bucket to Mayfly pin D10, connect other side of bucket switch to Mayfly ground

}


void loop()
{

  //Update the timer
  timer.update();

  if (AlarmFlag == 1) {
  //  Serial.println(" DS3231 Alarm ");
    AlarmFlag = 0;

    if (currenthour == 23 && currentminute == 59)  {
        midnightflag = 1;
        digitalWrite(8, HIGH);
        dataRec = createDataRecord();
        logData(dataRec);
        assembleURL();

        delay(500);
        wakeXbee();
        delay(3000);
        sendviaXbee();
        delay(2500);
        sleepXbee();

        delay(500);
        String dataRec = "";
        digitalWrite(8, LOW);

        midnightflag = 0;
        daily_tips = 0;         //start the daily total back to 0
        delay(200);
    }

    if (currentminute % 5 == 0)  {

        digitalWrite(8, HIGH);
        dataRec = createDataRecord();
        logData(dataRec);
        assembleURL();

        delay(500);
        wakeXbee();
        delay(2000);
        sendviaXbee();
        delay(2500);
        sleepXbee();

        String dataRec = "";
        digitalWrite(8, LOW);


        delay(100);
    }   //end if minute % 5 = 0

  rtc.clearINTStatus(); //This function call is  a must to bring /INT pin HIGH after an interrupt.
  AlarmFlag = 0;

  }     //end " if alarm_flag=1"


    if (TipFlag == 1) {
      // Serial.println(" TIP! ");
       daily_tips++;
       delay(1000);
       TipFlag = 0;
    }

     //Sleep
     //Serial.println("Going to sleep");
     systemSleep();

    // Serial.println("AWAKE!");

}

void showTime(uint32_t ts)
{
  //Retrieve and display the current date/time
  String dateTime = getDateTime();
  //Serial.println(dateTime);
}

void setupTimer()
{

    //Schedule the wakeup every minute
  timer.every(READ_DELAY, showTime);

  //Instruct the RTCTimer how to get the current time reading
  timer.setNowCallback(getNow);


}

void wakeISR()
{
     AlarmFlag = 1;
}

void setupSleep()
{
  pinMode(RTC_PIN, INPUT_PULLUP);
  PcInt::attachInterrupt(RTC_PIN, wakeISR);

  //Setup the RTC in interrupt mode
  rtc.enableInterrupts(RTC_INT_PERIOD);

  //Set the sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void systemSleep()
{

  //Wait until the serial ports have finished transmitting
  Serial.flush();
  Serial1.flush();

  //The next timed interrupt will not be sent until this is cleared
  rtc.clearINTStatus();

  //Disable ADC
  ADCSRA &= ~_BV(ADEN);

  //Sleep time
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();

  //Enbale ADC
  ADCSRA |= _BV(ADEN);


}


String getDateTime()
{
  String dateTimeStr;

  //Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(rtc.now().getEpoch()));

  currentepochtime = (dt.get());    //Unix time in seconds

  currentminute = (dt.minute());
    currenthour = (dt.hour());

  //Convert it to a String
  dt.addToString(dateTimeStr);
  return dateTimeStr;
}

uint32_t getNow()
{
  currentepochtime = rtc.now().getEpoch();
  return currentepochtime;
}

void greenred4flash()     //fast blinks the LEDs 4 times
{
  for (int i=1; i <= 4; i++){
  digitalWrite(8, HIGH);
  digitalWrite(9, LOW);
  delay(50);
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
  delay(50);
  }
  digitalWrite(9, LOW);
}

void setupLogFile()
{
  //Initialise the SD card
  if (!SD.begin(SD_SS_PIN))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
    //Hang
  //  while (true);
  }

  //Check if the file already exists
  bool oldFile = SD.exists(FILE_NAME);

  //Open the file in write mode
  File logFile = SD.open(FILE_NAME, FILE_WRITE);

  //Add header information if the file did not already exist
  if (!oldFile)
  {
    logFile.println(LOGGERNAME);
    logFile.println(DATA_HEADER);
  }

  //Close the file to save it
  logFile.close();
}


void logData(String rec)
{
  //Re-open the file
  File logFile = SD.open(FILE_NAME, FILE_WRITE);

  //Write the CSV data
  logFile.println(rec);

  //Close the file to save it
  logFile.close();
}

String createDataRecord()
{
  //Create a String type data record in csv format
  String data = getDateTime();
  data += ",-5,";   //adds UTC-timezone offset (5 hours is the offset between UTC and EST)


    rtc.convertTemperature();          //convert current temperature into registers
    boardtemp = rtc.getTemperature(); //Read temperature sensor value

    batterysenseValue = analogRead(batteryPin);
    batteryvoltage = (3.3/1023.) * 4.7 * batterysenseValue;

    data += currentepochtime;
    data += ",";

    addFloatToString(data, boardtemp, 3, 1);    //float
    data += ",";
    addFloatToString(data, batteryvoltage, 4, 2);

    data += ",";
    data += daily_tips;

  //Serial.print("Data Record: ");
  //Serial.println(data);
  return data;
}


static void addFloatToString(String & str, float val, char width, unsigned char precision)
{
  char buffer[10];
  dtostrf(val, width, precision, buffer);
  str += buffer;
}



void assembleURL()
{
    targetURL = "";
    targetURL = "http://somewebsite.com/capturescript.php?";   //put a php script on a server with mySQL database to capture the data
    targetURL += "LoggerID=SL053&Loggertime=";
    targetURL += currentepochtime;
    targetURL += "&BoardTemp=";
    addFloatToString(targetURL, boardtemp, 3, 1);     //float
    targetURL += "&Battery=";
    addFloatToString(targetURL, batteryvoltage, 4, 2);     //float
    targetURL += "&RainTips=";
    targetURL += daily_tips;
    targetURL += "&Summary=";
    targetURL += midnightflag;
}


void sendviaXbee() {
     Serial1.println(targetURL);
}


void sleepXbee() {
  delay (1000);
  pinMode (XBEE_SleepPin,OUTPUT);    // put XBee to sleep
  digitalWrite(XBEE_SleepPin,HIGH);  // Setting this pin to LOW turns off the pull up resistor, thus saving precious current
}

void wakeXbee() {
  pinMode(XBEE_SleepPin,OUTPUT);   // Set the "wake-up pin" to output
  digitalWrite(XBEE_SleepPin,LOW); // wake-up XBee
  delay(500); //make sure that XBee is ready
}

void pin10interrupt()
{
   TipFlag = 1;     //a tip was detected, set the tip flag

}

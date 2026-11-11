#include <Arduino.h>
#include "RTC.h"
// #include <WiFiClient.h>
// #include <HttpClient.h>
// #include <ArduinoJson.h>

#include <NTPClient.h>
// change next line to use with another board/shield
#include <WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
#include "key.h"

#include <Adafruit_MQTT_Client.h>

void periodicCallback();

volatile bool irqFlag = false;

volatile bool ledState = false;

RTCTime ALARM_TIME_1;
RTCTime ALARM_TIME_2;

AlarmMatch match;

bool Alarm1Set = true;

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP);
// You can specify the time server pool and the offset, (in seconds)
// additionally you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

#define IO_USERNAME  "IEEEGreen"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

Adafruit_MQTT_Publish test = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/test");

Adafruit_MQTT_Subscribe testPull = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/test2");

const int led = LED_BUILTIN;
bool isHigh = LOW;

bool isTriggered = false;

void alarmTrigger(){

  AlarmMatch match;
  match.addMatchHour();
  // AlarmMatch match2;
  // match2.addMatchHour();
  // match2.addMatchSecond();
  // AlarmMatch match2;
  // match2.addMatchHour();
  // match2.addMatchSecond();

  digitalWrite(LED_BUILTIN, isHigh);
  isHigh = !isHigh;

  Alarm1Set = !Alarm1Set;
  if(Alarm1Set){
    RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_1, match);
  }
  else{
    RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_2, match);
  }
}

void minTest(){
  digitalWrite(LED_BUILTIN, isHigh);
  isHigh = !isHigh;

  isTriggered = true;
}

int NTPSync(){
  timeClient.update();
  long epochTime = timeClient.getEpochTime();
  RTCTime timeToSet = RTCTime(epochTime);
  RTC.setTime(timeToSet);
}

void setup() {
  ALARM_TIME_1.setSecond(0);
  //ALARM_TIME_1.setHour(6);
  //ALARM_TIME_1.setHour(18);
  //ALARM_TIME_1.setSecond(10);
  //ALARM_TIME_2.setHour(18);

  //Serial.begin(9600);
  Serial.begin(115200);
  WiFi.begin("UB_Devices", "goubbulls");

  while (WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.print("Connected: ");
  Serial.println(WiFi.status() == WL_CONNECTED);
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  //subscribe
  mqtt.subscribe(&testPull);

  timeClient.begin();
  delay(1000);
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getEpochTime());

  // Initialize the RTC

  RTC.begin();


  // RTC.setTime() must be called for RTC.setPeriodicCallback to work, but it doesn't matter

  // what date and time it's set to

  //RTCTime mytime(12, Month::NOVEMBER, 2025, 19, 43, 00, DayOfWeek::WEDNESDAY, SaveLight::SAVING_TIME_ACTIVE);

  NTPSync();

  minTest();

  match.addMatchSecond();
  //match.addMatchHour();
  if (!RTC.setAlarmCallback(minTest, ALARM_TIME_1, match))
  {
    Serial.println("ERROR: Initial alarm callback not set");
  }

  // if (!RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_1, match)) {
  //   Serial.println("ERROR: Initial alarm callback not set");
  // }

  digitalWrite(LED_BUILTIN, HIGH);

  //alarmTrigger();
  Serial.println("Setup Success");
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}

uint32_t x = 25;

void loop(){  
  if (isTriggered){
    isTriggered = false;
    MQTT_connect();
    Serial.print(F("\nSending val "));
    Serial.println(x);

    if (!test.publish(x++))
    {
      Serial.println(F("Failed"));
    }
    else
    {
      Serial.println(F("OK!"));
    }
    Serial.println("--read--");

    Adafruit_MQTT_Subscribe *sub = mqtt.readSubscription(5000);

    if (sub == &testPull)
    {
      Serial.println((char *)testPull.lastread);
    }
    else{
      Serial.println("failed read");
    }

    if (isHigh){
      NTPSync();
    }
    Serial.println(timeClient.getFormattedTime());
  }
}


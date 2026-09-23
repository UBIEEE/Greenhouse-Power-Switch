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

const int led = LED_BUILTIN;
bool isHigh = LOW;

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
  digitalWrite(13, isHigh);
  isHigh = !isHigh;

  Alarm1Set = !Alarm1Set;
  if(Alarm1Set){
    RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_1, match);
  }
  else{
    RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_2, match);
  }
}

// void syncTimeOverHTTP() {
//   if (WiFi.status() == WL_CONNECTED) {
//     WiFiClient c;
//     HttpClient http = HttpClient(c, "https://timeapi.io", 80);
//     // Querying an HTTP Time API
//     http.beginRequest();
//     http.get("/api/v1/time/current/utc");
    
//     int httpCode = http.responseStatusCode();
    
//     Serial.println(httpCode);
//     if (httpCode > 0) {
//       String payload = http.responseBody();
//       Serial.println(payload);

//       // // Parse the JSON payload
//       // StaticJsonDocument<512> doc;
//       // deserializeJson(doc, payload);
      
//       // // Get Unix Epoch Time (seconds since Jan 1 1970)
//       // long long epochTime = doc["unixtime"]; 
//       // Serial.print("Synchronized Epoch Time over HTTP: ");
//       // Serial.println(epochTime);
      
//       // // You can now feed this value into the standard <time.h> library 
//       // // or set your hardware Real Time Clock (RTC)
//     }
//     http.endRequest();
//   }
// }

void minTest(){
  // digitalWrite(LED_BUILTIN, isHigh);
  // digitalWrite(13, isHigh);
  // isHigh = !isHigh;

  RTCTime now;
  RTC.getTime(now);

  //Serial.print("Executed exactly on the whole minute: ");
  Serial.print(now.getHour());
  Serial.print(":");
  Serial.print(now.getMinutes());
  Serial.print(":");
  Serial.println(now.getSeconds());
}

int NTPSync(){
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

  //syncTimeOverHTTP();

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

  long epochTime = timeClient.getEpochTime();
  RTCTime timeToSet = RTCTime(epochTime);
  RTC.setTime(timeToSet);

  match.addMatchSecond();
  //match.addMatchHour();
  if (!RTC.setAlarmCallback(minTest, ALARM_TIME_1, match))
  {
    Serial.println("ERROR: Initial alarm callback not set");
  }

  // if (!RTC.setAlarmCallback(alarmTrigger, ALARM_TIME_1, match)) {
  //   Serial.println("ERROR: Initial alarm callback not set");
  // }

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(13, LOW);

  //alarmTrigger();
  Serial.println("Setup Success");
}


void loop(){ 
  delay(1000); 
  RTCTime now;
  RTC.getTime(now);

  Serial.print(now.getHour());
  Serial.print(":");
  Serial.print(now.getMinutes());
  Serial.print(":");
  Serial.println(now.getSeconds());
}

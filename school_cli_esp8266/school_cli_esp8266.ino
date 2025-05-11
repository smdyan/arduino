// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
#define ENABLE_GxEPD2_GFX 0 // enable or disable GxEPD2_GFX base class
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF
#include "user_interface.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <ArduinoJson.h> //https://arduinojson.org/v7/example/parser/
#include <TimeLib.h> //https://github.com/PaulStoffregen/Time/blob/master/TimeLib.h
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h> //https://docs.arduino.cc/language-reference/en/functions/wifi/client/
#include "secrets.h"

// ESP8266 CS(SS)=15,SCL(SCK)=14,SDA(MOSI)=13,BUSY=16,RES(RST)=5,DC=4
// 2.9'' EPD Module
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // DEPG0290BS 128x296, SSD1680

const char ssid[] = SECRET_SSID;  //  your network SSID (name)
const char pass[] = WIFI_PASS;       // your network password
String formatted_date = "2000-01-01";
String formatted_time = "00:00:00";
String wd_str = "none";
String temp_ya = "none";
String subject[6];
int lockHour = 0;

//IPAddress host(103,7,54,224);
char host[]="ars.salihof.ru";

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = 3;     // Central European Time
WiFiUDP udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
void refreshing_white();
void header_text();
void showPartialUpdate();
bool getSchedule();
void wifiConnect();
void fpm_wakup_cb_func();
void lightSleepNow();
void fpm_wakeup_man_func();
void modemSleepNow();
void showDebug(String);


void fpm_wakup_cb_func() {
  display.init(115200, false, 50, false);
  wifi_fpm_close();     // disable force sleep function
  wifi_set_opmode(STATION_MODE);     	 // set station mode
  wifi_station_connect();        	 // connect to AP
  Serial.end();
}

void fpm_wakeup_man_func()
{
 wifi_fpm_do_wakeup();
 wifi_fpm_close();  	 	 	 // disable force sleep function
 wifi_set_opmode(STATION_MODE);     	 // set station mode
 wifi_station_connect();        	 // connect to AP
 delay(200);
}

//https://kevinstadler.github.io/notes/esp8266-deep-sleep-light-sleep-arduino/
void lightSleepNow() {
  long sleepTimeMilliSeconds = 5000;
  display.hibernate();
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);       //Set the WiFi operating mode, and save it to Flash. Wifi_set_opmode has to be set to NULL_MODE before entering forced sleep mode.Disable the force sleep function by wifi_fpm_close before setting Wi-Fi mode back to normal mode. 
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();      //Enable force sleep function.
  wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func);      //Set a callback of waken up from force sleep because of time out.This API can only be called when force sleep function is enabled, after calling wifi_fpm_open.
  wifi_fpm_do_sleep(sleepTimeMilliSeconds * 1000);      //Force ESP8266 enter sleep mode, and it will wake up automatically when time out. This API can only be called when force sleep function is enabled, after calling wifi_fpm_open.ESP8266 will not enter sleep mode immediately, it is going to sleep in the system idle task. Please do not call other WiFi related function
  delay(sleepTimeMilliSeconds + 1);
  //esp_delay(sleepTimeMilliSeconds + 5, [](){ return wifi_sleeping; });
}

void modemSleepNow() {
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);      
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
  wifi_fpm_open();
  wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
}

void wifiConnect(){
  showDebug("wifi");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.flush();
}

void setup()
{
  Serial.begin(9600);
  extern os_timer_t* timer_list;
  timer_list = nullptr; // for timer-based light sleep to work, the os timers need to be disconnected
  wifiConnect();
  //setSyncProvider(getNtpTime);
  //setSyncInterval(3600);
  setTime( getNtpTime() );
  while( !getSchedule())
    delay( 100 );
  while( !getYaTemperature())
    delay( 100 );
  display.init(115200, true, 50, false);
  refreshing_white();
  header_text();
  showPartialUpdate();
  //display.display(false);
  //display.hibernate();
  showDebug("setup");
}

//time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
  if( lockHour != hour() ){
    // void fpm_wakeup_man_func();
    // void wifiConnect();
    while( !getYaTemperature())
      delay(500);
    lockHour = hour();
    showDebug("ynx");
  }
  formatted_time = String(hour() < 10 ? "0" : "") + String(hour()) + ":" +
                   String(minute() < 10 ? "0" : "") + String(minute()) + ":" +
                   String(second() < 10 ? "0" : "") + String(second());
  showPartialUpdate();
  if (formatted_date != String(year()) + "-" + String(month()) + "-" + String(day())) {
    //wificonnect();
    setTime( getNtpTime() );
    while( !getSchedule())
      delay(500);
    refreshing_white();
    header_text();
  }
  // if( wifi_get_opmode() != NULL_MODE ){
  //   modemSleepNow();
  // }
  showDebug("loop");
}

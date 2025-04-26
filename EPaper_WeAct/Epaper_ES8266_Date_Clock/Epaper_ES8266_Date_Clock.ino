
// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"

// ESP8266 CS(SS)=15,SCL(SCK)=14,SDA(MOSI)=13,BUSY=16,RES(RST)=5,DC=4

// 2.13'' EPD Module
//GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // DEPG0213BN 122x250, SSD1680
//GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // GDEY0213Z98 122x250, SSD1680

// 2.9'' EPD Module
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // DEPG0290BS 128x296, SSD1680
//GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // GDEM029C90 128x296, SSD1680

const char ssid[] = SECRET_SSID;  //  your network SSID (name)
const char pass[] = WIFI_PASS;       // your network password
String formatted_date = "2000-01-01";
String formatted_time = "00:00:00";

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = 7;     // Central European Time

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

void setup()
{
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  delay(250);
  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  display.init(115200, false, 50, false);
  header_text();
  delay(1000);
  refreshing_white();
  delay(1000);

}

time_t prevDisplay = 0; // when the digital clock was displayed

void header_text()
{
  formatted_date = String(year()) + "-" + String(month()) + "-" + String(day());
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(formatted_date, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;

  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 15);
    display.print("Date:");
    display.setCursor(0, 75);
    display.print("Time:");
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(x, 50);
    display.print(formatted_date);
  }
  while (display.nextPage());

}

void refreshing_white()
{
  // Box for time and date
  uint16_t box_w = 200;
  uint16_t box_h = 30;

  // calculate the center position
  uint16_t box_x = (display.width() - box_w) / 2;
  uint16_t box_y = ((display.height() - box_h) / 2) + 40;

  for (uint16_t r = 1; r < 15; r++)
  {
    display.setRotation(1);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    }
    while (display.nextPage());
    delay(200);
  }
}

void showPartialUpdate()
{
  //delay(100);
  // Box for time and date
  uint16_t box_w = 200;
  uint16_t box_h = 30;

  // calculate the center position
  uint16_t box_x = (display.width() - box_w) / 2;
  uint16_t box_y = ((display.height() - box_h) / 2) + 40;

  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBold18pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);

  display.setRotation(1);
  display.setPartialWindow(box_x, box_y, box_w, box_h);
  display.firstPage();
  do
  {
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);

    // Hitung posisi tengah kotak
    int16_t x = box_x + (box_w - tbw) / 2;
    int16_t y = box_y + (box_h - tbh) / 2 + tbh;

    display.setCursor(x, y);
    display.print(formatted_time);
  }
  while (display.nextPage());
}

void loop() {
  formatted_time = String(hour() < 10 ? "0" : "") + String(hour()) + ":" +
                   String(minute() < 10 ? "0" : "") + String(minute()) + ":" +
                   String(second() < 10 ? "0" : "") + String(second());
  showPartialUpdate();
  if (formatted_date != String(year()) + "-" + String(month()) + "-" + String(day())) {
    header_text();
    delay(1000);
    refreshing_white();
    delay(1000);
  }
}

/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

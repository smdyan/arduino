
// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// ESP8266 CS(SS)=15,SCL(SCK)=14,SDA(MOSI)=13,BUSY=16,RES(RST)=5,DC=4

// 2.13'' EPD Module
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // DEPG0213BN 122x250, SSD1680
//GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // GDEY0213Z98 122x250, SSD1680


void setup()
{
  display.init(115200, false, 50, false);
  header_text(); 
  delay(1000);

  showPartialUpdate(); 
  delay(1000);

  display.hibernate();
}

void header_text()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();              
  display.firstPage();                 
  do                                       
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 15);       
    display.print("Incremental");
    display.setCursor(0, 35);       
    display.print("Counting");  
  }
  while (display.nextPage());    
}

//yaitu menggambar atau memperbarui hanya bagian tertentu dari layar (bukan keseluruhan)
void showPartialUpdate()
{
  delay(1000);
  // ukuran box
  uint16_t box_w = 100;
  uint16_t box_h = 50;

  // menghitung posisi tengah
  uint16_t box_x = (display.width() - box_w) / 2; 
  uint16_t box_y = ((display.height() - box_h) / 2) + 20;

  int value = 100; 
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3; 
  display.setFont(&FreeMonoBold24pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0); 
  display.setTextColor(GxEPD_BLACK);

  //because I uses 3 colors module, refresh several times so the screen becomes white
  for (uint16_t r = 1; r < 10; r++)
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
  
  // show updates in the update box
  for (uint16_t r = 100; r < 1000; r++)
  {
    display.setRotation(1);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      // Calculate text size
      char r_str[10];
      sprintf(r_str, "%d", r);
      int16_t tbx, tby; 
      uint16_t tbw, tbh;
      display.getTextBounds(r_str, 0, 0, &tbx, &tby, &tbw, &tbh);

      // Calculate the center position of the box
      int16_t x = box_x + (box_w - tbw) / 2;
      int16_t y = box_y + (box_h - tbh) / 2 + tbh; 

      display.setCursor(x, y); 
      display.print(r_str);
    }
    while (display.nextPage());
    delay(500);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

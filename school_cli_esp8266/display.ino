void header_text()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();

  formatted_date = String(year()) + "-" + String(month()) + "-" + String(day());
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(formatted_date, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(170, 16);
    display.print(formatted_date);
    display.setCursor(170, 32);
    display.print( wd_str );
    int line_y = 16;
    for(int i=0; i<6; i++){
      display.setCursor(0, line_y);
      line_y += 16;
      display.print( subject[i] );
    }
  }
  while (display.nextPage());
  delay(200);
}

void showPartialUpdate()
{
  int16_t tbx, tby;
  uint16_t tbw = 96;
  uint16_t tbh = 32;
  uint16_t x = 170;
  uint16_t y = 40;

  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.setPartialWindow(x, y, tbw, tbh);
  
  display.firstPage();
  do
  {
    display.fillRect(x, y, tbw, tbh, GxEPD_WHITE);
    display.setCursor(x, y+12);
    display.print(formatted_time);
    display.setCursor(x, y+26);
    display.print( temp_ya + " C");
  }
  while (display.nextPage());
  delay(200);
}

void refreshing_white()
{
  for (uint16_t r = 1; r < 3; r++)
  {
    display.setRotation(1);
    display.setFullWindow();
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
    }
    while (display.nextPage());
    delay(200);
  }
}

void showDebug(String msg){
  uint16_t x = 170;
  uint16_t y = 100;
  display.setPartialWindow(x, y-10, 70, 20);
  display.setCursor(x, y);
  display.firstPage();
  do
  {
    display.fillRect(x, y-10, 70, 20, GxEPD_WHITE);
    display.print(msg);
  }
  while (display.nextPage());
  delay(100);
}
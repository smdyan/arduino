bool getSchedule(){
  Serial.begin(9600);
  delay(500);
  WiFiClientSecure client;
  JsonDocument doc;
  int wd_unix = weekday();
  if (wd_unix == 1)
    wd_unix = 8;
  int wd_int = wd_unix - 1;
  client.setInsecure();
  if( client.connect( host, 443 )){
    client.println(String("GET /school/schedule/") + String( wd_int ) + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    delay(1000);                                        
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println("Invalid response");
      delay(100);
      return false;
    }
    deserializeJson(doc, client);
    client.stop();
  }
  wd_str = String( doc["dayName"] );
  for( int i=0; i<6; i++ ){
    subject[i] = String( doc["lessons"][i] );
  }
  return true;
}

bool getYaTemperature(){
  WiFiClientSecure client_ya;
  JsonDocument doc_ya;
  client_ya.setInsecure();
  if( client_ya.connect( "yandex.com", 443 )){
    client_ya.println(String("GET /time/sync.json?geo=213") + " HTTP/1.1\r\nHost: yandex.com\r\nConnection: close\r\n\r\n");
    delay(1000);                                        
    char endOfHeaders[] = "\r\n\r\n";
    if (!client_ya.find(endOfHeaders)) {
      Serial.println("Invalid response");
      return false;
    }
    deserializeJson(doc_ya, client_ya);
    client_ya.stop();
    delay(200);
  }
  temp_ya = String( doc_ya["clocks"]["213"]["weather"]["temp"] );
  return true;
}
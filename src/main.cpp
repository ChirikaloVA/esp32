#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
//#include <LiquidCrystal_PCF8574.h>
#include "LiquidMenu.h"
#include "LiquidMenu_config.h"
#include "Button.h"
#include "html.h"
const char* host = "esp32";
const char* ssid = "SCORPION_2.4";
const char* password = "2996236S";
//variabls to blink without delay:
WebServer server(80);
void writeMCP(byte a, byte b);

#define I2C_SDA 21
#define I2C_SCL 22
#define MCPaddr 0x20
#define LCDaddr 0x27

const long interval = 5000; //interval v ms
unsigned long previousMillis = 0;
unsigned long lastMillis = 0;
bool status;

Adafruit_MCP23017 relay;
//LiquidCrystal_PCF8574 lcd(LCDaddr);
LiquidCrystal_I2C lcd(LCDaddr, 20, 4);
// LiquidMenu menu(lcd);
/*
 * Переменные, используемые для периодического выполнения кода. Первая - это период
 * в миллисекундах, а вторая - время последнего выполнения кода.
 */
unsigned int period_check = 1000;
unsigned long lastMs_check = 0;

unsigned int period_nextScreen = 5000;
unsigned long lastMs_nextScreen = 0;

/*
 * Объекты LiquidLine представляют собой отдельные строки текста и/или переменных
 * на дисплее. Первые два параметра - это столбец и строка, с которых
 * строка начинается, остальные параметры - это текст и/или переменные,
 * которые будут напечатаны на дисплее. Их количество может составлять до четырех.
 */
// Здесь строка устанавливается в столбец 1, строку 0, и будут напечатаны
// переданные строка и переменная.
//LiquidLine welcome_line1(1, 0, "LiquidMenu ", LIQUIDMENU_VERSION);
// Здесь столбец равен 3, а строка - 1, и строка "Hello Menu".
//LiquidLine welcome_line2(1, 1, "Hello Menu I2C");

/*
 * Объекты LiquidScreen представляют собой отдельные экраны. Экран формируется
 * из одного или более объектов LiquidLine. Отсюда может быть вставлено до четырех
 * объектов LiquidLine, но больше может быть добавлено позже в setup() с помощью
 * welcome_screen.add_line(someLine_object);.
 */
// Здесь два объекта LiquidLine, созданных выше.
//LiquidScreen welcome_screen(welcome_line1, welcome_line2);

// Здесь не только строка текста, но еще и изменяющаяся целочисленная переменная.
//LiquidLine analogReading_line(0, 0, "Analog: ", millis());
//LiquidScreen secondary_screen(analogReading_line);

/*
 * Объект LiquidMenu объединяет объекты LiquidScreen, чтобы сформировать меню. 
 * Здесь он только инициализируется, а экраны добавляются позже с помощью
 * using menu.add_screen(someScreen_object);. Этот объект используется для
 * управления меню, например: menu.next_screen(), menu.switch_focus()...
 */



void _set(void){
  lcd.setCursor(40, 0);
}

Button left(8);
Button right(9);
Button up(10);
Button down(11);
Button enter(12);




 
void setup() {

  
  // put your setup code here, to run once:
 Serial.begin(115200);
 // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
 Wire.setClock(1700000);
 //Wire.begin(I2C_SDA, I2C_SCL, 400000);
 relay.begin(MCPaddr, &Wire);
  //lcd.begin(20, 4); //colums - 20, rows - 4
  //lcd.init();
  lcd.clear();
  lcd.print(F("PCF8574 is OK...")); //(F()) saves string to flash & keeps dynamic memory free
  delay(2000);

  lcd.clear();

 Wire.beginTransmission(MCPaddr);
 if(Wire.endTransmission() == 0){
   lcd.print("found at addr 0x20");
   delay(2000);
 }
 else{
   Serial.println("Could not find RELAY");
   while(1);
 }

 lcd.clear();
 //lcd.setBacklight(1);
 
  //relay.begin(MCPaddr, &Wire);
  //writeMCP(0x0D, 0x1F); //настроить PORT B pullaup 100k
  Wire.beginTransmission(0x20);  // i2c – адрес (A0-0,A1-0,A2-0)
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // настроить PORT A как output
  Wire.endTransmission();
 
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  
  void (*set)();
  set = _set;
  // чтение данных из PORT B
  /* Wire.beginTransmission(0x20);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1);
  byte input=Wire.read();
  Serial.println(input, HEX);
  if (input == 0xFF)
  {
    writeMCP(0x00, 0x00);
  } */
  
 /* unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) 
  {
    lcd.setBacklight(status);
    previousMillis = currentMillis;
    status = !status;
    Wire.beginTransmission(0x20);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(0x20, 1);
    byte input=Wire.read();
    lcd.setCursor(18, 0);
    lcd.print("  ");
    lcd.setCursor(18, 0);
    lcd.print(input, HEX);
    //Serial.println(input, HEX);
    if (input == 0xFF){
      writeMCP(0x00, 0x00);
    }
    if(status == true){
      writeMCP(0x12, 0xFF);
    }
    else{
      writeMCP(0x12, 0x00);
    }
  }
   */
  /* Wire.beginTransmission(0x20);
  Wire.write(0x12); // address PORT A
  Wire.write(0xFF);    // PORT A
  Wire.endTransmission();
  delay(5000);
  
  Wire.beginTransmission(0x20);
  Wire.write(0x12); // address PORT A
  Wire.write(0x00);    // PORT A
  Wire.endTransmission();
  delay(5000); */

  // Проверить все кнопки
  if (right.check() == LOW) 
  {
    set();
    lcd.print(F("RIGHT button clicked"));
    //menu.next_screen();
  }
  if (left.check() == LOW) 
  {
    set();
    lcd.print(F("LEFT  button clicked"));
    //menu.previous_screen();
  }
  if (up.check() == LOW) 
  {
    set();
    lcd.print(F("UP    button clicked"));
    // Вызвать функцию, идентифицированную номером 1
    // для выделенной строки.
    //menu.call_function(1);
  }
  if (down.check() == LOW) 
  {
    set();
    lcd.print(F("DOWN  button clicked"));
    //menu.call_function(2);
  }
  if (enter.check() == LOW) 
  {
    set();
    lcd.print(F("ENTER button clicked"));
    // Переключить фокус на следующую строку.
    //menu.switch_focus();
  }
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) 
  {
    lcd.clear();
    previousMillis = currentMillis;
    lcd.setCursor(0, 0);
    lcd.print(lastMillis);
    lastMillis = millis();
    lcd.setCursor(20, 0);
    lcd.print(millis());  
  }
}
void writeMCP(byte a, byte b){
  Wire.beginTransmission(0x20);
  Wire.write(a);
  Wire.write(b);
  Wire.endTransmission();
}
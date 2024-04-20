#define BLYNK_TEMPLATE_ID "TMPL3R43JFgge"
#define BLYNK_TEMPLATE_NAME "BATTERY MONITORING SYSTEM"
#define BLYNK_AUTH_TOKEN "wWHtNt4k-VWaCj-LtBGbrw72Dvm__zjT"

char auth[] = "wWHtNt4k-VWaCj-LtBGbrw72Dvm__zjT";//Enter your Auth token
char ssid[] = "internet";//Enter your WIFI name
char pass[] = "password";//Enter your WIFI password

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int TLrelay= 14;
int L1relay= 26;
int L2relay= 27;

int TLstate;
int L1state;
int L2state;
int switchstate;
char c;
String x, y, z;

String dataToSend;

float B1volt;
float B2volt;
float TLamp;
float L1amp;
float L2amp;
float Steinhart;
float SOC;
float power;

unsigned long previousTime = 0;
unsigned long delayTime = 1000;

HardwareSerial MCUserial(2);

LiquidCrystal_I2C lcd1(0x27, 16, 2);
LiquidCrystal_I2C lcd2(0x3C, 16, 2);

TaskHandle_t lcdTask;  // Handle for the LCD task
SemaphoreHandle_t dataSemaphore;  // Semaphore to protect data access
/*
void lcdTaskFunction(void *parameter) {
  for (;;) {
    // Acquire the semaphore to access data
    xSemaphoreTake(dataSemaphore, portMAX_DELAY);

    // Clear LCD and display data
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("B1: ");  lcd1.print(B1volt, 1);  lcd1.print("V");
    lcd1.setCursor(0, 1);
    lcd1.print("B2: ");  lcd1.print(B2volt, 1);  lcd1.print("V");

    // Release the semaphore
    xSemaphoreGive(dataSemaphore);

    // Delay or use vTaskDelay depending on your desired timing
    vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500 milliseconds
  }
}
*/

void setup() 
{ 
  Serial.begin(9600);
  MCUserial.begin(115200);
  
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();
  lcd1.print("Calibrating...");
  lcd2.print("Calibrating...");
  WiFi.begin(ssid,pass);

      while (WiFi.status() != WL_CONNECTED) 
      {
      delay(500);
      Serial.print(".");
      }
  
  Blynk.begin(auth,ssid,pass);

  pinMode(TLrelay, OUTPUT);
  pinMode(L1relay, OUTPUT);
  pinMode(L2relay, OUTPUT);


  
  
  while (!Blynk.connected())
  {
    Blynk.run();
  }
}

void loop() 
{ 

 serialR();

unsigned long currentTime = millis();

if ( currentTime - previousTime >= delayTime )
{
    Blynk.virtualWrite( V0,B1volt);
    Blynk.virtualWrite( V1,B2volt); 
    Blynk.virtualWrite( V2, TLamp);
    Blynk.virtualWrite( V3, L1amp);
    Blynk.virtualWrite( V4, L2amp);
    Blynk.virtualWrite( V5, Steinhart);
    Blynk.virtualWrite( V6, SOC);
    Blynk.virtualWrite( V7,L1state);//l1
    Blynk.virtualWrite( V8,L2state);
    Blynk.virtualWrite( V9,TLstate);

    previousTime = millis();
    serialR ();
    serialR ();
}
      
if (Steinhart >= 40)
{

 digitalWrite(TLrelay,LOW);
 digitalWrite(L1relay,LOW);
 digitalWrite(L2relay,LOW);

 TLstate=0;
 L1state=0;
 L2state=0;
 

 lcd2.clear();
 lcd2.setCursor(0,0);
 lcd2.print("High Temperature");
 delay(1000);
}

if (L2amp >= 1)
{
 //Blynk.virtualWrite( V8,0);//l2

 digitalWrite(L2relay,LOW);
 L2state=0;

 lcd2.clear();
 lcd2.setCursor(0,0);
 lcd2.print("L2 Overload!");
 delay(1000);
}

if (L1amp >= 1)
{
 

 digitalWrite(L1relay,LOW);
 L1state=0;
 lcd2.setCursor(0,0);
 lcd2.clear();
 lcd2.print("L1 Overload!");
 delay(1000);
}

/*
if (B1volt <= 2)
{
 lcd2.setCursor(0,0);
 lcd2.clear();
 lcd2.print("Cell 1 down!");
 delay(1000);
}

if (B2volt <= 2)
{
 lcd2.setCursor(0,0);
 lcd2.clear();
 lcd2.print("Cell 2 down!");
 delay(1000);
}*/


power=(B1volt+B2volt)*(L1amp+L2amp);
if (power<1)
power=0;

lcd1.clear();
lcd1.setCursor(0,0);
lcd1.print("B1:");  lcd1.print(B1volt,1);  lcd1.print("V")  ; lcd1.print(" ");
lcd1.print("B2:");  lcd1.print(B2volt,1);  lcd1.print(" ");  lcd1.print("V");
lcd1.setCursor(0,1);
lcd1.print("TL:");  lcd1.print(TLamp,2);  lcd1.print(" ");
lcd1.print("L1:");  lcd1.print(L1amp,2);  lcd1.print(" ");

lcd2.clear();
lcd2.setCursor(0,0);
lcd2.print("L2:"); lcd2.print(L2amp,2); lcd2.print(" ");
lcd2.print("T:"); lcd2.print(Steinhart,2); lcd2.print("c"); 
lcd2.setCursor(0,1);
lcd2.print("SOC:"); lcd2.print(SOC,1); lcd2.print("%"); 
lcd2.setCursor(10,1);
lcd2.print("P:"); lcd2.print(power,1); lcd1.print("W"); 

}

void parseData(String data) {
  /*
   if (data.startsWith("#") && data.endsWith("$")) 
   {
  data = data.substring(1, data.length() - 1);
*/
  B1volt = data.substring(0, data.indexOf(',')).toFloat();
  data = data.substring(data.indexOf(',') + 1);
  B2volt = data.substring(0, data.indexOf(',')).toFloat();
  data = data.substring(data.indexOf(',') + 1);
  TLamp = data.substring(0, data.indexOf(',')).toFloat();;
  data = data.substring(data.indexOf(',') + 1);
  L1amp = data.substring(0, data.indexOf(',')).toFloat();;
  data = data.substring(data.indexOf(',') + 1);
  L2amp = data.substring(0, data.indexOf(',')).toFloat();;
  data = data.substring(data.indexOf(',') + 1);
  Steinhart = data.substring(0, data.indexOf(',')).toFloat();;
  data = data.substring(data.indexOf(',') + 1);
  SOC = data.substring(0, data.indexOf(',')).toFloat();;
  data = data.substring(data.indexOf(',') + 1);
  //power = data.substring(0, data.indexOf(',')).toFloat();;
  
  // Print the received variables
  Serial.print("Received data: ");
  Serial.print("B1v = ");
  Serial.println(SOC);
  Serial.print("B2V = ");
  Serial.println(B1volt);
  
  

  delay(100);  // Optional delay before reading the next data
}

void serialR ()
{
      if (MCUserial.available())
     {
        String receivedData = MCUserial.readStringUntil('\n');  // Read the received data
        Serial.print(receivedData);
        parseData(receivedData);  // Parse the received data
     }
}

BLYNK_WRITE(V9) 
{ 
  TLstate = param.asInt();
  digitalWrite(TLrelay, TLstate);
}

BLYNK_WRITE(V7) 
{ 
  L1state = param.asInt();
  digitalWrite(L1relay, L1state);
}

BLYNK_WRITE(V8) 
{ 
  L2state = param.asInt();
  digitalWrite(L2relay, L2state);
}

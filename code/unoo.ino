#include <SoftwareSerial.h>
//#include <ArduinoJson.h>

const int Bat1 = A5;
const int Bat2 = A4;
const int TL = A3;
const int L2 = A2;
const int L1 = A1;
const int Thermistor = A0;
const int trigger = 12;

const float thermistorResistance = 10000;  // Resistance at nominal temperature (in ohms)
const float thermistorNominalTemp = 25;   // Nominal temperature (in degrees Celsius)
const float thermistorBeta = -3950;        // Beta coefficient of the thermistor


SoftwareSerial UNOserial( 10 ,11 );

const int numEntries = 30;
const float batteryVoltage[numEntries] = {
  8.25, 8.20, 8.15, 8.10, 8.05, 8.00, 7.95, 7.90, 7.85, 7.80,
  7.75, 7.70, 7.65, 7.60, 7.55, 7.50, 7.45, 7.40, 7.35, 7.30,
  7.25, 7.20, 7.15, 7.10, 7.05, 7.00, 6.95, 6.90, 6.85, 6.80
};
const int stateOfCharge[numEntries] = {
  100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
  50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
  8, 7, 6, 5, 4, 3, 2, 1, 0, 0
};
//StaticJsonDocument<200> dataR;


float B1volt;
float B2volt;
float B2svolt;

float TLamp;
float L1amp;
float L2amp;
float SOC;
float capacity = 2.5*3600;
float acs1err=0;
float acs2err=0;
float acs3err=0;
float resistance;
float steinhart;
float power;

String TLstate;
String L1state;
String L2state;

String dataToSend;
String receivedData;

float m,k=0,TLampfn ,Q,t ;
int a;



void setup() 
{ 
  Serial.begin(9600);
  UNOserial.begin(115200);
  pinMode(Bat1, INPUT);
  pinMode(Bat2, INPUT);
  pinMode(TL, INPUT);
  pinMode(L2, INPUT);
  pinMode(L1, INPUT);

  delay(5000);
  calibrateTL();
  calibrateL1();
  calibrateL2();
  
  B2volt = analogRead(Bat2) * 9.7656e-3 - 0.2;
  SOC=getSOC(B2volt);
  Q=capacity*(SOC/100);
  delay(5000);

  
}

void loop()
{
  B1volt = analogRead(Bat1) * 7.3242e-3;
  B2svolt = analogRead(Bat2) * 9.7656e-3 - 0.2;
  B2volt = abs(B2svolt-B1volt);


  //Serial.println(B1volt);
  //Serial.println(B2volt);


  
  for (a=0;a<=100;a++)
          {   t = (      analogRead(TL) * (5.0 / 1024.0)    );
              m = ( acs1err - t ) /0.185;
              k+=m;
              delay(0);
          }
        
          TLamp = abs(k/100 - 0.15) ;
          k=0;
  
  for (a=0;a<=100;a++)
          {
              t = (      analogRead(L1) * (5.0 / 1024.0)    );
              m = ( acs2err - t ) /0.185;
              k+=m;
              delay(0);
          }
        
          L1amp = abs(k/100 - 0.15) ;
          k=0;

  for (a=0;a<=100;a++)
          {
              t = (      analogRead(L2) * (5.0 / 1024.0)    );
              m = ( acs3err - t ) /0.185;
              k+=m;
              delay(0);
          }
        
          L2amp = abs(k/100 - 0.15) ;
          k=0;
  temp();
  soc();  

  power=B2svolt*(L1amp+L2amp);

  dataToSend = String(B1volt) + "," + String(B2volt) + "," + String(TLamp)+ "," +
               String(L1amp)+ "," + String(L2amp)+ "," + String(steinhart) + "," + String(SOC); //+","+String(power);
  UNOserial.println(dataToSend);
  Serial.println(dataToSend);
  delay(300);
}

  void soc() 
  {  
    
      for (a=0;a<=100;a++)
          {
              t = (      analogRead(TL) * (5.0 / 1024.0)    );
              m = ( acs1err - t ) /0.185;
              k+=m;
              delay(0);
          }
        
          TLampfn = ( k/100 - 0.15 )*-1 ;
          k=0;
          Serial.println(TLampfn);
    if (abs(TLampfn) >= .1)
    {

    Q = Q + TLampfn;
    SOC = (Q/capacity)*100;
    delay(1);

    }
    //Serial.println(m);
   // Serial.println(TLampfn);
    
  }

  void temp()
  {
  
      resistance = thermistorResistance * (1023.0 /analogRead(Thermistor)  - 1.0  );

      // Calculate the temperature using the Steinhart-Hart equation
      steinhart = resistance / thermistorResistance;                      // (R/Ro)
      steinhart = log(steinhart);                                         // ln(R/Ro)
      steinhart /= thermistorBeta;                                        // 1/B * ln(R/Ro)
      steinhart += 1.0 / (thermistorNominalTemp + 273.15);                // + (1/To)
      steinhart = 1.0 / steinhart;                                        // Invert
      steinhart -= 273.15;                                                 // Convert to Celsius

  }

  void calibrateTL()
  {      
          //Serial.println("cbrtng.....");

          for (a=0;a<=100;a++)
          {
          m=analogRead(TL) * (5.0 / 1024.0);
          k+=m;
          }
          acs1err=k/100;
          k=0;
          //Serial.println(acs1err);
  }
  void calibrateL1()
     {      
          for (a=0;a<=100;a++)
          {
          m=analogRead(L1) * (5.0 / 1024.0);
          k+=m;   
          } 
          acs2err=k/100;
          k=0;
          //Serial.println(acs1err);
     }

void calibrateL2()
     {  
          for (a=0;a<=100;a++)
          {
          m=analogRead(L2) * (5.0 / 1024.0);
          k+=m;
          }
          acs3err=k/100;
          k=0;
          Serial.println(acs1err);

     }

int getSOC(float voltage) {
  // Find the corresponding SoC from the lookup table based on battery voltage
  for (int i = 0; i < numEntries; i++) {
    if (voltage >= batteryVoltage[i]) {
      return stateOfCharge[i];
    }
  }
  return 0;  // Default to 0% SoC if voltage is below the lookup table range
}


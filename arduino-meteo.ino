#include "RTClib.h"
#include <Adafruit_BMP085_U.h>
#include <Adafruit_HTU21DF.h>
#include <Ethernet.h>
#include <EEPROMex.h>
#include <ICMPPing.h>
#include <util.h>
#include <MemoryFree.h>
#include "Xarxa.h"
#include "Sensors.h"
#include "Rellotge.h"
#include "Utilitats.h"
#include "Temporitzador.h"

// Temps
RTC_DS1307 rtc;
DateTime now;

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

//Debug
//String cadena;
byte numEntradesBolcat=0;

// Estructura per guardar les dades meteo
struct METEO
{
  byte commit;
  byte any;
  byte mes;
  byte dia;
  byte hora;
  byte minut;
  int temperatura;              //la variable guarda dos posicions decimals. Dividir per 100
  int humitat;                  //la variable guarda dos posicions decimals. Dividir per 100
  int pressio;                  //la variable guarda una posició decimal. Dividir per 10
  int pluja;                    //la variable guarda dos posicions decimals. Dividir per 100
  int direcciovent;             //la variable guarda dos posicions decimals. Dividir per 100
  int mitjanavelocitatvent;     //la variable guarda dos posicions decimals. Dividir per 100
  int maximavelocitatvent;      //la variable guarda dos posicions decimals. Dividir per 100
};

METEO meteo;
int error;

byte lecturesVentHora=0;
float sumaVelocitatVentHora=0;
float maximaVelocitatVentHora=0;

byte lecturesVentMinut=0;
float sumaVelocitatVentMinut=0;
float maximaVelocitatVentMinut=0; 

const byte ledPIN = 9;
bool estat=HIGH;

bool pasFet=true;

void setup() {
  cli(); //Desactivo interrupcions
  Serial.begin(9600);
  pinMode(ledPIN, OUTPUT); //Llum led d'estat

  //Serial.println("OK");
  
  // Activo com a màxim només 30000 escritures  per protecció del EEPROM
  EEPROM.setMaxAllowedWrites(30000);
  EEPROM.setMemPool(0, 1024);
  
  //Serial.print("Direcció EEPROM=");
  //Serial.println(proximaDireccioEEPROM());
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, dnsServer, gateway, subnet);
  server.begin();
  
  //Serial.println("Pressure Sensor Test"); Serial.println("");
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    //Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
 //   while(1);
  }

  //Serial.println("HTU21D-F test");

  if (!htu.begin()) {
    //Serial.println("Couldn't find sensor!");
//    while (1);
  }
  
  /*Pluja*/
  pinMode(interruptPinPluja, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinPluja), plujaIRQ, FALLING);

    /*Anemòmetre*/
  pinMode(interruptPinAnemometre, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinAnemometre), anemometreIRQ, FALLING);

  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    //Serial.println("RTC is NOT running!");
  }

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //Dono 5 segons de temps per a que tot s'activi
  delay(5000);
  
  now = rtc.now();
  while (now.day() == 165) //comprovo error típic del RTC
  {
    delay(1);
    now = rtc.now();
  }
  segonAnterior=now.second();
  minutAnterior=now.minute();
  horaAnterior=now.hour();
  
  gosdeguaitaSetup();//Activo el gos de guaita
  sei();//Activo interrupcions
}

void desarEEPROM(){
  int direccio=0;
  direccio=proximaDireccioEEPROM();
  
  meteo.commit=0;
  meteo.any=now.year()%100;
  meteo.mes=now.month();
  meteo.dia=now.day();
  meteo.hora=now.hour();
  meteo.minut=now.minute();
  //meteo.temperatura=int(getTemperatura()*100);    //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.temperatura=int(getTemperatura2()*100);    //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.humitat=int(getHumitat()*100);            //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.pressio=int(getPressio()*10);            //la variable guarda una posició decimal. Multiplicar per 10
  meteo.pluja=int(getPluja()*100);                //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.direcciovent=int(getDireccioVent()*100);  //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.mitjanavelocitatvent=int(getMitjanaVelocitatVentHora()*100);  //la variable guarda dos posicions decimals. Multiplicar per 100
  meteo.maximavelocitatvent=int(getMaximaVelocitatVentHora()*100);  //la variable guarda dos posicions decimals. Multiplicar per 100
  
  if (EEPROM.updateBlock(direccio, meteo))
  {  
    direccio=direccio+sizeof(meteo);
    if (direccio>1001) //lliure a partir de la direcció 1020 inclosa
    {
      direccio=0;
    }
    EEPROM.updateInt(1020,direccio); //guardem a la direcció 1020 la pròxima direcció EEPROM lliure
  }    
}

float getMaximaVelocitatVentHora()
{
  float vent=0;
  vent=maximaVelocitatVentHora*3600/1000; //passo els m/s a km/h
  return vent;
}

float getMitjanaVelocitatVentHora()
{
  float vent=0;
  vent=sumaVelocitatVentHora/lecturesVentHora;
  return vent;  
}

float getMaximaVelocitatVentMinut()
{
  float vent=0;
  vent=maximaVelocitatVentMinut*3600/1000; //passo els m/s a km/h
  return vent;
}

float getMitjanaVelocitatVentMinut()
{
  float vent=0;
  vent=sumaVelocitatVentMinut/lecturesVentMinut*3600/1000; //passo els m/s a km/h
  return vent;  
}

int proximaDireccioEEPROM()
{
  int direccio=0;
  direccio=EEPROM.readInt(1020);
  return direccio;
}

void loop() {
  //Reinicio el gos de guaita
  wdt_reset();

  // put your main code here, to run repeatedly:
  now = rtc.now();
  while (now.day() == 165) //comprovo error típic del RTC
  {
    delay(1);
    now = rtc.now();
  }
  segonActual=now.second();
  minutActual=now.minute();
  horaActual=now.hour();
 
  if (segonActual>59)
  {
    segonActual=segonAnterior;
    error=error+1;
  }
  if (minutActual>59)
  {
    minutActual=minutAnterior;
    error=error+1;
  }
  if (horaActual>23)
  {
    horaActual=horaAnterior;
    error=error+1;
  }
  
  servidor_WEB();
  
  if (segonActual>segonAnterior)
  {
    segonAnterior=segonActual;
    cada_Segon();
  }
  
  if (minutActual>minutAnterior)
  {
    minutAnterior=minutActual;
    segonAnterior=0;
    cada_Segon();
    cada_Minut();
  }
  
  if (horaActual>horaAnterior)
  {
    horaAnterior=horaActual;
    segonAnterior=0;
    minutAnterior=0;
    cada_Segon();
    cada_Minut();
    cada_Hora();
    pasFet=false;
  }

  if (horaActual == 0 && minutActual == 0 && segonActual == 0) 
  {
    segonAnterior=0;
    minutAnterior=0;
    horaAnterior=0;
    cada_Segon();
    cada_Minut();
    cada_Hora();
    pasFet=false;
  }

  if (horaActual%2 == 0 && pasFet==false) 
  {
    bolcat_de_EEPROM_a_inet();
    pasFet=true;
  }

}

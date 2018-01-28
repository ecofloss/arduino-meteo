#include "RTClib.h"
#include <Adafruit_BMP085_U.h>
#include <Adafruit_HTU21DF.h>
#include <Ethernet.h>
#include <EEPROMex.h>
#include <ICMPPing.h>
#include <util.h>
#include <MemoryFree.h>


//Xarxa
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(0,0,0,0);
IPAddress subnet(0,0,0,0);
IPAddress gateway(0,0,0,0);
IPAddress dnsServer(0,0,0,0);
IPAddress meteoServer(0,0,0,0);
IPAddress pingAddr(0,0,0,0); // ip address to ping
SOCKET pingSocket = 0;
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));
EthernetServer server(80);
EthernetClient client;

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

//Variables Pluja
const byte interruptPinPluja = 3;
volatile unsigned long plujaMarcaTemps, plujaMarcaTempsUltim, plujaInterval;
volatile float pluja = 0;

//Variables Anemòmetre
const byte interruptPinAnemometre = 2;
volatile unsigned long ventMarcaTemps, ventMarcaTempsUltim, ventInterval;
byte contadorAnemometre = 0;
byte voltesAnemometre = 0;
float velocitatVent = 0;

byte lecturesVentHora=0;
float sumaVelocitatVentHora=0;
float maximaVelocitatVentHora=0;

byte lecturesVentMinut=0;
float sumaVelocitatVentMinut=0;
float maximaVelocitatVentMinut=0; 

//Variables direcció vent
const byte pinDireccioVentAnalogic = 0;

int segonActual, segonAnterior, minutActual, minutAnterior, horaActual, horaAnterior;
const byte ledPIN = 9;
bool estat=HIGH;

bool pasFet=true;

void setup() {
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
}

float getPressio(){
  sensors_event_t event;
  bmp.getEvent(&event);
  return event.pressure+32;
}

float getTemperatura(){
  return htu.readTemperature();
}

float getTemperatura2(){
    float temperature;
    bmp.getTemperature(&temperature);
    return temperature;
}

float getHumitat(){
  return htu.readHumidity();
}

float getPluja(){
  return pluja;
}

//Funció a executar quant es rep l'interrupció del sensor de pluja 
void plujaIRQ()
{
    plujaMarcaTemps = millis();  
    plujaInterval = plujaMarcaTemps - plujaMarcaTempsUltim;

    if (plujaInterval > 10)
    {
        pluja+=0.2794;
        plujaMarcaTempsUltim = plujaMarcaTemps;
    }

}

//Funció a executar quant es rep l'interrupció de l'anemòmetre
void anemometreIRQ()
{
  ventMarcaTemps = millis();  
  ventInterval = ventMarcaTemps - ventMarcaTempsUltim;

  if (ventInterval > 20)
  {
    contadorAnemometre=contadorAnemometre+1;
    if (contadorAnemometre=2)
    {
      contadorAnemometre=0;
      voltesAnemometre=voltesAnemometre+1;
    }  
    ventMarcaTempsUltim = ventMarcaTemps;
  }
  
}

//Funció per obtenir la direcció del vent
float getDireccioVent()
{
  unsigned int lecturaDireccioVent;

  lecturaDireccioVent = analogRead(pinDireccioVentAnalogic);

  if (lecturaDireccioVent < 70) return (112.5);
  if (lecturaDireccioVent < 88) return (67.5);
  if (lecturaDireccioVent < 97) return (90);
  if (lecturaDireccioVent < 130) return (157.5);
  if (lecturaDireccioVent < 188) return (135);
  if (lecturaDireccioVent < 248) return (202.5);
  if (lecturaDireccioVent < 291) return (180);
  if (lecturaDireccioVent < 410) return (22.5);
  if (lecturaDireccioVent < 465) return (45);
  if (lecturaDireccioVent < 603) return (247.5);
  if (lecturaDireccioVent < 634) return (225);
  if (lecturaDireccioVent < 706) return (337.5);
  if (lecturaDireccioVent < 789) return (0);
  if (lecturaDireccioVent < 831) return (292.5);
  if (lecturaDireccioVent < 890) return (315);
  if (lecturaDireccioVent < 948) return (270);
  return (0);
}

void cada_Segon(){
  velocitatVent=voltesAnemometre*0.667; // 2.4km/h=0.667m/s
  sumaVelocitatVentMinut=sumaVelocitatVentMinut+velocitatVent;
  lecturesVentMinut=lecturesVentMinut+1;

  if (velocitatVent>maximaVelocitatVentMinut)
  {
    maximaVelocitatVentMinut=velocitatVent;
  }
  
  voltesAnemometre=0;
  velocitatVent=0;
  
  estat=!estat;
  digitalWrite(ledPIN, estat);
}

void cada_Minut(){
  float ventMinut=0;
  ventMinut=getMitjanaVelocitatVentMinut();
  
  if (maximaVelocitatVentMinut>maximaVelocitatVentHora)
  {
    maximaVelocitatVentHora=maximaVelocitatVentMinut;
  }

  maximaVelocitatVentMinut=0;
  sumaVelocitatVentMinut=0;
  lecturesVentMinut=0;
  
  lecturesVentHora=lecturesVentHora+1;
  sumaVelocitatVentHora=sumaVelocitatVentHora+ventMinut;
  //creaCadena();
  //Serial.println(debug());
}

void cada_Hora(){
  desarEEPROM();
  //Serial.println(cadena);
  
  lecturesVentHora=0;
  sumaVelocitatVentHora=0;
  maximaVelocitatVentHora=0;
  pluja=0;
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

String formata00(byte numero)
{
  String car="";
  if (numero<10)
  {
    car="0";
  }
  car.concat(numero);
  return car;
}

String formataPuntComa(float numero)
{
  String car="";
  car.concat(numero);
  car.replace('.',',');
  return car;  
}

void servidor_WEB()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("Got a client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // print the current readings, in HTML format:
          //client.print(cadena);
          client.println("<br />");
          client.print(debug());
          client.println("<br />");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

String debug()
{
  String cad="";
  cad.concat("numEntradesBolcat=");
  cad.concat(numEntradesBolcat);
  cad.concat("freeMemory=");
  cad.concat(freeMemory());
  cad.concat(";");
  cad.concat(segonActual);
  cad.concat(";");
  cad.concat(minutActual);
  cad.concat(";");
  cad.concat(horaActual);
  cad.concat(";");
  cad.concat(segonAnterior);
  cad.concat(";");
  cad.concat(minutAnterior);
  cad.concat(";");
  cad.concat(horaAnterior);
  cad.concat(";");
  cad.concat("DireccioEEPROM=");
  cad.concat(proximaDireccioEEPROM());
  cad.concat(";");
  cad.concat("NúmeroErrors=");
  cad.concat(error);
  cad.concat(";");

  cad.concat(now.year());
  cad.concat(formata00(now.month()));
  cad.concat(formata00(now.day()));
  cad.concat(formata00(now.hour()));
  cad.concat(formata00(now.minute()));
  cad.concat(formata00(now.second()));
  cad.concat(";");
  cad.concat(formataPuntComa(getTemperatura()));
  cad.concat(";");
  cad.concat(formataPuntComa(getHumitat()));
  cad.concat(";");
  cad.concat(formataPuntComa(getPressio()));
  cad.concat(";");
  cad.concat(formataPuntComa(getTemperatura2()));
  cad.concat(";");
  cad.concat(formataPuntComa(getPluja()));
  cad.concat(";");
  cad.concat(formataPuntComa(getDireccioVent()));
  cad.concat(";");
  cad.concat(formataPuntComa(getMitjanaVelocitatVentMinut()));
  cad.concat(";");
  cad.concat(formataPuntComa(getMaximaVelocitatVentMinut()));
  return cad;
}

void bolcat_de_EEPROM_a_inet()
{
  METEO meteoEEPROM;
  String cad;
  numEntradesBolcat=numEntradesBolcat+1;
  ICMPEchoReply echoReply = ping(pingAddr, 1);
  if (echoReply.status == SUCCESS)
  {
    for (int direccio=0;direccio<1001;direccio=direccio+sizeof(meteo))
    {
      EEPROM.readBlock(direccio, meteoEEPROM);
      if (meteoEEPROM.commit==0) // No s'ha bolcat el registre a internet
      {
        cad="";
        cad.concat("GET /meteo/m3t304rd11n0.php?cadena=");
        cad.concat(meteoEEPROM.any);
        cad.concat(formata00(meteoEEPROM.mes));
        cad.concat(formata00(meteoEEPROM.dia));
        cad.concat(formata00(meteoEEPROM.hora));
        cad.concat(formata00(meteoEEPROM.minut));
        cad.concat("00");
        cad.concat(";");
        cad.concat((float)meteoEEPROM.temperatura/100);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.humitat/100);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.pressio/10);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.pluja/100);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.direcciovent/100);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.mitjanavelocitatvent/100);
        cad.concat(";");
        cad.concat((float)meteoEEPROM.maximavelocitatvent/100);
        cad.concat(" HTTP/1.1");

        if (client.connect(meteoServer, 80))
        {
          client.println(cad);
          client.println("Host: www.botocasademont.name");
          client.println("Connection: close");
          client.println();
          client.flush();
          client.stop();

          //Marquem que el registre ja està guardat a internet
          meteoEEPROM.commit=1;
          EEPROM.updateBlock(direccio, meteoEEPROM);
        
        }
        //else
        //{
          // if you didn't get a connection to the server:
        //  Serial.println("connection failed");
        //}
      }
    }
  }
}

void loop() {
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

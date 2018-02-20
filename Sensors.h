#include <Arduino.h>

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

//Variables direcció vent
const byte pinDireccioVentAnalogic = 0;

float getPressio();
float getTemperatura();
float getTemperatura2();
float getHumitat();
float getPluja();
float getDireccioVent();


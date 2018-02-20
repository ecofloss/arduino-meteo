//#include "Sensors.h"

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


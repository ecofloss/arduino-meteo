
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



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


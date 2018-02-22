
void gosdeguaitaSetup(void)
{
//Desactivo interruptions
cli();

//Reinicio el gos de guaita
wdt_reset();

/*
Configuració:
WDIE = 1: Interrupt Enable
WDE = 1 :Reset Enable
WDP3 = 1 : 8 segons de guaita
WDP2 = 0 : 8 segons de guaita
WDP1 = 0 : 8 segons de guaita
WDP0 = 1 : 8 segons de guaita
*/
//Entro en el mode de configuració del gos de guaita
WDTCSR |= (1<<WDCE) | (1<<WDE);

//Configuro el gos de guaita
WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);

//Activo interruptions
sei();
}

ISR(WDT_vect)
{
// Include your interrupt code here.
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


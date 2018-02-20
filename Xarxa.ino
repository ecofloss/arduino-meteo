//#include "Xarxa.h"

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



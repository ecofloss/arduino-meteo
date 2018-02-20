#include <Arduino.h>

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

void servidor_WEB();
void bolcat_de_EEPROM_a_inet();

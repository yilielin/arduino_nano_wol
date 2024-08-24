#include <EtherCard.h>

#define STR_PROG_VER "1.0.0"
#define STR_AUTHOR "Author: SnailYilie"

//Command list
#define CMD_WOL  'w'
#define CMD_INFO 'i'
#define CMD_HELP '?'

// ethernet interface
uint8_t Ethernet::buffer[700]; // configure buffer size
uint8_t g_srcIp[4]     = { 169, 254, 96, 100 };
uint8_t g_srcMac[6]    = { 0x30, 0x24, 0xA9, 0x2D, 0x30, 0x31 };
uint8_t g_gatewayIp[4] = { 169, 254, 96, 1 };
uint8_t g_subnet[4]    = { 255, 255, 0, 0 };
uint8_t g_destIp[4]    = { 255, 255, 255, 255 };  // Broadcast in this case
uint8_t g_destMac[6]   = { 0x5C, 0x60, 0xBA, 0x5A, 0xD8, 0x1F }; // Dest PC: HP victus15
uint32_t g_wolUdpPort  = 40000;
uint8_t g_magicPacket[102];  // (0xFF * 6)+ (6 bytes MAC * 16), without passwd

void setup() {
  Serial.begin(115200);
  // motd print
  Serial.print(F("\r\n----- Wake on LAN Tool "));
  Serial.print(F(STR_PROG_VER));
  Serial.print(F(" -----\r\n"));
  Serial.println(F(STR_AUTHOR));
  // Init NIC
  if (!ether.begin(sizeof Ethernet::buffer, g_srcMac))
  {
    // handle failure to initiate network interface
    Serial.println(F("Access Ethernet controller failed."));
    Serial.println(F("Stop the program."));
    while (1); //CPU Trap
  }
  ether.staticSetup(g_srcIp, g_gatewayIp, NULL, g_subnet);
  printWolConfig();
  Serial.println(F("Send Wol(UDP) in 2 secs..."));
  Serial.println(F("--------"));
  delay(2000); //wait for NIC is ready
  sendWolUdp(g_destIp, g_destMac);
  Serial.println(F("Send '?' to get the help."));
}

void loop() {
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case CMD_WOL:
        sendWolUdp(g_destIp, g_destMac);
        break;
      case CMD_INFO:
        printWolConfig();
        break;
      case CMD_HELP:
        printHelp();
        break;
    }
  }

} // void loop

void printHelp() {
  Serial.println(F("Command List:"));
  Serial.print(CMD_WOL);
  Serial.println(F(" - Invite WOL"));
  Serial.print(CMD_INFO);
  Serial.println(F(" - Current configuration"));
  Serial.print(CMD_HELP);
  Serial.println(F(" - help"));
  Serial.print(F("\r\n"));
}

void serialPrintIp(const uint8_t* ip) {
  static char strIp[16];
  sprintf(strIp, "%d.%d.%d.%d\0", ip[0], ip[1], ip[2], ip[3]);
  Serial.print(strIp);
}

void serialPrintMac(const uint8_t* mac) {
  static char strMac[30];
  sprintf(strMac, "%02X-%02X-%02X-%02X-%02X-%02X\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(strMac);
}

void sendWolUdp(uint8_t* destIp, uint8_t* destMac) {
  Serial.println(F("\r\nSend WOL magic packet (UDP)"));
  printWolConfig();

  uint8_t* magicPacket_p = g_magicPacket;
  // Create synchronization stream
  for (uint8_t cnt = 0; cnt < 6; cnt++) {
    *magicPacket_p++ = 0xff;
  }
  // Copy MAC from g_destMac to magicPacket
  const uint8_t macSize = sizeof(g_destMac) / sizeof(g_destMac[0]);
  for (uint8_t cnt = 0; cnt < 16; cnt++) {
    memcpy(magicPacket_p, g_destMac, macSize);
    magicPacket_p += macSize;
  }
  ether.copyIp(ether.hisip, g_destIp);
  ether.sendUdp(g_magicPacket, sizeof(g_magicPacket), g_wolUdpPort, ether.hisip, g_wolUdpPort);
  // Print payload
  static char strPayload[20];
  Serial.print(F("\r\nUDP Payload:"));
  for (uint8_t i = 0; i < sizeof(g_magicPacket); i++) {
    if ( (i & 0x0F) == 0) {
      Serial.print(F("\r\n"));
    }
    sprintf(strPayload, "%02X ", (uint8_t)g_magicPacket[i]);
    Serial.print(strPayload);
  }
  Serial.print(F("\r\n\n"));
} // sendWolUdp

void printWolConfig() {
  // IP
  Serial.print(F("Source IP:       "));
  serialPrintIp(g_srcIp);
  Serial.print(F("\r\n"));
  Serial.print(F("Destination IP:  "));
  serialPrintIp(g_destIp);
  Serial.print(F("\r\n"));
  // MAC
  Serial.print(F("Source MAC:      "));
  serialPrintMac(g_srcMac);
  Serial.print(F("\r\n"));
  Serial.print(F("Destination MAC: "));
  serialPrintMac(g_destMac);
  Serial.print(F("\r\n"));
  // Port
  Serial.print(F("UDP port: "));
  Serial.print(g_wolUdpPort, DEC);
  Serial.print(F("\r\n"));
} // printWolConfig

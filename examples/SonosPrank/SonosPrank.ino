/*

 This example connects to an encrypted Wifi network.

Then uses the SonosUpnP library to scan for sonos devices,
If exsist, prints Sonos infor of the first Device found

Created JV - 2020

 */
#include <WiFiNINA.h>
#include <SonosUPnP.h>      // Adapted Library : Fork for WifiNINA support and MetaData Attrribute parsing

#include "arduino_secrets.h"  ///////please enter your sensitive data in the Secret tab/arduino_secrets.h

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Setup Sonos Variables and settings
#define MAXSONOS 6
WiFiClient G_Sonosclient;                               // setup wifiClient for Sonos Control
SonosUPnP G_Sonos = SonosUPnP(G_Sonosclient);           // Setup SonosUpnp Device G_Sonos
IPAddress ACTIVE_sonosIP,G_SonosIPList[MAXSONOS]  ; 
const char BadMusic[] = {"The Beatles"};
  
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

  // Setup Sonos connections
  //G_Sonos.CheckUPnP(G_SonosIPList,MAXSONOS);
   if ( G_Sonos.CheckUPnP(G_SonosIPList,MAXSONOS)!=0 )                            // check Sonos IP's on network , short scan, scan twice for large networks
   {
    ACTIVE_sonosIP = G_SonosIPList[0];
    Serial.print("\nActive Sonos ");Serial.println(ACTIVE_sonosIP);
    Serial.print("\nTry to Play ");Serial.println(BadMusic);
   }
   else {
    Serial.println("\nNo UPnP found.");while(1);
   }
}

void loop() {
  char text[128];
  FullTrackInfo info;
  
   info= G_Sonos.getFullTrackInfo(ACTIVE_sonosIP); // Rest full track info
   Serial.println();Serial.print(info.album);
   Serial.print(" - ");Serial.println(info.creator);   Serial.print(info.number);
   Serial.print(". ");Serial.print(info.title);   Serial.print("  ");Serial.println(info.position);

 if (strcmp(info.creator, BadMusic) == 0) G_Sonos.skip(ACTIVE_sonosIP,SONOS_DIRECTION_FORWARD);
   
  delay(5000);
  
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

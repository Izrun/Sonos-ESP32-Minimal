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
   if ( G_Sonos.CheckUPnP(G_SonosIPList,MAXSONOS)!=0 )                            // check Sonos IP's on network 
   {
    ACTIVE_sonosIP = G_SonosIPList[0];
    Serial.print("\nActive Sonos ");Serial.print(ACTIVE_sonosIP);
   }
   else {
    Serial.print("\nNo UPnP found.");while(1);
   }
}

// EXAMPLE LOOP TO SHOW INFO AVAILABLE
void loop() {
  char text[128];
  FullTrackInfo info;
  SonosInfo info2;

  info2= G_Sonos.getSonosInfo(ACTIVE_sonosIP); // Request Sonos Device info
  Serial.print("\nActive Sonos IP ");Serial.println(ACTIVE_sonosIP);
  Serial.print("Sonos Type ");Serial.print(info2.seriesid);
  Serial.print(", Serial-[");Serial.print(info2.serial);
  Serial.print("] ");Serial.print(info2.status);
  Serial.print(" in Zone ");Serial.print(info2.zone);
  Serial.print(" via ");Serial.println(info2.medium);

  Serial.print("Playing from source ");Serial.print(info2.source);
  Serial.print(", using play mode ");Serial.println(info2.playmode);
  Serial.print("\n");

while(1) {
   info= G_Sonos.getFullTrackInfo(ACTIVE_sonosIP); // Resd full track info
   Serial.print("\n");Serial.print(info.album);
   Serial.print(" - ");Serial.println(info.creator);
   Serial.print(info.number);
   Serial.print(". ");Serial.print(info.title);
   Serial.print("  ");Serial.println(info.position);
   delay(10000);
  }
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

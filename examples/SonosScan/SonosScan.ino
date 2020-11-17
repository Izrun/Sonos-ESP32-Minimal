/*
Sonos Network Test - build for MKR1010 / WIFININA using the uBlox NINA-W10 modules

THIS PROGERAM CREATES A SONOS HTTP-HUB

1. Easywifi:  Initialisation of Wifi network , including AccessPoint 'Sonoscontrol' for entering your wifi SSID/Pass by Smartphone
   Makes use of ESP32 Storage for storing your last SSID/Password and your last active Sonos Device in Flash
2. Sonos.h: Scan for Sonos devices using SSDP over UDP.
   Checks Sonos detaildata by HTTP :1400 commands and Sonos Xpath Library (XML parser)
3. Setup WebServer to check status and modify/scan for Sonos details per http page


Coding Libraries/  3rd party libraries: 
Wifi : WIFININA by Arduino
EasyWiFi: by JayV
Sonos Control : Written by Thomas Mittet (code@lookout.no)  - adapted version to be used with wifi wireless, extended functions and types and parser tweak
MicroXpath : Written by Thomas Mittet (code@lookout.no), used in Sonos Control for XML parsing HTTP:1400 packet feedback

Code style:
GLOBAL variables start with 'G_' or with 'ACTIVE'
LOCAL variables are lowercase words
DEBUG_X  switch : for general debug by serial print

(C) jAY fOX 2020 / 
*************************************************************/
#include <WiFiNINA.h>
#include <SonosUPnP.h> // Adapted Library : Fork for WifiNINA and MetaData Attrribute parsing
#include <MicroXPath.h>
#include <WiFiUdp.h>
#include <EasyWiFi.h>
#define DEBUG_X 1   // Show Debug info on Serial Channel

//
//Define WebServerSettings
//
#define SERVER_PORT 80                // Local HTTP Server Port
#define VERSION "1.2.0"               // Software verson nr
#define MAX_MISSED_DATA 2000          // MAX data missed from Client/Web HTTP reply before time-out (accept short messages only)
#define MAXSONOS 6                    // Max number of Sonos IP devices in the list incl. reserved index #0

WiFiServer G_Myserver(SERVER_PORT);
IPAddress G_Myip;
long G_Myrssi;

EasyWiFi MyEasyWiFi;
char MyAPName[]= {"SonosHub_AP"};

// SonosDevice Structure
#define SONOS_IP0 192,168,200,4       // Your Sonos Speaker1 - default- not 'perse' required
#define IPLISTFILE "/fs/ipfile"
IPAddress ACTIVE_sonosIP, G_SonosIPList[MAXSONOS] ={IPAddress(SONOS_IP0)} ;  // pre-defined IP adresses #0 of Sonos devices
WiFiClient G_Sonosclient;                               // setup wifiClient for Sonos Control
SonosUPnP G_Sonos = SonosUPnP(G_Sonosclient);           // Setup SonosUpnp Device G_Sonos
struct SonosDevice { // /status/zp details Structure
  char Zonename[32]; // ZoneName
  char UID[32];      // LocalUID RINCON_xxx
  char Serialnumber[32];  // serial number full
  char Seriesid[16]  ;     //SeriesID
  } ACTIVE_sonosDV,G_SonosList[MAXSONOS];     // Device array of max 4 Devices in your network
int ACTIVE_sonosNR=0;

//
// Setup / initialisation 
//
void setup()
{ 
/*********** Serial SETUP  **********/
int t=10;  //Initialize serial and wait for port to open, max 10 second waiting
Serial.begin(115200);
while (!Serial) {
    delay(1000);
    if ( (t--)== 0 ) break; // no serial, but continue program
    }
  
/*********** WIFI SETUP  **********/
if (WiFi.status() == WL_NO_SHIELD) {   // check for the presence of the shield:
#if DEBUG_X
    Serial.println("* WiFi shield not present - halted");
#endif
    while (true);     // don't continue if no shield
  }
delay(1000);
#if DEBUG_X
   Serial.println("* WiFi starting...");
#endif  
MyEasyWiFi.apname(MyAPName);
MyEasyWiFi.seed(4);
MyEasyWiFi.start();     // Start Wifi login 
if ( WiFi.status() != WL_CONNECTED){
#if DEBUG_X
   Serial.println("* WiFi cant connect. halted");
#endif
    while (true);     // don't continue if no wifi
    }
G_Myip = WiFi.localIP();

/*********** Server SETUP  **********/
#if DEBUG_X
   Serial.println("* Webserver starting...");
#endif  
G_Myserver.begin();

/*********** Sonos IP SETUP  **********/
if (Read_BaseIP(&ACTIVE_sonosIP)!=0){
  G_SonosIPList[0]=ACTIVE_sonosIP; // if Stored Ip is available, load it in index#0, overwriting the default from code
  #if DEBUG_X
   Serial.println("* Stored Sonos IP found, set Active");
}
#endif 
else
{
  #if DEBUG_X
   Serial.println("* Default#0 Sonos-IP set to intitial value");
#endif  
}
G_Sonos.CheckUPnP(&G_SonosIPList[1],MAXSONOS-1);                                             // check Sonos IP's on network as from index#1
for(t=0;t<MAXSONOS;++t) SonosCheck(G_SonosIPList[t],&G_SonosList[t]);                       // Read Sonos speaker info via IP into AudioDevice structure List 
if ( G_SonosList[0].Zonename[0]==0 && G_SonosList[1].Zonename[0]!=0 ) ACTIVE_sonosNR=1;    // if initial device in #0 is invalid, choose new found device #1 as active device
else ACTIVE_sonosNR=0;
ACTIVE_sonosIP=G_SonosIPList[ACTIVE_sonosNR];                                           // Assing Active Sonos IP 
ACTIVE_sonosDV=G_SonosList[ACTIVE_sonosNR];                                            // Assign Active Audiodevice G_SonosList 
} // endSetup






//
// Main SUPER Loop
//
void loop()
{
int t;
char c,text[128];
FullTrackInfo info;
SonosInfo infoX;
  infoX= G_Sonos.getSonosInfo(ACTIVE_sonosIP); // Request Sonos Device info
  Serial.print("\nActive Sonos IP ");Serial.print(ACTIVE_sonosIP);Serial.print(", UID ");Serial.println(infoX.uid);
  Serial.print("Sonos Type ");Serial.print(infoX.seriesid);
  Serial.print(", Serial[");Serial.print(infoX.serial);
  Serial.print("] ");Serial.print(infoX.status);
  Serial.print(" in Zone ");Serial.print(infoX.zone);
  Serial.print(" via ");Serial.println(infoX.medium);

  Serial.print("Playing from source ");Serial.print(infoX.source);
  Serial.print(", using play mode ");Serial.println(infoX.playmode);Serial.print("\n");

while(1) {
   info= G_Sonos.getFullTrackInfo(ACTIVE_sonosIP);
   Serial.print(info.album);
   Serial.print(" - ");Serial.println(info.creator);
   Serial.print(info.number);
   Serial.print(". ");Serial.print(info.title);
   Serial.print("  ");Serial.println(info.position);
   //Serial.print("/");Serial.println(info.duration);
   for(t=0;t<10000;++t) {
    CheckServerClients();  
    }
   }

} // end Main loop






/**** WIFI ROUTINES  *****/

// SERIALPRINT Wifi Status - only for debug
void printWiFiStatus() {
#if DEBUG_X
    // print the SSID of the network you're attached to:
    Serial.print("* SSID: "); Serial.print(WiFi.SSID());
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP(); Serial.print(" - IP Address: "); Serial.print(ip);
    // print your WiFi gateway:
    IPAddress ip2 = WiFi.gatewayIP(); Serial.print(" - IP Gateway: ");Serial.print(ip2);    
    // print the received signal strength:
    long rssi = WiFi.RSSI(); Serial.print("- Rssi: "); Serial.print(rssi); Serial.println(" dBm");
#endif
}


/**** Sonosroutine  *****/
void  SonosCheck(IPAddress SAdress, SonosDevice* Device){
SonosInfo info;
int t;
// initialise date to zero
Device->Zonename[0] = 0; Device->UID[0]=0 ; Device->Serialnumber[0]=0 ;Device->Seriesid[0]=0;

if (SAdress!=0){ // check if IP is not zero
#if DEBUG_X
    Serial.print("* Check Sonos Device ");Serial.println(SAdress);
#endif
     info=G_Sonos.getSonosInfo(SAdress);
     if(info.zone[0]!=0){
      for(t=0;info.zone[t]!=0;++t) {Device->Zonename[t]=info.zone[t];} Device->Zonename[t]=0;
      for(t=0;info.uid[t]!=0;++t) {Device->UID[t]=info.uid[t];} Device->UID[t]=0;
      for(t=0;info.serial[t]!=0;++t) {Device->Serialnumber[t]=info.serial[t];} Device->Serialnumber[t]=0;
      for(t=0;info.seriesid[t]!=0;++t) {Device->Seriesid[t]=info.seriesid[t];} Device->Seriesid[t]=0;      
#if DEBUG_X
      Serial.print(" - ");Serial.print(Device->Seriesid);
      Serial.print("@");Serial.print(Device->Zonename);
      Serial.print("/");Serial.println(Device->UID);
#endif
  }
 else
 {
 #if DEBUG_X
    Serial.println("*Sonos not connected");           // print a message out the serial port
#endif
 }
}
else { // if IP was zero, dont scan, but fill with zero
#if DEBUG_X
  Serial.print("* Check Sonos Device ");Serial.print(SAdress);Serial.println(" is invalid IP.");
#endif  
}
}

/* Read IP adress in 4 byte values - no Cyphering */
byte Read_BaseIP(IPAddress * adress)
{
IPAddress ip;
  int c=0;
  byte buf[4];
  WiFiStorageFile file = WiFiStorage.open(IPLISTFILE);
  if (file) {
    file.seek(0);
      if (file.available()) {  // read file buffer into memory, max size is 64 bytes for 2 char-strings
      c=c+file.read(buf, 4);  //Serial.write(buf, c);
   ip[0]= buf[0]; ip[1]= buf[1]; ip[2]= buf[2]; ip[3]= buf[3];
 *adress=ip;
      }
#if DEBUG_X
   Serial.print("* Read IPList #");Serial.print(c);Serial.print(", IP:");Serial.println(ip);
#endif    
   file.close(); return(c);
 }
 else {
#if DEBUG_X
   Serial.println("* Cant read IPlist :");
#endif    
  file.close();return(0);
 }
}

/* Write  ip adress in 4 byte values. no Cyhpering*/
byte Write_BaseIP(IPAddress adress)
{
  int c=0;
  byte i;
  WiFiStorageFile file = WiFiStorage.open(IPLISTFILE);
  if (file) {
    file.erase();     // erase content bnefore writing
  }
  i=adress[0];
  c=c+file.write(&i, 1);
    i=adress[1];
  c=c+file.write(&i, 1);
    i=adress[2];
  c=c+file.write(&i, 1);
    i=adress[3];
  c=c+file.write(&i, 1);
   if(c!=0) {
#if DEBUG_X
 Serial.print("* Written IPList #");Serial.println(c);
#endif
   file.close(); return(c);
 }
 else {
#if DEBUG_X
   Serial.println("* Cant write IPList");
#endif  
  file.close(); return(0);
 }
}

/* Erase IPlist in flkash file */
byte Erase_BaseIP()
{
char empty[16]="0empty0o0empty0";  
  WiFiStorageFile file = WiFiStorage.open(IPLISTFILE);
  if (file) {
  file.seek(0);
  file.write(empty,16); //overwrite flash
  file.erase();
#if DEBUG_X
 Serial.println("* Erased IPlistfile : ");
#endif  
  file.close(); return(1);
 }
 else {
  #if DEBUG_X
 Serial.println("* Could not erase IPlist : ");
#endif  
  file.close(); return(0);
 }
}

/********* HUB WEBSERVER ROUTINES *************/

// Check server if client is there, and service requests //
void CheckServerClients() {

  // Local Varaibles
  String currentLine = "";  // date line input client
  int t=0, d = 0,v=0; // client data counter and loop counter, t is local loop counter
  byte hr,mn,sc; // hour and minute read characters
  int h1,h2,h3,h4; // input read words
  char st[7] = {0,0,0,0,0,0,0};
  char *pcolor = &st[0];
  struct SonosDevice Selectdevice;
  
  G_Myip = WiFi.localIP();
  G_Myrssi = WiFi.RSSI(); // RSSI data
  WiFiClient client = G_Myserver.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
//#if DEBUG_X
//    Serial.println("*New client*");           // print a message out the serial port
//#endif
    currentLine = "";                       // make a String to hold incoming data from the client
    d = 0;
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
//#if DEBUG_X
//        Serial.write(c);                     // print it out the serial monitor - debug only
//#endif
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
          SendHTMLHeader1(client);     
            client.print( "<p style=\"font-family:Helvetica; color:GhostWhite\">&nbsp<font size=1>o</font><font size=2>o</font><font size=3>O</font><font size=2>Q</font><font size=1>*</font><font size=2>Q</font><font size=3>O</font><font size=2>o</font><font size=1>o</font> <br>");
            client.print( "<font size=5>Sonos Control </font>&nbsp&nbsp&nbsp&nbsp<font size=3>Version ");client.print(VERSION);client.println("</font><br>");
            client.print( "<font size=2>");client.print(WiFi.SSID());client.println(" / ");client.print(G_Myip);client.println("</font></p>");
            client.print("<p style=\"font-family:Helvetica; color:#334455\">-------------------------------------------<font size=2><br>");
            if(ACTIVE_sonosDV.Zonename[0]!=0) 
              {
               client.print("Active Device # "); client.print(ACTIVE_sonosNR); client.print("<br>");
               client.print("Device: "); client.print(ACTIVE_sonosDV.Seriesid);client.print(" @ Zone "); client.print(ACTIVE_sonosDV.Zonename); client.print("<br>");
               client.print("UID: "); client.print(ACTIVE_sonosDV.UID);  client.print("<br>");
               client.print("IP: "); client.print(ACTIVE_sonosIP);client.print("<br>");
              }
            else
              {
              client.print("Active device #"); client.print(ACTIVE_sonosNR);client.print(" not a valid Sonos, check List<br>");
              }
           client.print("-------------------------------------------<br>");
           client.print("Click <a href=\"/I\">status</a> to see status info.<br>");
           client.print("Click <a href=\"/H\">help</a> for Help.<br><br>");
           // The HTTP response ends with another blank line:
           client.println();
            // break out of the while loop:
#if DEBUG_X
            Serial.println("*Web: Html Home-page send");
#endif
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (currentLine.endsWith("GET /I")) {
          SendHTMLHeader1(client);     
            client.print( "<p style=\"font-family:Helvetica; color:GhostWhite\">&nbsp<font size=1>o</font><font size=2>o</font><font size=3>O</font><font size=2>O</font><font size=1>o</font><font size=2>O</font><font size=3>O</font><font size=2>o</font><font size=1>o</font> <br>");
            client.print( "<font size=5>Sonos Control </font>&nbsp&nbsp&nbsp&nbsp<font size=4>Status Menu</font>");
            client.print("<p style=\"font-family:Helvetica; color:#334455\">-------------------------------------------<font size=2><br>");
            client.print("IP Adress : "); client.print(WiFi.localIP()); client.print("<br>");
            client.print("Ssid : "); client.print(WiFi.SSID()); 
            client.print(", Rssi : "); client.print(WiFi.RSSI()); client.print("dB<br>");
            client.print("-------------------------------------------<br>");
            client.print("Sonos Device List (*=active control)<br>");
            for(t=0;t<MAXSONOS;t++) {
              Selectdevice = G_SonosList[t];
              client.print("Device");client.print(t);
              if(t==ACTIVE_sonosNR) client.print("*  "); else client.print(".  ");   // Mark ActiveSonos device
              client.print(G_SonosIPList[t]); 
              client.print(" / ");client.print(Selectdevice.Seriesid);
              client.print(" @ ");client.print(Selectdevice.Zonename); 
              client.print("<br>");
              }
              if(ACTIVE_sonosDV.Zonename[0]!=0) {
                client.print("-------------------------------------------<br>"); 
                client.print("Active Device");client.print(ACTIVE_sonosNR);client.print(" Settings <br>");
                client.print("Volume: "); client.print( G_Sonos.getVolume(ACTIVE_sonosIP));   
                client.print("  Treble: "); client.print( G_Sonos.getTreble(ACTIVE_sonosIP));          
                client.print("  Bass: "); client.print( G_Sonos.getBass(ACTIVE_sonosIP)); client.print("<br>"); 
               SonosInfo infoX= G_Sonos.getSonosInfo(ACTIVE_sonosIP); // Request Sonos Device info 
                client.print(""); client.print( infoX.status);   
                client.print(" from ");  client.print( infoX.source);        
                client.print(" in playmode "); client.print(infoX.playmode); client.print("<br>");   
                }
            client.print("-------------------------------------------<br>");
            client.print("Click <a href=\"/\">here</a> to return to menu.<br></p>");
            client.println();
#if DEBUG_X
          Serial.println("* Web: Info Send");
#endif
          break;
        }

        if (currentLine.endsWith("GET /H")) {
          SendHTMLHeader1(client);     
          client.print( "<p style=\"font-family:Helvetica; color:GhostWhite\">&nbsp<font size=1>o</font><font size=2>o</font><font size=3>O</font><font size=2>O</font><font size=1>o</font><font size=2>O</font><font size=3>O</font><font size=2>o</font><font size=1>o</font> <br>");
          client.print( "<font size=5>Sonos Control </font>&nbsp&nbsp&nbsp&nbsp<font size=4>Help Menu</font>");
          client.print("<p style=\"font-family:Helvetica; color:#334455\">-------------------------------------------<font size=2><br>");

          client.print("Use HTML-command   http:\\\\"); client.print(G_Myip); client.print("/[cmd]<br><br>");
          client.print("[cmd] = PLAY  Start Play<br>");
          client.print("[cmd] = STOP  Stop playing<br>");
          client.print("[cmd] = PAUSE  Pause play<br>");
          client.print("[cmd] = SKIPF  Skip Forward<br>");
          client.print("[cmd] = SKIPB  Skip Backward<br>");
          client.print("[cmd] = VOL[XX]  Volume to xx<br>");
          client.print("[cmd] = BAS[XX]  Bass to xx<br>");          
          client.print("[cmd] = TRE[XX]  Treble to xx<br><br>");    
          client.print("[cmd] = CHECK  Check Active Device<br>");
          client.print("[cmd] = SCAN  Scan Network <br>");
          client.print("[cmd] = SETIP[xx.xx.xx.xx]  Set Active Ip-address<br>");
          client.print("[cmd] = SAVEIP  Save active deviceIP to Flash<br>");
          client.print("[cmd] = SETA[X]  Set active device#<br>");
          client.print("[cmd] = SETC[X]  Set color<br><br>");
          client.print("[cmd] = I  Info-Status<br>");
          client.print("[cmd] = H  Help<br>");
          client.print("<br>");
          client.print("Click <a href=\"/\">here</a> to return to menu.<br><br>");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Help info Send");
#endif
          break;
        }
        
        if (currentLine.endsWith("GET /VOL")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);   
          h1=abs(h1)%52;               
          client.print("Sonos set Volume : "); client.print(h1); client.print(".<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.setVolume(ACTIVE_sonosIP, h1);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Volume Set");
#endif
        break;
        }     

        if (currentLine.endsWith("GET /BAS")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);   
          if (h1<-10) h1=-10; if (h1>10) h1=10;         
          client.print("Sonos set Bass : "); client.print(h1); client.print(".<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.setBass(ACTIVE_sonosIP, h1);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Bass Set");
#endif
        break;
        }  

        if (currentLine.endsWith("GET /TRE")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);   
          if (h1<-10) h1=-10; if (h1>10) h1=10;         
          client.print("Sonos set Treble : "); client.print(h1); client.print(".<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.setTreble(ACTIVE_sonosIP, h1);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Treble Set");
#endif
        break;
        }  


        if (currentLine.endsWith("GET /PLAY")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Sonos Play.<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.play(ACTIVE_sonosIP);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Play request");
#endif
          break;
        }          

        if (currentLine.endsWith("GET /STOP")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Sonos Stop.<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.stop(ACTIVE_sonosIP);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Stop request");
#endif
          break;
        }

        if (currentLine.endsWith("GET /PAUSE")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Sonos Pause.<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.pause(ACTIVE_sonosIP);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Pause request");
#endif
          break;
        }

        if (currentLine.endsWith("GET /SKIPB")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Sonos Skip Backward.<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.skip(ACTIVE_sonosIP,SONOS_DIRECTION_BACKWARD);   
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Skip Backward request");
#endif
          break;
        }

        if (currentLine.endsWith("GET /SKIPF")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Sonos Skip Forward.<br>");
          if(ACTIVE_sonosDV.Zonename[0]!=0) G_Sonos.skip(ACTIVE_sonosIP,SONOS_DIRECTION_FORWARD);   
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Skip Forward request");
#endif
          break;
        }
                
            
        if (currentLine.endsWith("GET /SCAN")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Scan-Network - New parts are loaded as from index #1<br>Wait a few seconds.<br>");
          G_Sonos.CheckUPnP(&G_SonosIPList[1],MAXSONOS-1);  // scan for upnp IP as from index #1
          for(t=0;t<MAXSONOS;++t) SonosCheck(G_SonosIPList[t],&G_SonosList[t]); // Read Sonos speaker info via IP into AudioDevice structure List 
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Scan Network request");
#endif
          break;
        }     

        if (currentLine.endsWith("GET /CHECK")) {
          //ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);                  
          client.print("Check Active SonosDevice, Wait a few seconds.<br>");
          SonosCheck(G_SonosIPList[ACTIVE_sonosNR],&G_SonosList[ACTIVE_sonosNR]);
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Check Device request");
#endif
          break;
        }     
        
        if (currentLine.endsWith("GET /SETIP")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);   
          client.print("Set IP, Scan Sonos - Wait a few seconds "); client.print(".<br>");
          G_SonosIPList[ACTIVE_sonosNR] = IPAddress(h1,h2,h3,h4);
          SonosCheck(G_SonosIPList[ACTIVE_sonosNR],&G_SonosList[ACTIVE_sonosNR]);
          ACTIVE_sonosIP=G_SonosIPList[ACTIVE_sonosNR];    // Assing Active Sonos IP to first IP in the list
          ACTIVE_sonosDV=G_SonosList[ACTIVE_sonosNR];      // Assign Active Audiodevice G_SonosList #
          if(ACTIVE_sonosDV.Zonename[0]==0) 
            client.print("Invalid IP, no device found.<br>");
          else
          client.print("IP set, found device ");client.print(ACTIVE_sonosDV.Seriesid);client.print(".<br>");
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: IP Set");
#endif
          break;
        }     

        
        if (currentLine.endsWith("GET /SAVEIP")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);   
          client.print("Save Active SonosIP to Flash :"); client.print(G_SonosIPList[ACTIVE_sonosNR]); client.print(".<br>");
          if ( Write_BaseIP(G_SonosIPList[ACTIVE_sonosNR] ) !=0 ) client.print("WrittenSonosIP to Flash <br>");
          else client.print("Could not write to Flash <br>");
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: IP Set");
#endif
          break;
        }     
 
        if (currentLine.endsWith("GET /SETA")) {
          ReadPointSeparatedValues(client,&h1,&h2,&h3,&h4);
          SendHTMLHeader1(client);  
          client.print("Set Active part, checking Sonos - Wait a few seconds.<br>"); 
          if(h1>=0 && h1<MAXSONOS ) 
          {
             ACTIVE_sonosNR=h1;
             ACTIVE_sonosIP=G_SonosIPList[ACTIVE_sonosNR];    // Assing Active Sonos IP to first IP in the list
             ACTIVE_sonosDV=G_SonosList[ACTIVE_sonosNR];      // Assign Active Audiodevice G_SonosList #0             
            if( ACTIVE_sonosDV.Zonename[0]==0) { 
                client.print("Active Device set to Invalid entry: check IP.<br>"); }
                }
          client.print("<br>Click <a href=\"/\">here</a> to return to menu.<br>");
          client.print("<meta http-equiv=\"refresh\" content=\"3;url=/\" />");
          client.println();
#if DEBUG_X
          Serial.println("* Web: Active Sonos Set");
#endif
          break;
        } 
        
      }
      else {
        d++;
        if (d > MAX_MISSED_DATA) { // defined in arduino_secrets.h
#if DEBUG_X
          Serial.println("*Client missed-data time-out");
#endif
          break;   // time-out to prevent to ever waiting for misssed non-send data newclient //
        }
      }
    }
    // close the connection:
    client.stop();
    
//#if DEBUG_X
//    Serial.println("*Client disonnected*");
//#endif
  } // end ifloop (client)
  else
  {
    client.stop();
    /*
    client.flush();h1=0;
    while( client.connected() ) {
      client.stop(); h1++;
      if (h1>10) break;    
    }
    */
  } 
}


// read 4 'point'-separated integer values from open client, format GET [command]wwww.xxxx.yyyy.zzzz -> read as from first x-value after GET comand.
// maps wwww-> h1 xxxx -> h2, yyyy -> h3 zzzz -> h4, separator can be :  [.] [/] [:] [,], usable for time, dates, ipadress etc etc.
void ReadPointSeparatedValues(WiFiClient client,int *h1,int *h2, int *h3,int *h4)
{
byte hr,t=1;
uint16_t h;
int signh=1;
*h1=0;*h2=0;*h3=0,*h4=0;
hr=(byte)client.read();                                               // readfirst character
while( (hr!=32) && (hr!=13) )                                         // Read till space (end of command) or CR character (end of line)
      { 
      if( hr =='.' || hr ==':' || hr==',' || hr=='/') { *h4=*h3;*h3=*h2;*h2=(*h1)*signh;*h1=0;signh=1;++t; }                     // at separator: new int valueto read: shift values to next pushed variable
      if( hr =='-') { signh=-1; }  
      if (hr>=48 && hr<=57) { *h1= (*h1)*10 + (hr-48); }              // process only ascii numbers 0-9
      hr=(byte)client.read(); Serial.print(hr);
      }    
 if  (t ==1)  *h1=(*h1)*signh;                                 // 1 ints read: value in h1 = ok, adapt sign
 else if  (t ==2)  {h=*h1;*h1=*h2;*h2=h;}                        // 2 ints read : swap value h1 and h2, h3 is kept 0, h4 is kept 0
 else if  (t ==3)  {h=*h1;*h1=*h3;*h3=h;}                        // 3 ints read : swap order 3->1, (2=2), 1->3
 else if  (t ==4)  {h=*h1;*h1=*h4;*h4=h;h=*h2;*h2=*h3;*h3=h;}    // 4 ints read : swap order : 4->1, 1->4, 2->3 3->2
 else {*h1=0;*h2=0;*h3=0;*h4=0; }                                // 5 or more ints read : failure, return 0's
}


// send HTML Header with Mata Code
void SendHTMLHeader1(WiFiClient client){
     client.println(F("HTTP/1.1 200 OK")); 
     client.println(F("Content-type:text/html"));
     client.println();    
     client.println(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">")); // metaview          
     client.println(F("<p style=\"font-family:Helvetica;\">"));
     client.print(F("<body style=\"background-color:#"));
     client.print("A");client.print(random(0,15),HEX);
     client.print("A");client.print(random(0,15),HEX);
     client.print("A");client.print(random(0,15),HEX);
     client.println("\">"); 
}

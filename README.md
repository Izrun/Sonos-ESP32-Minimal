# SonosUPnP


Fork From original Sonos library : https://github.com/tmittet/sonos

** MKR1010 / WifiNINA uBlox NINA W10 : **

WIFININA port, have a look this fork: https://github.com/javos65/sonos
Added Sonos upnp scan to find your sonos devices on your network
Added a patched function parsing for TrackMetaData parsing (very loose!)

New SonosUPnP-class functions:

getZone(IPAddress,.buffer)
getSerial(IPAddress,buffer)
getSource(IPAddress,buffer)
getMedium(IPAddress,buffer) - including Spotify


getFullTrackInfo(IPAddress) : using new Structure : FullTrackInfo to use TrackMetaData
getSonosInfo(IPAddress) : using new Structure : SonosInfo


Track MetaData :
getTrackCreator(IPAddress, buffer)
getTrackTitle(IPAddress, buffer)
getTrackAlbum(IPAddress, buffer)

CheckUPnP(IPAddress *Plist) -> SCAN FUNCTION for finding your uPnP Sonos on your network, results in a list of Ip-adresses

Original:
This library makes interfacing an Arduino with your Sonos system a breeze. The
library supports both controlling and reading the state of your Sonos components.
Playing URIs, files, online radio stations, playlists, and line-in is supported
as well as the most common commands like play, pause, skip, mute, volume and
speaker grouping. What sets this library apart from similar libraries written
for the Arduino platform it its ability to read the state of most of the Sonos
functions: getting source, player state, track number, track position, volume
and more. The library is relatively compact and has a small enough memory
footprint to run on the Arduino Uno and Duemilanove.


**ESP8266:**  
For an ESP8266 port, have a look this fork:
[github.com/antonmeyer/sonos](https://github.com/antonmeyer/sonos)

**Getting Started:**
- Download and add this library to your Ardiono libraries folder.
- Download and add the MicroXPath library
([github.com/tmittet/microxpath](https://github.com/tmittet/microxpath)).

**Note:**  
If you download the libraries as ZIP files from GitHub, "-master" will be added
to the end of the file names. You need to rename the files, such that e.g.
"microxpath-master.zip" becomes "microxpath.zip", before adding the libraries
to your Ardiono libraries folder.

**Arduino Example Sketch:**  
Example of IP scanning the network for UpnP:1400 sonos devices

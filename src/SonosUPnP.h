/************************************************************************/
/* Sonos UPnP, an UPnP based read/write remote control library, v2.1.   */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet (code@lookout.no) January 2015.             */
/*                                                                      */
/* Modified for use with Wifi.h or WifiNINA.h for wireless Networking   */
/* Added uPnP Network Scan function  Jay Vox 2020  
// 
// Modified to work with the Heltec WiFi Kit32 with integrated wifi,
//	OLED display,  Geoff Shorten 2020
//                   												 */
/************************************************************************/

#ifndef SonosUPnP_h
   #define SonosUPnP_h

//#define SONOS_WRITE_ONLY_MODE

#include <Arduino.h>
//#if (defined(__AVR__))
//  #include <avr/pgmspace.h>
//#else
//  #include <pgmspace.h>
//#endif
// below added by GS from joeybab3 fork

#if defined(__AVR__)
	#include <avr/pgmspace.h>
	#include <WiFi.h>
#elif defined(ESP8266)
	#include <ESP8266WiFi.h>
#else
	//#include <avr/pgmspace.h>
	#include <pgmspace.h>
	#include <WiFi.h>
#endif


#ifndef SONOS_WRITE_ONLY_MODE
#include <MicroXPath_P.h>
#endif
//#include <WiFiNINA.h>  // JV : Wireless Mkr1010  //GS use WiFi.h above
#include <WiFiUdp.h>   // JV : UDP service

// HTTP:
#define HTTP_VERSION " HTTP/1.1\n"
#define HEADER_HOST "Host: %d.%d.%d.%d:%d\n"
#define HEADER_CONTENT_TYPE "Content-Type: text/xml; charset=\"utf-8\"\n"
#define HEADER_CONTENT_LENGTH "Content-Length: %d\n"
#define HEADER_SOAP_ACTION "SOAPAction: \"urn:"
#define HEADER_SOAP_ACTION_END "\"\n"
#define HEADER_CONNECTION "Connection: close\n"

// SOAP tag data:
#define SOAP_ENVELOPE_START "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
#define SOAP_ENVELOPE_END "</s:Envelope>"
#define SOAP_BODY_START "<s:Body>"
#define SOAP_BODY_END "</s:Body>"
#define SOAP_TAG_START "<%s>"
#define SOAP_TAG_END "</%s>"
#define SOAP_TAG_LEN 5
#define SOAP_TAG_ENVELOPE "s:Envelope"
#define SOAP_TAG_BODY "s:Body"

// UPnP config:
#define UPNP_PORT 1400
#define UPNP_MULTICAST_IP (byte[]) {239,255,255,250}
#define UPNP_MULTICAST_PORT 1900
#define UPNP_MULTICAST_TIMEOUT_S 2
#define UPNP_RESPONSE_TIMEOUT_MS 3000  //GS was 3000 increased to 5000
#define UPNP_DEVICE_SCAN "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\nST: urn:schemas-upnp-org:device:ZonePlayer:1\r\n\0"
//JB from @Frank-Bemelman #define UPNP_DEVICE_SCAN "M-SEARCH * HTTP/1.1\nHOST: 239.255.255.250:1900\nMAN: \"ssdp:discover\"\nMX: 2\nST: urn:schemas-upnp-org:device:ZonePlayer:1\n\0"

// UPnP tag data:
#define SOAP_ACTION_START_TAG_START "<u:"
#define SOAP_ACTION_START_TAG_NS " xmlns:u=\"urn:"
#define SOAP_ACTION_START_TAG_END "\">"
#define SOAP_ACTION_END_TAG_START "</u:"
#define SOAP_ACTION_END_TAG_END ">"
#define SOAP_ACTION_TAG_LEN 24

// UPnP service data:
#define UPNP_URN_SCHEMA "schemas-upnp-org:service:"
#define UPNP_AV_TRANSPORT 1
#define UPNP_AV_TRANSPORT_SERVICE "AVTransport:1"
#define UPNP_AV_TRANSPORT_ENDPOINT "/MediaRenderer/AVTransport/Control"
#define UPNP_RENDERING_CONTROL 2
#define UPNP_RENDERING_CONTROL_SERVICE "RenderingControl:1"
#define UPNP_RENDERING_CONTROL_ENDPOINT "/MediaRenderer/RenderingControl/Control"
#define UPNP_DEVICE_PROPERTIES 3
#define UPNP_DEVICE_PROPERTIES_SERVICE "DeviceProperties:1"
#define UPNP_DEVICE_PROPERTIES_ENDPOINT "/DeviceProperties/Control"


#define SONOS_TAG_GET_ZONE_ATTR "GetZoneAttributes"
#define SONOS_TAG_GET_ZONE_ATTR_RESPONSE "u:GetZoneAttributesResponse"
#define SONOS_TAG_ZONENAME "CurrentZoneName"
#define SONOS_TAG_GET_ZONE_INFO "GetZoneInfo"
#define SONOS_TAG_GET_ZONE_INFO_RESPONSE "u:GetZoneInfoResponse"
#define SONOS_TAG_SERIAL "ExtraInfo"

#define SONOS_GET_ZPSUPPORTINFO "ZPSupportInfo"
#define SONOS_GET_ZPINFO "ZPInfo"
#define SONOS_GET_ZPZONE "ZoneName"
#define SONOS_GET_ZPLOCALUID "LocalUID"
#define SONOS_GET_ZPSERIAL "SerialNumber"
#define SONOS_GET_ZPSERIESID "SeriesID"

#define SONOS_GET_ROOT "root"
#define SONOS_GET_DEVICE "device"
#define SONOS_GET_MODEL "modelName"
#define SONOS_GET_DISPLAYNAME "displayName"
#define SONOS_GET_ICONLIST "iconList"
#define SONOS_GET_ICON "icon"
#define SONOS_GET_ICONURL "url"

// Sonos speaker state control:
/*
<u:Play>
  <InstanceID>0</InstanceID>
  <Speed>1</Speed>
</u:Play>
<u:Seek>
  <InstanceID>0</InstanceID>
  <Unit>REL_TIME</Unit>
  <Target>0:01:02</Target>
</u:Seek>
*/
#define SONOS_TAG_PLAY "Play"
#define SONOS_SOURCE_RINCON_TEMPLATE "RINCON_%s0%d%s"
#define SONOS_TAG_SPEED "Speed"
#define SONOS_TAG_STOP "Stop"
#define SONOS_TAG_PAUSE "Pause"
#define SONOS_TAG_PREVIOUS "Previous"
#define SONOS_TAG_NEXT "Next"
#define SONOS_DIRECTION_BACKWARD 0
#define SONOS_DIRECTION_FORWARD 1
#define SONOS_INSTANCE_ID_0_TAG "<InstanceID>0</InstanceID>"

#define SONOS_TAG_SEEK "Seek"
#define SONOS_TAG_TARGET "Target"
#define SONOS_SEEK_MODE_TAG_START "<Unit>"
#define SONOS_SEEK_MODE_TAG_END "</Unit>"
#define SONOS_SEEK_MODE_TRACK_NR "TRACK_NR"
#define SONOS_SEEK_MODE_REL_TIME "REL_TIME"
#define SONOS_TIME_FORMAT_TEMPLATE "%d:%02d:%02d"

#define SONOS_TAG_SET_AV_TRANSPORT_URI "SetAVTransportURI"
#define SONOS_TAG_CURRENT_URI "CurrentURI"
#define SONOS_URI_META_LIGHT_START "<CurrentURIMetaData>"
#define SONOS_URI_META_LIGHT_END "</CurrentURIMetaData>"
#define SONOS_RADIO_META_FULL_START "<CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/46&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;"
#define SONOS_RADIO_META_FULL_END "&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData>"

#define SONOS_TAG_BECOME_COORDINATOR_OF_STANDALONE_GROUP "BecomeCoordinatorOfStandaloneGroup"

#define SONOS_TAG_SET_LED_STATE "SetLEDState"
#define SONOS_TAG_DESIRED_LED_STATE "DesiredLEDState"

// Playlist & Queue
/*
<u:AddURIToQueueResponse xmlns:u="urn:schemas-upnp-org:service:AVTransport:1">
  <FirstTrackNumberEnqueued>21</FirstTrackNumberEnqueued>
  <NumTracksAdded>36</NumTracksAdded>
  <NewQueueLength>56</NewQueueLength>
</u:AddURIToQueueResponse>
*/
#define SONOS_TAG_ADD_URI_TO_QUEUE "AddURIToQueue"
#define SONOS_TAG_ENQUEUED_URI "EnqueuedURI"
#define SONOS_SAVED_QUEUES "file:///jffs/settings/savedqueues.rsq#%d"
#define SONOS_TAG_REMOVE_ALL_TRACKS_FROM_QUEUE "RemoveAllTracksFromQueue"
#define SONOS_PLAYLIST_META_LIGHT_START "<EnqueuedURIMetaData></EnqueuedURIMetaData><DesiredFirstTrackNumberEnqueued>"
#define SONOS_PLAYLIST_META_LIGHT_END "0</DesiredFirstTrackNumberEnqueued><EnqueueAsNext>1</EnqueueAsNext>"
#define SONOS_PLAYLIST_META_FULL_START "<EnqueuedURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;SQ:0&quot; parentID=&quot;SQ:&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;"
#define SONOS_PLAYLIST_META_FULL_END "&lt;/dc:title&gt;&lt;upnp:class&gt;object.container.playlistContainer&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;RINCON_AssociatedZPUDN&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</EnqueuedURIMetaData><DesiredFirstTrackNumberEnqueued>0</DesiredFirstTrackNumberEnqueued><EnqueueAsNext>1</EnqueueAsNext>"

// Track & source:
/*
<u:GetPositionInfoResponse>
  <Track>1</Track>
  <TrackDuration>0:03:21</TrackDuration>
  <TrackMetaData>[Meta data in DIDL-Lite]</TrackMetaData>
  <TrackURI></TrackURI>
  <RelTime>0:01:23</RelTime>
  <AbsTime>NOT_IMPLEMENTED</AbsTime>
  <RelCount>2147483647</RelCount>
  <AbsCount>2147483647</AbsCount>
</u:GetPositionInfoResponse>
*/
#define SONOS_TAG_GET_POSITION_INFO "GetPositionInfo"
#define SONOS_TAG_GET_POSITION_INFO_RESPONSE "u:GetPositionInfoResponse"
#define SONOS_TAG_TRACK "Track"
#define SONOS_TAG_TRACK_DURATION "TrackDuration"
#define SONOS_TAG_TRACK_URI "TrackURI"
#define SONOS_TAG_REL_TIME "RelTime"
#define SONOS_TAG_TRACKMETA "TrackMetaData" 


#define SONOS_ATTRIB_TITLE "dc:title"
#define SONOS_ATTRIB_CREATOR "dc:creator"
#define SONOS_ATTRIB_ALBUM "upnp:album"
#define SONOS_ATTRIB_ARTIST "r:albumArtist"

// radio stream
// r:streamContent : name artist & nummer
// dc:title : radio title

#define SONOS_MAXSOURCE 11

#define SONOS_SOURCE_UNKNOWN_SCHEME "z-sonos-unknown:\0"
#define SONOS_SOURCE_SPOTIFY_SCHEME "x-sonos-spotify:\0"
#define SONOS_SOURCE_FILE_SCHEME "x-file-cifs:\0"
#define SONOS_SOURCE_HTTP_SCHEME "x-sonos-http:\0"
#define SONOS_SOURCE_RADIO_SCHEME "x-rincon-mp3radio:\0"
#define SONOS_SOURCE_RADIO_AAC_SCHEME "aac:\0"
#define SONOS_SOURCE_LINEIN_SCHEME "x-rincon-stream:\0"
#define SONOS_SOURCE_MASTER_SCHEME "x-rincon:\0"
#define SONOS_SOURCE_QUEUE_SCHEME "x-rincon-queue:\0"
#define SONOS_SOURCE_SPOTIFYSTATION_SCHEME "x-sonosprog-spotify:\0"
#define SONOS_SOURCE_LOCALHTTP_SCHEME "http:\0"

#define UNKNOWN_SCHEME "Unknown"
#define FILE_SCHEME "File"
#define SPOTIFY_SCHEME "Spotify"
#define HTTP_SCHEME "Http"
#define RADIO_SCHEME "Radio"
#define RADIO_AAC_SCHEME "Radio AAC"
#define LINEIN_SCHEME "Line-In"
#define MASTER_SCHEME "Master"
#define QUEUE_SCHEME "Queue"
#define SPOTIFYSTATION_SCHEME "Spotify Station"
#define SPOTIFY_SCHEME "Spotify"
#define LOCALHTTP_SCHEME "Local Http"

// Volume, bass & treble:
/*
<u:GetVolume>
  <InstanceID>0</InstanceID>
  <Channel>Master</Channel>
</u:GetVolume>
<u:GetVolumeResponse>
  <CurrentVolume>[0-100]</CurrentVolume>
</u:GetVolumeResponse>
*/
#define SONOS_TAG_CHANNEL "Channel"
#define SONOS_CHANNEL_MASTER "Master"
//#define SONOS_CHANNEL_LEFT "LF"
//#define SONOS_CHANNEL_RIGHT "RF"
#define SONOS_TAG_GET_MUTE "GetMute"
#define SONOS_TAG_GET_MUTE_RESPONSE "u:GetMuteResponse"
#define SONOS_TAG_CURRENT_MUTE "CurrentMute"
#define SONOS_TAG_GET_VOLUME "GetVolume"
#define SONOS_TAG_GET_VOLUME_RESPONSE "u:GetVolumeResponse"
#define SONOS_TAG_CURRENT_VOLUME "CurrentVolume"
#define SONOS_TAG_GET_OUTPUT_FIXED "GetOutputFixed"
#define SONOS_TAG_GET_FIXED_RESPONSE "u:GetOutputFixedResponse"
#define SONOS_TAG_CURRENT_FIXED "CurrentFixed"
#define SONOS_TAG_GET_BASS "GetBass"
#define SONOS_TAG_GET_BASS_RESPONSE "u:GetBassResponse"
#define SONOS_TAG_CURRENT_BASS "CurrentBass"
#define SONOS_TAG_GET_TREBLE "GetTreble"
#define SONOS_TAG_GET_TREBLE_RESPONSE "u:GetTrebleResponse"
#define SONOS_TAG_CURRENT_TREBLE "CurrentTreble"
#define SONOS_TAG_GET_LOUDNESS "GetLoudness"
#define SONOS_TAG_GET_LOUDNESS_RESPONSE "u:GetLoudnessResponse"
#define SONOS_TAG_CURRENT_LOUDNESS "CurrentLoudness"

// Set volume, bass & treble:
/*
<u:SetVolume>
  <InstanceID>0</InstanceID>
  <Channel>Master</Channel>
  <DesiredVolume>[0-100]</DesiredVolume>
</u:SetVolume>
*/
#define SONOS_TAG_SET_MUTE "SetMute"
#define SONOS_TAG_DESIRED_MUTE "DesiredMute"
#define SONOS_TAG_SET_VOLUME "SetVolume"
#define SONOS_TAG_DESIRED_VOLUME "DesiredVolume"
#define SONOS_TAG_SET_BASS "SetBass"
#define SONOS_TAG_DESIRED_BASS "DesiredBass"
#define SONOS_TAG_SET_TREBLE "SetTreble"
#define SONOS_TAG_DESIRED_TREBLE "DesiredTreble"
#define SONOS_TAG_SET_LOUDNESS "SetLoudness"
#define SONOS_TAG_DESIRED_LOUDNESS "DesiredLoudness"
#define SONOS_CHANNEL_TAG_START "<Channel>"
#define SONOS_CHANNEL_TAG_END "</Channel>"

// Play Mode:
/*
<u:GetTransportSettingsResponse>
  <PlayMode>[NORMAL/REPEAT_ALL/SHUFFLE/SHUFFLE_NOREPEAT]</PlayMode>
  <RecQualityMode>NOT_IMPLEMENTED</RecQualityMode>
</u:GetTransportSettingsResponse>
*/
#define SONOS_TAG_GET_TRANSPORT_SETTINGS "GetTransportSettings"
#define SONOS_TAG_GET_TRANSPORT_SETTINGS_RESPONSE "u:GetTransportSettingsResponse"
#define SONOS_TAG_PLAY_MODE "PlayMode"
#define SONOS_PLAY_MODE_NORMAL B00
#define SONOS_PLAY_MODE_NORMAL_VALUE "NORMAL"
#define SONOS_PLAY_MODE_REPEAT B01
#define SONOS_PLAY_MODE_REPEAT_VALUE "REPEAT_ALL"
#define SONOS_PLAY_MODE_SHUFFLE B10
#define SONOS_PLAY_MODE_SHUFFLE_VALUE "SHUFFLE_NOREPEAT"
#define SONOS_PLAY_MODE_SHUFFLE_REPEAT B11
#define SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE "SHUFFLE"
// Set Play Mode:
#define SONOS_TAG_SET_PLAY_MODE "SetPlayMode"
#define SONOS_TAG_NEW_PLAY_MODE "NewPlayMode"

// State:
/*
<u:GetTransportInfoResponse>
  <CurrentTransportState>[PLAYING/PAUSED_PLAYBACK/STOPPED]</CurrentTransportState>
  <CurrentTransportStatus>[OK/ERROR]</CurrentTransportStatus>
  <CurrentSpeed>1</CurrentSpeed>
</u:GetTransportInfoResponse>
*/

// new
#define SONOS_TAG_GET_MEDIA_INFO "GetMediaInfo"
#define SONOS_TAG_GET_MEDIA_INFO_RESPONSE "u:GetMediaInfoResponse"
#define SONOS_TAG_MEDIUM_STATUS "PlayMedium"
#define SONOS_MEDIUM_NONE 1
#define SONOS_MEDIUM_NONE_VALUE "NONE"
#define SONOS_MEDIUM_LINEIN 2
#define SONOS_MEDIUM_LINEIN_VALUE "LINE-IN"
#define SONOS_MEDIUM_NETWORK 3
#define SONOS_MEDIUM_NETWORK_VALUE "NETWORK"
#define SONOS_TAG_ARTIST_STATUS "CurrentURIMetaData"

#define SONOS_TAG_GET_TRANSPORT_INFO "GetTransportInfo"
#define SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE "u:GetTransportInfoResponse"
#define SONOS_TAG_CURRENT_TRANSPORT_STATE "CurrentTransportState"
#define SONOS_STATE_PLAYING 1
#define SONOS_STATE_PLAYING_VALUE "PLAYING"
#define SONOS_STATE_PAUSED 2
#define SONOS_STATE_PAUSED_VALUE "PAUSED_PLAYBACK"
#define SONOS_STATE_STOPPED 3
#define SONOS_STATE_STOPPED_VALUE "STOPPED"
#define SONOS_STATE_TRANSISTION 4
#define SONOS_STATE_TRANSISTION_VALUE "TRANSISTION"


///CHUNK DECODING
#define CHUNK_HTTP_RESPONSE 0
#define CHUNK_HTTP_HEADERS 1

#define CHUNK_UNINITIALIZED 2
#define CHUNK_HEADER 3
#define CHUNK_CONTENT 4
#define CHUNK_HEADER_CLOSING 5
#define CHUNK_FULL 6
#define CHUNK_FULL_CLOSING 7
#define CHUNK_FINAL 8
#define CHUNK_WAITING_HEADER 9


struct TrackInfo
{
  uint16_t number;
  uint32_t duration;
  uint32_t position;
  char *uri;
  };

struct FullTrackInfo // JV new, pass text-info as char string
{
  uint16_t number;
  char *duration;
  char *position;
  char *creator;
  char *title;
  char *album;
  
  };

struct SonosInfo // JV new, pass text info as Char string
{
  bool exists;
  char *uid;        // Rincon_xxxx 32 bytes
  char *serial;     // 24 bytes serialnumber
  char *seriesid;   // Series ID or Sonos Type - 24bytes
  char *zone;        // Zone name - 32 bytes  
};

struct SonosFullInfo : SonosInfo // JV new, pass text info as Char string
{
  char *model;          // Model Name (Sonos Playbar) - 24 bytes - root/device/modelName
  char *displayName;    // Model Name (Playbar) - 16 bytes - root/device/displayName
  char *icon;           // Relative path to icon - 32 bytes - root/device/iconList/icon/url
  char *medium;         // medium - network, linein etc - 16 bytes
  char *status;         // Status - play/stop/pause etc - 16 bytes
  char *playmode;       // playmode, see SONOS_PLAY_MODE definitions - 16 bytes
  char *source;         // source, defined in URI , see  SONOS_SOURCE definitions - 16 bytes
};



class SonosUPnP
{

  public:

    SonosUPnP(WiFiClient client,void (*ethernetErrCallback)(void));   // wireless //GS added error callback
    
	void setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address);
    void seekTrack(IPAddress speakerIP, uint16_t index);
    void seekTime(IPAddress speakerIP, uint8_t hour, uint8_t minute, uint8_t second);
    void setPlayMode(IPAddress speakerIP, uint8_t playMode);
    void play(IPAddress speakerIP);
    void playFile(IPAddress speakerIP, const char *path);
    void playHttp(IPAddress speakerIP, const char *address);
    void playRadio(IPAddress speakerIP, const char *address, const char *title);
    void playLineIn(IPAddress speakerIP, const char *speakerID);
    void playQueue(IPAddress speakerIP, const char *speakerID);
    void playConnectToMaster(IPAddress speakerIP, const char *masterSpeakerID);
    void disconnectFromMaster(IPAddress speakerIP);
    void stop(IPAddress speakerIP);
    void pause(IPAddress speakerIP);
    void skip(IPAddress speakerIP, uint8_t direction);
    void setMute(IPAddress speakerIP, bool state);
    void setVolume(IPAddress speakerIP, uint8_t volume);
    void setVolume(IPAddress speakerIP, uint8_t volume, const char *channel);
    void setBass(IPAddress speakerIP, int8_t bass);
    void setTreble(IPAddress speakerIP, int8_t treble);
    void setLoudness(IPAddress speakerIP, bool state);
    void setStatusLight(IPAddress speakerIP, bool state);
    void addPlaylistToQueue(IPAddress speakerIP, uint16_t playlistIndex);
    void addTrackToQueue(IPAddress speakerIP, const char *scheme, const char *address);
    void removeAllTracksFromQueue(IPAddress speakerIP);
    uint8_t CheckUPnP(IPAddress *List,int Listsize); // new JV uPnP IPlist check by SSDP over UDP
    
    #ifndef SONOS_WRITE_ONLY_MODE
    
    void setRepeat(IPAddress speakerIP, bool repeat);
    void setShuffle(IPAddress speakerIP, bool shuffle);
    void toggleRepeat(IPAddress speakerIP);
    void toggleShuffle(IPAddress speakerIP);
    void togglePause(IPAddress speakerIP);
    void toggleMute(IPAddress speakerIP);
    void toggleLoudness(IPAddress speakerIP);

    uint8_t getState(IPAddress speakerIP,char *buf); // new JV : string passthrough
    uint8_t getState(IPAddress speakerIP);

    uint8_t getMedium(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getMedium(IPAddress speakerIP); // new JV 

    uint8_t getPlayMode(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getPlayMode(IPAddress speakerIP); 

    bool getZone(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    bool getSerial(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getSource(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getSource(IPAddress speakerIP);

    bool getRepeat(IPAddress speakerIP);
    bool getShuffle(IPAddress speakerIP);
    TrackInfo getTrackInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize);
    FullTrackInfo getFullTrackInfo(IPAddress speakerIP); // new JV - parse track info without TrackURI
    SonosInfo getSonosInfo(IPAddress speakerIP); // new JV - parse Get status/zp XLM
    SonosFullInfo getSonosFullInfo(IPAddress speakerIP); // new JV - parse Get status/zp XLM
    uint16_t getTrackNumber(IPAddress speakerIP);
    void getTrackURI(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize);
    void getTrackCreator(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Creator
    void getTrackTitle(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Title
    void getTrackAlbum(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Album
    uint8_t getSourceFromURI(const char *uri);
    uint32_t getTrackDurationInSeconds(IPAddress speakerIP);
    uint32_t getTrackPositionInSeconds(IPAddress speakerIP);
    uint16_t getTrackPositionPerMille(IPAddress speakerIP);
    bool getMute(IPAddress speakerIP);
    uint8_t getVolume(IPAddress speakerIP);
    uint8_t getVolume(IPAddress speakerIP, const char *channel);
    bool getOutputFixed(IPAddress speakerIP);
    int8_t getBass(IPAddress speakerIP);
    int8_t getTreble(IPAddress speakerIP);
    bool getLoudness(IPAddress speakerIP);
 

    #endif

  private:

    WiFiClient ethClient;

    uint16_t _chunkSize;
    uint16_t _chunkCount;
    uint8_t _chunk_state;
    void chunk_reset();

    void (*ethernetErrCallback)(void);
    void seek(IPAddress speakerIP, const char *mode, const char *data);
    void setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address, PGM_P metaStart_P, PGM_P metaEnd_P, const char *metaValue);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);
    bool upnpPost(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);
    const char *getUpnpService(uint8_t upnpMessageType);
    const char *getUpnpEndpoint(uint8_t upnpMessageType);
    void ethClient_write(const char *data);
    void ethClient_write_P(PGM_P data_P, char *buffer, size_t bufferSize);
    void ethClient_stop();
    void readback_IP(IPAddress *IPa,char* buf,char pointer,char bufsize); // New JV : readback IP from (UDP)buffer
    bool upnpGetZP(IPAddress ip); // New JV GET command status/zp  
    bool upnpGetDevice(IPAddress ip); // New JV GET command status/zp  
    uint8_t toHex(char c);  //Convert a ascii hex string to number.  Used in chunk

    #ifndef SONOS_WRITE_ONLY_MODE
    MicroXPath_P xPath;
    void ethClient_xPath(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize);
    void ethClient_xPath2(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize); // modified version to search/parse in Metadata
    void ethClient_xPath_chunk(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize); // movidifed version supporintg chuinked encoding
    void upnpGetString(IPAddress speakerIP, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value, PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize);
    uint32_t getTimeInSeconds(const char *time);
    uint32_t uiPow(uint16_t base, uint16_t exp);
    uint8_t convertMedium(const char *input);
    uint8_t convertState(const char *input);
    uint8_t convertPlayMode(const char *input);
    uint8_t convertMetaData(char *input ); 


    #endif
};

#endif

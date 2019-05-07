// Select your modem:
//final codes today
#define TINY_GSM_MODEM_SIM800 
#include <TinyGPS++.h>
#include <TinyGsmClient.h> 
#include <ArduinoHttpClient.h> 
#include <SoftwareSerial.h>
 
#define SerialMon Serial 
#define DEVICE_ID "Keyless00001" 
#define modemBAUD 9600 
#define gpsBAUD 9600 
SoftwareSerial SerialAT (10, 11); // RX, the 
SoftwareSerial ss (4, 3); // RX, TX serial GPS

// Internet setting 
const char apn [] = "internet"; 
const char user [] = ""; 
const char pass [] = "";

// Firebase .
const char server  [] = "keyless-f343e.firebaseio.com"; 
const int port = 443; 
const String UPDATE_PATH = "gps_devices/" + String (DEVICE_ID); // firebase root table

// global variables 
int count = 0; 
String fireData = "";

#ifdef DUMP_AT_COMMANDS 
#include <StreamDebugger.h> 
StreamDebugger debugger (SerialAT, SerialMon); 
TinyGsm modem(debugger); 
#else 
TinyGsm modem(SerialAT); 
#endif 

TinyGsmClientSecure client (modem, 0); 
HttpClient https(client, server, port);

TinyGPSPlus gps;

void setup () { 
  //set console baud rate ;
SerialMon.begin (9600);
delay (10); 
initializeModem (); // check modem communication 
connection(); // test internet connection 
ss.begin (gpsBAUD); // open gps serial 
}

void loop () {

if (ss.isListening ()) { 
Serial.println ("gps listening"); 
while (fireData.equals ("")) { 
scan ();} 
} 
else { 
Serial.println ("gps not listening"); 
fireData = ""; 
ss.begin (gpsBAUD); 
} 
}

// send data to firebase 
void sendData (const char* method, const String & path, const String & data, HttpClient* http) { 
String response; 
http-> connectionKeepAlive (); // Currently, this is needed for HTTPS 
String url; 
if (path[0] != '/') { 
url = "/"; 
} 
url += path + ".json"; 
url += "?print=silent"; 
url += "&x-http-method-override="; 
url += String(method); 
Serial.print( "POST: ");  
Serial.println(url); 
String contentType = "application/json"; 
http->post(url, contentType, data); 
// read the status code and body of the response 
int statusCode = http->responseStatusCode(); 
Serial.print ("Status code:");
Serial.println (statusCode); 
response = http-> responseBody (); 
Serial.print( "Response: "); 
Serial.println(response);

if (!http-> connected ()) { 
Serial.println (); 
http-> stop (); //shutdown
Serial.println("HTTP POST disconnected"); 
SerialAT.begin (modemBAUD); 
connection (); 
fireData = ""; 
ss.begin (gpsBAUD); 
} 
}

// scanning gps location 
void scan () { 
while (ss.available ()> 0) 
if (gps.encode (ss.read ())) 
displayInfo (); 
}

void displayInfo () 
{ 
Serial.print (F ("Realtime Loc:")); 
if (gps.location.isValid ()) 
{ 
Serial.print (gps.location.lat (), 6); 
Serial.print (","); 
Serial.print (gps.location.lng (), 6); 
data();
Serial.println ();  
SerialAT.begin (modemBAUD); 
updateData ();
} 
else 
{ 
Serial.print ("INVALID");
Serial.println ();
delay (300); 
 
 
}  
}

// connect to internet 
void connection () { 
tes: 
SerialMon.print (F ("Connection to Network ...")); 
if (!modem.waitForNetwork ()) { 
SerialMon.println ("failed"); 
delay (300); 
goto tes; 
} 
SerialMon.println ("OK");

SerialMon.print (F ("Connection to APN:")); 
SerialMon.print (apn); 
if (!modem.gprsConnect (apn, user, pass)) { 
SerialMon.println ("failed"); 
delay (300);

goto tes; 
} 
SerialMon.println ("OK");

}

// update the firebase 
void updateData () { 
sendData ("PUT", UPDATE_PATH, fireData, &https); 
}

// initialize the 
void initializeModem () { 
SerialAT.begin (modemBAUD); 
delay (3000); 
SerialMon.println (F ("Initializing modem ...")); 
modem.restart();

String modemInfo = modem.getModemInfo(); 
SerialMon.print (F ("Modem:")); 
SerialMon.println (modemInfo); 
if (!modem.hasSSL()) { 
SerialMon.println (F ("SSL is not supported by this modem")); 
while (true) {delay (1000); } 
} 
Serial.println ("wait 10 seconds"); 
delay(10000); 
}

// get gps data 
void data () { 
fireData = ""; 
fireData += "{"; 
fireData += "\"Longitute\":\" "+ String (gps.location.lng (), 6) +" \"," ;  
fireData += "\"Latitute\":\" "+ String (gps.location.lat (), 6) +" \"" ;
fireData += "}"; 
} 

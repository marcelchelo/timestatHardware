//Connect to IFTTT arduino mkrGSM 1400 and post fingerprint data to google sheets. 
//by Marcelo Villalba

#include <Adafruit_Fingerprint.h>
#include <ACROBOTIC_SSD1306.h>
#include <Wire.h>
#define mySerial Serial1
#include <MKRGSM.h>
#include "arduino_secrets.h" 


// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = SECRET_PINNUMBER;
// APN data
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

//name and id of each person
const char* NAME;
const char* ID;
const char* LOCATION = "Jamaica";


// initialize the library instance
GSMClient client;
GPRS gprs;
GSM gsmAccess;

// URL, path and port (for example: example.org)
char server[] = "maker.ifttt.com";
char path[] = "/";
int port = 80; // port 80 is the default for HTTP

//WEBHOOK info 
String Event_Name = "Fingerprint";
//String Key = "cYr-jsTSkUXGq2NuVe2bYd";
String Key   = SECRET_KEY;

String resource = "/trigger/" + Event_Name + "/with/key/" + Key;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  oled.init();
  oled.clearDisplay();
  oled.setTextXY(0,0);
  
  //remove this line when using power only. 
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//    Serial.print("connect Component");
//  }

  bool connected = false;
  while(!connected){
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
      oled.clearDisplay();
      oled.setTextXY(0,0);
      oled.putString("GSM OK");
      delay(100);
      
    } else {
      Serial.println(" GSM Not connected");
      oled.clearDisplay();
      oled.setTextXY(0,0);
      oled.putString("GSM Not OK");
      delay(100);
      
  
    }
  }

  Serial.println("Conectando");
  oled.setTextXY(0,0);
  oled.putString("GSM ok");
  
  
  oled.setTextXY(1,0);
  oled.putString("Welcome");
  oled.setTextXY(2,0);
  //oled.putString("Connect to IDE");
  Serial.print("Connected to oled");
  // initialize serial communications and wait for port to open: Use only in development, connecteted to computer
 
//   while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//    Serial.print("connect Component");
//  }

  //Connect to Fingerprint.
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(2);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor");
    oled.setTextXY(3,0);
    oled.putString("Sensor OK");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    oled.setTextXY(3,0);
    oled.putString("Sensor !OK"); 
    while (1) { delay(1); }
  }
  
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
  

}

void loop() {
  // put your main code here, to run repeatedly:
  getFingerprintID();
  if (finger.fingerID == 1) {

    Serial.print("!!--");
    Serial.println(finger.fingerID);
    NAME = "marcelo";
    ID = "1";
    if (finger.confidence >= 60) {
      Serial.print("Attendace Marked for "); Serial.println(NAME);
      makeIFTTTRequest();
  
      // digital write - open the door
    }

  }
  //delay(50); 
  delay(50);

  //reset the to zero so it wont run for ever. 
  finger.fingerID = 0;
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      oled.clearDisplay();
      oled.setTextXY(0,0);
      oled.putString("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
   p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  String ididen =  String(finger.fingerID);
  oled.setTextXY(1,0);
  oled.putString("Hola ID:" + ididen);

  //send http get request, get name. 
  //print name instad of ID

  
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

  void makeIFTTTRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);

  client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected()) {
    Serial.println("Failed to connect...");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);

//  
//  String jsonObject = String("{\"value1\":\"") + NAME + "\",\"value2\":\"" + ID
//                      + "\"}";
  String jsonObject = String("{\"value1\":\"") + NAME + "\",\"value2\":\"" + ID
                      + "\",\"value3\":\"" + LOCATION +  "\"}";

  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No response...");
  }
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("\n closing connection");
  oled.clearDisplay();
  oled.setTextXY(0,0);
  client.stop();
  oled.putString("conn Closed");
  oled.clearDisplay();
  oled.setTextXY(0,0);
  oled.putString("Thanks!");
  oled.setTextXY(1,0);
  oled.putString("Ready!");
  
  
  
}

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Replace with your network credentials
// Root certificate for howsmyssl.com
const char IRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ
RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD
VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX
DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y
ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy
VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr
mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr
IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK
mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu
XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy
dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye
jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1
BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3
DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92
9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx
jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0
Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz
ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS
R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp
-----END CERTIFICATE-----
)CERT";
const char* ssid = "ALHN-792C";
const char* password = "2Zm3rksa3a";
bool ready = false;
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
String input;
String url = "https://api.openai.com/v1/completions";

void inputKey(){
  while(true){
    if (Serial.available()){
      static byte ndx = 0;
      char endMarker = '\n';
      char rc;
      
      while (Serial.available() > 0 && newData == false) {
          rc = Serial.read();
          display.print(rc);
          display.display();
          if (rc != endMarker) {
              receivedChars[ndx] = rc;
              ndx++;
              if (ndx >= numChars) {
                  ndx = numChars - 1;
              }
          }
          else {
              receivedChars[ndx] = '\0'; // terminate the string
              ndx = 0;
              newData = true;
          }
      }
      break;
    }
  }
}

void cmd(){
  if (Serial.available()){
      input = Serial.readString();
      ready = true;
      input[input.length() - 1] = ' ';
      display.setCursor(0, 0);
      display.println(input);
      display.display();
    }
}

// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  // display.clearDisplay();  // Clear the buffer
  // display.display();
  // //Connect to Wi-Fi
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  // display.println(F("Please enter the ssid"));
  // display.display();
  // inputKey();
  // display.clearDisplay();  // Clear the buffer
  // display.display();
  // display.setCursor(0, 0);
  // display.println(F("Please enter password"));
  // display.display();
  // inputKey();
  display.clearDisplay();  // Clear the buffer
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  display.setCursor(0, 0);
  display.print("Connecting to WiFi ..");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    display.print('.');
    display.display();
    delay(1000);
  }
  display.clearDisplay();  // Clear the buffer
  display.display();
  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
}

void loop() {

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClientSecure* client = new WiFiClientSecure;
    client->setTrustAnchors(&cert);
    cmd();

    if (client && ready) {
      ready = false;
      HTTPClient https;
      //create an HTTPClient instance
      //Initializing an HTTPS communication using the secure client
      if (https.begin(*client, url)) {  // HTTPS
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", "Bearer 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");
        // start connection and send HTTP header
        String maxt = "88";
        https.setTimeout(10000);
        int httpCode = https.POST(String("{\"model\":\"text-davinci-003\",\"prompt\":\"" + input + "\",\"max_tokens\":" + maxt +"}"));
        // httpCode will be negative on error
        if (httpCode > 0) {
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            DynamicJsonDocument doc(1024);
            String response = https.getString();
            deserializeJson(doc, response);
            display.println((doc["choices"][0]["text"].as<String>())); // Print the text of the first choice
            display.display();
            while(!Serial.available()){}; 
          }
        }
        else {
          display.println(F("[HTTPS] GET... failed, error: "));
        }
        
      } else {
        display.println(F("[HTTPS] Unable to connect\n"));
      }
      display.clearDisplay();  // Clear the buffer
      display.display();
      https.end();
    }delete client;
    
  }
}
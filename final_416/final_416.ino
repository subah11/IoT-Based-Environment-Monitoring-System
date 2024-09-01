#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "DHT.h"
DHT dht(D6, DHT11); // check dht pin no. !!  D5
LiquidCrystal_I2C lcd(0x27, 16, 2);
 


#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
String apiKey = "7YCT8QEHZA6PADOI";
const char *ssid     = "Shafim2";
const char *password = "shafimbh";
const char* server = "api.thingspeak.com";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(D4, D3); //SIM800L Tx & Rx is connected to Arduino #3 & #2

WiFiClient client;
void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  lcd.init();
  lcd.clear();
  lcd.backlight();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");
  dht.begin();
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"01758869917\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();

  timeClient.begin();

  timeClient.setTimeOffset(21600);

  delay(1500);
}

void loop()
{
  timeClient.update();


  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);


  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float g = analogRead(A0);
  if (isnan(g)) {
    Serial.println("Failed to read from MQ-5 sensor!");
    return;
  }
  Serial.print(t);
  Serial.print(" ");
  Serial.print(h);
  Serial.print(" ");
  Serial.print(g);
  Serial.print(" ");
  Serial.println();

  float gaslevel = g -323;
  gaslevel=map(gaslevel,0,1024,400,5000);
  String msg;
  if(gaslevel<450)
  {
    msg="Good.";
  }
  else if(gaslevel<1000)
  {
    msg="Average.";
  }
  else
  {
    msg="Dangerous";
  }

  lcd.clear();
  lcd.setCursor (0, 0);
  lcd.print("Temp: ");
  lcd.setCursor(7, 0);
  lcd.print(t);
  lcd.setCursor(12, 0);
  lcd.print("*C");
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.setCursor(7, 1);
  lcd.print(h);
  lcd.setCursor(12, 1);
  lcd.print("%");

  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AQ INDEX: ");
  lcd.setCursor(3, 1);
  lcd.print(g);

delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
  lcd.setCursor(3, 1);
  lcd.print(gaslevel);
  

  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(g / 1023 * 100);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }


  if (t > 30 || h > 60 || g > 350)
  {
    Serial.println();
    Serial.print("Sending SMS");
    Serial.println();
    mySerial.print(formattedTime);
    updateSerial();
    mySerial.print(" Temp: ");
    updateSerial();
    mySerial.print(t); //text content
    updateSerial();
    mySerial.print(" Humidity: "); //text content
    updateSerial();
    mySerial.print(h); //text content
    updateSerial();
    mySerial.print("% "); //text content
    updateSerial();
    mySerial.print(" air quality: "); //text content
    updateSerial();
    mySerial.print(g); //text content
    updateSerial();
    mySerial.write(26);
  }
  delay(6000);
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

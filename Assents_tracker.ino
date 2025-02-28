
#include "WiFi.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"
#include <UrlEncode.h>

#define LED 35

HTTPClient http;

const char *ssid = "your SSID";   
const char *pass = "your wifi password";

String Payload = "";
String PhoneNumber = "Tour phone";
String ApiKey = "your api"; //from CallMeBot

void Read_Price(const String& stockname){
  String RequestAdress = "https://finnhub.io/api/v1/quote?symbol=" + stockname + "&token=cv0k499r01qo8ssh96g0cv0k499r01qo8ssh96gg";
  int httpcode;
  http.begin(RequestAdress);
  delay(10);
  httpcode = http.GET();
  delay(10);
  if (httpcode > 0){
    DynamicJsonDocument doc(1024);
    String payload = http.getString();
    //Serial.println(payload);
    deserializeJson(doc, payload);

    float ClosePrice = doc["pc"];
    float CurrentPrice = doc["c"];
    float DifferenceInPrice = ((CurrentPrice - ClosePrice) / ClosePrice) * 100.0;

    if(DifferenceInPrice > 5.0 || DifferenceInPrice < 5.0){
      char message[32];
      sprintf(message, "Btc price: %f \n Variation: %f%%", CurrentPrice, DifferenceInPrice);
      Send_Message(message);
    }

  }else{
    Serial.println("Erro in HTTP request");
  }
  delay(10);
  http.end();
}

void Send_Message(String message){
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + PhoneNumber + "&apikey=" + ApiKey + "&text=" + urlEncode(message);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httResponseCode = http.POST(url);
  
  if (httResponseCode == 200){
    Serial.println("Message sent successfully");
  }else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httResponseCode);
  }
  http.end();
}

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {  // connect to wifi
    Serial.println(WiFi.status());
    WiFi.begin(ssid, pass);
    delay(1000);
  }

}


void loop(){
  Read_Price("BINANCE:BTCUSDT");
  delay(5000);
}
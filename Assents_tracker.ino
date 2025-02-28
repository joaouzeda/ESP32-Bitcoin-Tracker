
#include "WiFi.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"

#define LED 35

HTTPClient http;

const char *ssid = "DoroteiaDiega-2.4";     // wifi name
const char *pass = "Carnesecajerimum5106";

String Payload = "";

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

    Serial.printf("BTC price: %f \n", CurrentPrice);
    Serial.printf("Variation: %f%% \n", DifferenceInPrice);
  }else{
    Serial.println("Erro in HTTP request");
  }
  delay(10);
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
  delay(1000);
}
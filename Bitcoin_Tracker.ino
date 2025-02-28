// -----------------------------------------------------------------
// -----Librarys-----
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"
#include <UrlEncode.h>
#include "WiFi.h"
#include "defs.h"

// -----------------------------------------------------------------
// -----Object-----
HTTPClient http;

const char* ssid  = SSID;
const char* pass  = PASS;

String Payload    = "";
String Phone      = PhoneNumber;
String CallMeApi  = CallMeBotApiKey;

// -----------------------------------------------------------------
// -----Read BTC price----
void Read_Price(const String& stockname){
  String RequestAdress = "https://finnhub.io/api/v1/quote?symbol=" + stockname + 
                              "&token=cv0k499r01qo8ssh96g0cv0k499r01qo8ssh96gg";
  int httpcode;
  http.begin(RequestAdress);
  httpcode = http.GET();

  if (httpcode > 0){
    DynamicJsonDocument doc(1024);
    String payload = http.getString();
    //Serial.println(payload);
    deserializeJson(doc, payload);

    float ClosePrice        = doc["pc"];
    float CurrentPrice      = doc["c"];
    float DifferenceInPrice = ((CurrentPrice - ClosePrice) / ClosePrice) * 100.0;

    if(DifferenceInPrice > 10.0 || DifferenceInPrice < -9.9){
      char message[32];
      sprintf(message, "Btc price: %f \n Variation: %f%%", CurrentPrice, DifferenceInPrice);
      Send_Message(message);
    }

  }else{
    Serial.println("Erro in HTTP request");
  }
  http.end();
}

// -----------------------------------------------------------------
// -----Sendo info to whatsapp-----
void Send_Message(String message){
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + Phone + "&apikey=" +
                                           CallMeApi + "&text=" + urlEncode(message);
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


// -----------------------------------------------------------------
// -----setup-----
void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {  // connect to wifi
    Serial.println(WiFi.status());
    WiFi.begin(ssid, pass);
    delay(1000);
  }
}

// -----------------------------------------------------------------
// -----loop-----
void loop() {
  Read_Price("BINANCE:BTCUSDT");
  delay(900000);
}

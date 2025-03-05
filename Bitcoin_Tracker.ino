// -----------------------------------------------------------------
// -----Librarys-----
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_task_wdt.h" 
#include "ArduinoJson.h"
#include "esp_system.h"
#include "HTTPClient.h"
#include <UrlEncode.h>
#include "WiFi.h"
#include "defs.h"

#define Task_Real_Sync_Flag     (1<<0)

// -----------------------------------------------------------------
// -----Object-----
HTTPClient http;

// -----------------------------------------------------------------
// -----Handles & events-----
TimerHandle_t       xTimer;
EventGroupHandle_t  xEvents;

// -----------------------------------------------------------------
// -----Task-----
void xTaskPriceUpdate(void *pvParameters);
void CallBackTimer(TimerHandle_t pxTimer);

const char* ssid  = SSID;
const char* pass  = PASS;

String Payload    = "";
String Phone      = PhoneNumber;
String CallMeApi  = CallMeBotApiKey;

int counter;


void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  TaskConfig();

  while (WiFi.status() != WL_CONNECTED) {  // connect to wifi
    Serial.println(WiFi.status());
    vTaskDelay(500);
  }
}

void loop(){}

void TaskConfig(){
  xEvents = xEventGroupCreate();
  xTimer = xTimerCreate("Timer", 
                        pdMS_TO_TICKS(1000), 
                        pdTRUE, 
                        0, 
                        CallBackTimer);
  
  xTaskCreatePinnedToCore(xTaskPriceUpdate,
              "Price Update",
              configMINIMAL_STACK_SIZE + (2048*2),
              NULL,
              1,
              NULL,
              1);

  xTimerStart(xTimer, 0);
  Serial.println("RTOS Configured");
}

void CallBackTimer(TimerHandle_t pxTimer){
  counter++;
  if (counter == 10){
    xEventGroupSetBits(xEvents, Task_Real_Sync_Flag);
    counter = 0;
  }
}

void xTaskPriceUpdate(void *pvParameters){
  EventBits_t xBits;
  while(1){
    xBits = xEventGroupWaitBits(xEvents, Task_Real_Sync_Flag, pdTRUE, pdTRUE, portMAX_DELAY);
    Serial.println("Task");
    Read_Price("BINANCE:BTCUSDT");
    vTaskDelay(1000);
  }
}

void Read_Price(const String& stockname){
  String RequestAdress = "https://finnhub.io/api/v1/quote?symbol=" + stockname + "&token=cv0k499r01qo8ssh96g0cv0k499r01qo8ssh96gg";
  int httpcode;
  http.begin(RequestAdress);
  httpcode = http.GET();
  if (httpcode > 0){
    DynamicJsonDocument doc(1024);
    String payload = http.getString();
    deserializeJson(doc, payload);
    float ClosePrice = doc["pc"];
    float CurrentPrice = doc["c"];
    float DifferenceInPrice = ((CurrentPrice - ClosePrice) / ClosePrice) * 100.0;

    if(DifferenceInPrice >= 5.0 || DifferenceInPrice <= 5.0){
      char message[32];
      sprintf(message, "Btc price: %.2f \nVariation: %.2f%%", CurrentPrice, DifferenceInPrice);
      Send_Message(message);
    }

  }else{
    Serial.println("Erro in HTTP request");
  }
  http.end();
}

void Send_Message(String message){
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + Phone + "&apikey=" + CallMeApi + "&text=" + urlEncode(message);
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
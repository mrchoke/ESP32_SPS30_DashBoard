/**
 * Test by MrChoke
 **/
 
#include <WiFi.h>
#include <WebServer.h>

// for blink LED
#include <Ticker.h>

#include "index.h" // html template

// Config here

#define WPA2EN false // true if use WPA2 Enterprise 

#define nodename "Node32 Lite"


// Config Wifi
#define SSIDNAME "APNAME"

#if WPA2EN
  #include "esp_wpa2.h"
  #include <Wire.h>
  
  const char* ssid     = SSIDNAME;
  const char* username = "Username";
  const char* password = "Password";
#else
  const char* ssid     = SSIDNAME;
  const char* password = "Password";
#endif

int counter = 0;
uint8_t error_cnt = 0;

unsigned long Timer1;

WebServer server(80);
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

// Read CPU Temp

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


// Handle URI Functions

void handleRoot() {
 String s = MAIN_page;
 server.send(200, "text/html", s);
}

void handleInfo() {
  int temp = ((temprature_sens_read() - 32) / 1.8); // CPU Temp
   
   // JSON Format
   
    String data = "[{\"name\":\"temp\",\"val\":\""+ String(temp) +"\"}";
    data += ",{\"name\":\"nodename\",\"val\":\"" + String(nodename) +"\"}]";

    // Print out for debug
    Serial.println("Sent data to Client");
    Serial.println(data);
    
    server.send(200, "application/json", data);
}

void handleBoardId() {
    uint8_t chipid[6];
    esp_efuse_read_mac(chipid);
    Serial.printf("%X\n",chipid);
    
  server.send(200, "text/html", "ok");
}
// Check wifi connection and get data from senser and sent to api server

void CheckWifi(){
 
 if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); // LED off
    counter = 0;
  } else if (WiFi.status() != WL_CONNECTED) {
     ticker.attach(0.5, tick); // blink util wifi connected
     #if WPA2EN

     esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
     esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
     esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
     esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
     esp_wifi_sta_wpa2_ent_enable(&config);
     WiFi.begin(ssid);
     
   #else
   
     WiFi.begin(ssid, password);

   #endif

      //  while(WiFi.waitForConnectResult() != WL_CONNECTED){
  while (WiFi.status() != WL_CONNECTED) {       
      delay(500);
      Serial.print(".");
      counter++;
      
      if(counter>=60){ //after 30 seconds timeout - reset board
        ESP.restart();
      }
   }

  }

}
void setup() {
  
   Serial.begin(115200);
   pinMode(LED_BUILTIN, OUTPUT);
   delay(500);
     
  // print out for debug

   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);
   
   WiFi.disconnect(true); // make sure 
   WiFi.mode(WIFI_STA);

   // connect type enterprise or use password only
   
  CheckWifi();
  // print out for debug

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // imprement server handle 
  
  server.on("/", handleRoot);
  server.on("/info", handleInfo);
  server.on("/boardid", handleBoardId);

  // start http server
  
  server.begin();
  Serial.println("HTTP server started at: http://" + String(WiFi.localIP()));

}

void loop() {

  // check wifi connection

  CheckWifi();
  
  // http server alway wait for client
  server.handleClient();
  
  delay(1);
 
}

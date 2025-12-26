#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "image.h"



#define FW_VERSION 1  // 目前韌體版本，之後要更新就改成 2,3,...

const char* WIFI_SSID = "Sputnik";
const char* WIFI_PASS = "qwerasdf";

// 下面這兩個 URL 等一下會對應到你 repo 裡的檔案
const char* VERSION_URL =
  "https://raw.githubusercontent.com/tzuchen/E6-display/main/version.txt";

const char* FIRMWARE_URL =
  "https://raw.githubusercontent.com/tzuchen/E6-display/main/firmware.bin";

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
}

// 從 GitHub 抓 version.txt，看是否有新版本
bool otaHasNewVersion() {
  connectWiFi();

  HTTPClient http;
  http.begin(VERSION_URL);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Version check failed, HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  payload.trim();
  int remoteVer = payload.toInt();
  Serial.printf("Local FW_VERSION=%d, Remote=%d\n", FW_VERSION, remoteVer);

  return (remoteVer > FW_VERSION);
}

// 真的執行 OTA 更新
bool otaDoUpdate() {
  connectWiFi();

  WiFiClientSecure client;
  client.setInsecure();  // 開發用：不驗證 SSL 憑證，最省事

  HTTPClient http;
  http.begin(client, FIRMWARE_URL);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Firmware download failed, HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
  WiFiClient * stream = http.getStreamPtr();

  if (!Update.begin(contentLength)) {
    Serial.println("Not enough space for OTA");
    http.end();
    return false;
  }

  Serial.printf("Start OTA, size=%d bytes\n", contentLength);
  size_t written = Update.writeStream(*stream);
  if (written != (size_t)contentLength) {
    Serial.printf("Written %u/%d bytes\n", (unsigned)written, contentLength);
    http.end();
    return false;
  }

  if (!Update.end()) {
    Serial.printf("Update error: %s\n", Update.errorString());
    http.end();
    return false;
  }

  http.end();
  Serial.println("OTA OK, rebooting...");
  delay(1000);
  ESP.restart();
  return true; // 到這裡其實不會回來
}

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("\nBooting...");

  // 1. 先檢查有沒有新版本
  if (otaHasNewVersion()) {
    Serial.println("New firmware available, starting OTA...");
    otaDoUpdate();   // 成功的話這裡不會回來，會直接重啟
  } else {
    Serial.println("Firmware is up to date.");
  }



   pinMode(A14, INPUT);  //BUSY
   pinMode(A15, OUTPUT); //RES 
   pinMode(A16, OUTPUT); //DC   
   pinMode(A17, OUTPUT); //CS   
   //SPI
   SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
   SPI.begin ();  
}


//Tips//
/*
1.Flickering is normal when EPD is performing a full screen update to clear ghosting from the previous image so to ensure better clarity and legibility for the new image.
2.There will be no flicker when EPD performs a partial refresh.
3.Please make sue that EPD enters sleep mode when refresh is completed and always leave the sleep mode command. Otherwise, this may result in a reduced lifespan of EPD.
4.Please refrain from inserting EPD to the FPC socket or unplugging it when the MCU is being powered to prevent potential damage.)
5.Re-initialization is required for every full screen update.
6.When porting the program, set the BUSY pin to input mode and other pins to output mode.
*/
void loop() {
  
#if 1//Full screen refresh demostration.

   /************Full display*******************/
    EPD_init_fast(); //Full screen refresh initialization.
    PIC_display(gImage_1);//To Display one image using full screen refresh.
    EPD_sleep();//Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    delay(5000); //Delay for 5s.
  
    
  #if 1//Demonstration of Dispaly colored stripes, to enable this feature, please change 0 to 1.
   /************Full display*******************/
    EPD_init(); //Full screen refresh initialization.
   EPD_Display_White();
   delay(5000); //Delay for 5s.
   EPD_Display_Black();
   delay(5000); //Delay for 5s.
   EPD_Display_Yellow();
   delay(5000); //Delay for 5s.
   EPD_Display_blue();
   delay(5000); //Delay for 5s.
   EPD_Display_Green();
   delay(5000); //Delay for 5s.
   EPD_Display_red();
   delay(5000); //Delay for 5s. 
    
  #endif    
    EPD_init(); //Full screen refresh initialization.

    PIC_display_Clear(); //Clear screen function.
 
    EPD_sleep();//Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.. 
     delay(2000);   
#endif        
    while(1); // The program stops here            
   
}

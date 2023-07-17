
#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Arduino_GFX_Library.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
bool setupSD(uint8_t pin)
{
  if(!SD.begin(25)){
    Serial.println("Card Mount Failed");
    return false;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);
  Serial.println("");
  return true;
}


Arduino_DataBus* bus = new Arduino_HWSPI(17, 16);
Arduino_GFX* gfx = new Arduino_ST7735(bus, 5, 0, false, 128,128, 1, 1, 0, 0, false);

char bmp[128*128*2];

void setup(){

  Serial.begin(115200);
  while(!setupSD(25)){
    Serial.println("SD initialization failed!");
    delay(500);
  }
  Serial.println("SD initialization done.");

  auto file = SD.open("/BRUH.bmp");
  if(!file.available()){
    Serial.println("File not available!");
  }

  

  auto res = file.readBytes(bmp, sizeof(bmp));
  if(res != sizeof(bmp)){
    Serial.println("File read failed!");
  };

  while (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }

}

void loop(){
  gfx->draw16bitBeRGBBitmap(0,0, reinterpret_cast<uint16_t*>(bmp), 128, 128);
  delay(5000);
  gfx->fillScreen(RED);
  delay(100);
  gfx->fillScreen(GREEN);
  delay(100);
  gfx->fillScreen(BLUE);
  delay(100);
}

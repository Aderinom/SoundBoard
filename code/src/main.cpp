
#include <Arduino.h>

#include "SD.h"
#include "SPI.h"
#include <Arduino_GFX_Library.h>

#define SD_CS_PIN 7
#define TFT_CS 6
#define TFT_DC 5
#define TFT_RST 4

#define TFT2_DC 3
#define TFT2_CS 2
#define TFT2_RST 8

// struct Button
// {
//   const uint8_t PIN;
//   uint32_t numberKeyPresses;
//   uint32_t lastPress;
//   bool pressed;
// };

// Button button1 = {BUTTON2_PIN, 0, 0, false};
// void buttonHandler()
// {

//   if (button1.lastPress + 250 > millis())
//   {
//     return;
//   }

//   button1.numberKeyPresses++;
//   button1.lastPress = millis();
//   button1.pressed = true;
// }

typedef struct __attribute__((packed))
{                            // Total: 54 bytes
  uint16_t type;             // Magic identifier: 0x4d42
  uint32_t size;             // File size in bytes
  uint16_t reserved1;        // Not used
  uint16_t reserved2;        // Not used
  uint32_t offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  uint32_t dib_header_size;  // DIB Header size in bytes (40 bytes)
  int32_t width_px;          // Width of the image
  int32_t height_px;         // Height of image
  uint16_t num_planes;       // Number of color planes
  uint16_t bits_per_pixel;   // Bits per pixel
  uint32_t compression;      // Compression type
  uint32_t image_size_bytes; // Image size in bytes
  int32_t x_resolution_ppm;  // Pixels per meter
  int32_t y_resolution_ppm;  // Pixels per meter
  uint32_t num_colors;       // Number of colors
  uint32_t important_colors; // Important colors
} BMPHeader;

bool setupSD(uint8_t pin)
{
  if (!SD.begin(pin))
  {
    Serial.println("Card Mount Failed");
    return false;
  }

  Serial.println("");
  return true;
}

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX *gfx = new Arduino_ST7735(bus, TFT_RST, 0, false, 128, 128, 2, 0, 0, 0, true);

Arduino_DataBus *bus2 = new Arduino_HWSPI(TFT2_DC, TFT2_CS);
Arduino_GFX *gfx2 = new Arduino_ST7735(bus2, TFT2_RST, 0, false, 128, 128, 2, 0, 0, 0, true);

SDLib::File file;
SDLib::File file2;

void setup()
{

  // pinMode(button1.PIN, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(button1.PIN), buttonHandler, FALLING);

  Serial.begin(115200);
  while (!setupSD(SD_CS_PIN))
  {
    Serial.println("SD initialization failed!");
    delay(500);
  }
  Serial.println("SD initialization done.");

  file = SD.open("/BRUH.bmp");
  if (!file.available())
  {
    Serial.println("File not available!");
  }

  file2 = SD.open("/SOVIET.bmp");
  if (!file.available())
  {
    Serial.println("File not available!");
  }

  while (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }

  gfx->invertDisplay(true);
  gfx->fillScreen(BLACK);

  while (!gfx2->begin())
  {
    Serial.println("gfx2->begin() failed!");
  }

  gfx2->invertDisplay(true);
  gfx2->fillScreen(BLACK);
}

struct drawState
{
  uint32_t currentline = 127;
  bool done = false;
};

bool drawNextFileChunk(Arduino_GFX* target, SDLib::File file, drawState &state)
{
  constexpr uint16_t linesPerChunk = 1;
  constexpr uint16_t bytesPerChunk = 128 * 2 * linesPerChunk;

  if (state.done) return false;

  if (state.currentline == 127)
  {
    file.seek(0);
    BMPHeader header;

    if (file.readBytes(reinterpret_cast<char *>(&header), sizeof(BMPHeader)) != sizeof(BMPHeader) || header.type != 0x4D42)
    {
      Serial.println("Not a BMP file!");
      return;
    }

    file.seek(header.offset - 1);
  }

  {
    if (state.currentline <= 0)
      return false;

    char bmp[bytesPerChunk];

    if (file.read(bmp, bytesPerChunk) != bytesPerChunk)
    {
      Serial.println("File read ended early!");
      state.done = true;
      return false;
    }

    target->draw16bitBeRGBBitmap(0, state.currentline, reinterpret_cast<uint16_t *>(bmp), 128, linesPerChunk);
    state.currentline -= linesPerChunk;

    if (state.currentline == 0)
    {
      state.done = true;
      return false;
    }
    else
    {
      return true;
    }
  }
}


bool invert = false;
bool drew = false;
void loop()
{
  drawState state1;
  drawState state2;

  if (!drew)
  {
    file.seek(0);

    bool done = false;
    while (false != drawNextFileChunk(gfx, file, state1) && false != drawNextFileChunk(gfx2, file2, state2))
    {
    }

    drew = true;

    // gfx->fillScreen(BLACK);
  }
  delay(100000);
  // delay(100);

  // invert = !invert;
  // gfx->invertDisplay(invert);
  // gfx2->invertDisplay(!invert);
}

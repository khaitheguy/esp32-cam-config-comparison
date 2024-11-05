#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1          /* Time ESP32 will go to sleep (in seconds) */

int64_t time_before_capture = 0;
int64_t time_after_capture = 0;
int64_t capture_time[5];
int64_t total_capture_time = 0;

framesize_t fs_config[] = {
  FRAMESIZE_UXGA,   
  FRAMESIZE_QVGA,
  FRAMESIZE_CIF,
  FRAMESIZE_VGA,
  FRAMESIZE_SVGA,
  FRAMESIZE_XGA,
  FRAMESIZE_SXGA
};

String fs_name[] = {
  "FRAMESIZE_UXGA",
  "FRAMESIZE_QVGA",
  "FRAMESIZE_CIF",
  "FRAMESIZE_VGA",
  "FRAMESIZE_SVGA",
  "FRAMESIZE_XGA",
  "FRAMESIZE_SXGA"
};

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float fs_avg_time[7];

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  /*
  The frame size can be set to one of these options:

    FRAMESIZE_UXGA (1600 x 1200)
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
  */

  Serial.println("psramFound(): " + String(psramFound()));
  Serial.println("frame size: " + String(config.frame_size));
  Serial.println("jpeg quality: " + String(config.jpeg_quality));
  Serial.println("fb count: " + String(config.fb_count));

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}

void loop() {
  for (int i = 0; i < 5; i++) {
    // Record initial and final time
    time_before_capture = esp_timer_get_time();
    capturePhotoSaveSpiffs();
    time_after_capture = esp_timer_get_time();

    // Calculate time difference
    capture_time[i] = time_after_capture - time_before_capture;
    total_capture_time += capture_time[i];

    Serial.println("Time taken for photo " + String(i) + String(" = ") + String((float)capture_time[i] / (float)1000000) + String(" s"));
  }
  
  // Calculate average of 5 readings
  fs_avg_time[bootCount] = ((float)total_capture_time / (float)1000000) / 5;
  Serial.println("Avg time taken per photo = " + String(fs_avg_time[bootCount]) + String(" s"));

  bootCount++;

  if (bootCount >= 6) {
    Serial.println("Framesize | Average time taken (s)");
    for (int i = 0; i < 7; i++) {
      Serial.println(fs_config[i] + " " + String(fs_avg_time[i]));
    }
    while(true) {
      // Program completed. Loop forever.
    }
  }

  Serial.println("Going into deep sleep...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.flush();
  esp_deep_sleep_start();
}

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

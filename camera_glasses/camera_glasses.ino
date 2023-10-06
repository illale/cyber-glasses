
#include "esp_camera.h"
#include "esp_err.h"
#include "SD_MMC.h"
#include "Wire.h"
#include "FS.h"
#include "FreeRTOSConfig.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"

#define XCLK_GPIO_NUM      0
#define PCLK_GPIO_NUM     22
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define PWDN_GPIO_NUM     32

#define RESET_GPIO_NUM    -1

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define CLK_GPIO_NUM      14
#define CMD_GPIO_NUM      15
#define DATA0_GPIO_NUM    2
#define DATA1_GPIO_NUM    4
#define DATA2_GPIO_NUM    12
#define DATA3_GPIO_NUM    13

#define UART_RX_NUM       3
#define UART_TX_NUM       1
#define GPIO_16           16
#define GPIO_0            0


camera_config_t cam_config = {
  .pin_sccb_sda   = DATA2_GPIO_NUM,
  .pin_sccb_scl   = DATA3_GPIO_NUM
  .pin_pwdn       = PWDN_GPIO_NUM,
  .pin_reset      = RESET_GPIO_NUM,
  .pin_xclk       = XCLK_GPIO_NUM,
  .pin_sccb_sda   = PIN_SIOD,
  .pin_sccb_scl   = PIN_SIOC,
  .pin_d7         = Y9_GPIO_NUM,
  .pin_d6         = Y8_GPIO_NUM,
  .pin_d5         = Y7_GPIO_NUM,
  .pin_d4         = Y6_GPIO_NUM,
  .pin_d3         = Y5_GPIO_NUM,
  .pin_d2         = Y4_GPIO_NUM,
  .pin_d1         = Y3_GPIO_NUM,
  .pin_d0         = Y2_GPIO_NUM,
  .pin_vsync      = VSYNC_GPIO_NUM,
  .pin_href       = HREF_GPIO_NUM,
  .pin_pclk       = PCLK_GPIO_NUM,

  .xclk_freq_hz   = 20000000,
  .ledc_timer     = LEDC_TIMER_0,
  .ledc_channel   = LEDC_CHANNEL_0,
  .pixel_format   = PIXFORMAT_JPEG,
  .frame_size     = FRAMESIZE_SVGA,
  .jpeg_quality   = 10,
  .fb_count       = 2,
  .grab_mode      = CAMERA_GRAB_WHEN_EMPTY
};

TaskHandle_t write_to_i2c_task = NULL;
TaskHandle_t write_to_sd_task = NULL;
QueueHandle_t queue;
File file;

void write_to_i2c(void* parameters) {
  // Do something
  camera_fb_t *framebuffer = esp_camera_fb_get();
  xQueueSend(queue, (void *) framebuffer, (TickType_t) 1000);

  Wire.beginTransmission(10);
  Wire.write();

}

void write_to_sd(void* parameters) {
  // Do something else
  camera_fb_t framebuffer;

  if (xQueueReceive(queue, &framebuffer, (TickType_t) 1000) == dpPASS) {
    file.write()
  }
}


void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

  // Initialize I2C Interface
  Wire.begin();
  
  // Initialize SD Card
  bool begin_fail = SD_MMC.begin("/sdcard", true);

  if (!begin_fail) {
    Serial.println("Failed to mount SD card");
    return;
  }

  pinMode(DATA1_GPIO_NUM, OUTPUT);
  digitalWrite(DATA1_GPIO_NUM, LOW);

  pinMode(DATA1_GPIO_NUM, INPUT);

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  queue = xQueueCreate(10, sizeof(camera_fb_t))

  file =  SD_MMC.open("test.pcm");

  // Initialize camera
  if (esp_camera_init(&cam_config) != ESP_OK) {
    
  }
}

void loop() {
  // put your main code here, to run repeatedly:


    // Start thread that writes video data to I2C interface
    if (write_to_i2c_task == NULL) {
      xTaskCreate(write_to_i2c, "I2C-TASK", 32000, null, 0, &write_to_i2c_task);
    }

    // Start thread that writes video data to SD card
    if (write_to_sd_task == NULL) {
      xTaskCreate(write_to_sd, "SD-TASK", 32000, null, 1, &write_to_sd_task);
    }
}

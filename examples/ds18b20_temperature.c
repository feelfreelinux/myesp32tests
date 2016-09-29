/* 
   This is example for my DS18B20 library
   https://github.com/feelfreelinux/ds18b20
   

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "ds18b20.h" //Include library
const int DS_PIN = 14; //GPIO where you connected ds18b20

void mainTask(void *pvParameters){
  DS_init(DS_PIN);

  while (1) {
    printf("Temperature: %0.1f\n",DS_get_temp());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void app_main()
{
    nvs_flash_init();
    system_init();
    xTaskCreatePinnedToCore(&mainTask, "mainTask", 2048, NULL, 5, NULL, 0);
}

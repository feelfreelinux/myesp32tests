/* Big thanks for MakerAsia for dns example, and stackoverflow.com/questions/26192758/how-can-i-send-a-simple-http-request-with-a-lwip-stack*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/tcp.h"
#include "lwip/ip4_addr.h"
#include "string.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "rom/gpio.h"
#include "lwip/inet.h"
ip4_addr_t ip;
ip4_addr_t gw;
ip4_addr_t msk;
bool bConnected = false;
bool bDNSFound = false;
struct tcp_pcb *testpcb;
err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("Data recieved.\n");
    if (p == NULL) {
        printf("The remote host closed the connection.\n");
        printf("Now I'm closing the connection.\n");
        return ERR_ABRT;
    } else {
        printf("Number of pbufs %d\n", pbuf_clen(p));
        printf("Contents of pbuf %s\n", (char *)p->payload);
    }

    return 0;
}
uint32_t tcp_send_packet(void)
{
    char *string = "HEAD /update.php?api_key=<key>&field1=<data> HTTP/1.0\r\nHost: api.thingspeak.com\r\n\r\n ";
    err_t error;
    /* push to buffer */
        error = tcp_write(testpcb, string, strlen(string), TCP_WRITE_FLAG_COPY);

    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_write)\n", error);
        return 1;
    }

    /* now send */
    error = tcp_output(testpcb);
    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_output)\n", error);
        return 1;
    }
    return 0;
}
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    printf("Connection Established.\n");
    printf("Now sending a packet\n");
    tcp_send_packet();
    return 0;
}
void tcp_setup(void)
{
    uint32_t data = 0xdeadbeef;

    /* create an ip */
    ip_addr_t ip;
    IP_ADDR4(&ip, 184, 106, 153, 149);    //IP of my PHP server

    /* create the control block */
    testpcb = tcp_new();    //testpcb is a global struct tcp_pcb
                            // as defined by lwIP


    /* dummy data to pass to callbacks*/

    tcp_arg(testpcb, &data);

    /* register callbacks with the pcb */

    tcp_err(testpcb, NULL);
    tcp_recv(testpcb, tcpRecvCallback);
    tcp_sent(testpcb, NULL);

    /* now connect */
    tcp_connect(testpcb, &ip, 80, connectCallback);

}
esp_err_t wifi_event_cb(void *ctx, system_event_t *event)
{
    if( event->event_id == SYSTEM_EVENT_STA_GOT_IP ) {
        ip = event->event_info.got_ip.ip_info.ip;
        gw = event->event_info.got_ip.ip_info.gw;
        msk = event->event_info.got_ip.ip_info.netmask;
        bConnected = true;
    }

    return ESP_OK;
}
void connect_wifi(void){
  esp_event_set_cb(wifi_event_cb, NULL);

  printf("Set mode to STA\n");
  esp_wifi_set_mode(WIFI_MODE_STA);

  wifi_config_t config;
  memset(&config,0,sizeof(config));

  strcpy( config.sta.ssid, "SSID" );
  strcpy( config.sta.password, "Password" );

  printf("Set config\n");
  esp_wifi_set_config( WIFI_IF_STA, &config );

  printf("Start\n");
  esp_wifi_start();

  printf("Connect\n");
  esp_wifi_connect();

  while( !bConnected )
      ;

  printf("Got IP: %s\n", inet_ntoa( ip ) );
  printf("Net mask: %s\n", inet_ntoa( msk ) );
  printf("Gateway: %s\n", inet_ntoa( gw ) );
}
void mainTask(void *pvParameters){
  connect_wifi();
  tcp_setup();
  while (1) {
    GPIO_OUTPUT_SET(25,1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    GPIO_OUTPUT_SET(25,0);
    if(GPIO_INPUT_GET(26)) printf("OK\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

  }
}
void app_main()
{
  xTaskCreatePinnedToCore(&mainTask, "mainTask", 2048, NULL, 5, NULL, 0);
}

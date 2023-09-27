/* Based on the ESP-IDF WiFi station Example (see https://github.com/espressif/esp-idf/tree/release/v4.4/examples/wifi/getting_started/station/main)

   This example code extends the WiFi example with the necessary calls to establish an
   OCPP connection on the ESP-IDF. 
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_timer.h" //ESP high-precision timer

/* MicroOcpp includes */
#include <mongoose.h>
#include <MicroOcpp_c.h> //C-facade of MicroOcpp
#include <MicroOcppMongooseClient_c.h> //WebSocket integration for ESP-IDF
#include "cxx_intf.h" //extra MicroOcpp functions which are not available in the C-API

#define MICROS_PER_SEC 1000000

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
#define EXAMPLE_MOCPP_OCPP_BACKEND    CONFIG_MOCPP_OCPP_BACKEND
#define EXAMPLE_MOCPP_CHARGEBOXID     CONFIG_MOCPP_CHARGEBOXID
#define EXAMPLE_MOCPP_AUTHORIZATIONKEY CONFIG_MOCPP_AUTHORIZATIONKEY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

bool ocpp_sim_plugged = false;
bool ocpp_plugged_input() {
    return ocpp_sim_plugged;
}

int ocpp_sim_e = 0;
int ocpp_energy_input() {
    return ocpp_sim_e;
}

float ocpp_sim_max_p = 0.f;
void ocpp_smartcharging_output(float p_max, float e_max, int n_phases) {
    ocpp_sim_max_p = p_max;
    (void)e_max;
    (void)n_phases;
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    OCPP_Connection *loopback = ocpp_loopback_make(); //this will be used for the automated test runs

    /*
     * Duration of cold boot
     */
    int64_t tick_init_begin = esp_timer_get_time();

    /* Initialize Mongoose (necessary for MicroOcpp)*/
    struct mg_mgr mgr;        // Event manager
    mg_mgr_init(&mgr);        // Initialise event manager
    mg_log_set(MG_LL_DEBUG);  // Set log level

    /* Initialize MicroOcpp */
    struct OCPP_FilesystemOpt fsopt = { .use = false, .mount = false, .formatFsOnFail = true};

    OCPP_Connection *osock = ocpp_makeConnection(&mgr,
            NULL, NULL, NULL, NULL, fsopt); //this is created to measure a realistic initialization phase
    ocpp_initialize(loopback, "ESP-IDF charger", "Your brand name here", fsopt);

    ocpp_setConnectorPluggedInput(ocpp_plugged_input);
    ocpp_setEnergyMeterInput(ocpp_energy_input);
    ocpp_setSmartChargingOutput(ocpp_smartcharging_output);

    int64_t tick_init = esp_timer_get_time() - tick_init_begin;

    vTaskDelay(1);

    /*
     * The first loop runs executes a few special initialization routines. Take their maximum execution time
     */
    int64_t tick_loop_max = 0;

    for (unsigned int i = 0; i < 128; i++) {
        int64_t tick_loop_begin = esp_timer_get_time();
        ocpp_loop();
        int64_t tick_loop_d = esp_timer_get_time() - tick_loop_begin;
        if (tick_loop_d > tick_loop_max) {
            tick_loop_max = tick_loop_d;
        }
    }

    vTaskDelay(1);

    /*
     * Average idle core usage
     */
    int64_t tick_avg_begin = esp_timer_get_time();
    const int64_t N_AVG = 10000;

    for (unsigned int i = 0; i < N_AVG; i++) {
        ocpp_loop();
    }

    int64_t tick_avg = (esp_timer_get_time() - tick_avg_begin) / N_AVG;

    vTaskDelay(1);

    /*
     * GetDiagnostics execution time (large payload, but not SPI-bus-bound)
     */
    int64_t tick_gdiag_begin = esp_timer_get_time();
    const int64_t N_GDIAG = 10; //no of GDiag messages to be sent
    const int64_t N_GDIAG_IDLE = 10; //no of idle loop calls between each GDiag
    const int64_t DELAY_GDIAG_MICROS = 100 * 1000; //delays

    for (unsigned int i = 0; i < N_GDIAG; i++) {
        ocpp_send_GetDiagnostics(loopback); //processed immediately
        ocpp_loopback_set_connected(loopback, false); //response is created but won't be sent
        
        for (unsigned int i = 0; i < N_GDIAG_IDLE; i++) {
            ocpp_loop();
        }

        ocpp_loopback_set_connected(loopback, true); //connect again

        vTaskDelay(1);
    }

    int64_t tick_gdiag = (esp_timer_get_time() - tick_gdiag_begin - (N_GDIAG_IDLE * tick_avg)) / N_GDIAG;

    /*
     * Transaction cycle execution time (heavily SPI-bus-bound)
     */
    int64_t tick_tx_begin = esp_timer_get_time();
    const int64_t N_TX = 10;
    const int64_t N_TX_IDLE = 100;

    ocpp_sim_plugged = true; //connector is plugged all the time

    for (unsigned int i = 0; i < N_TX; i++) {
        ocpp_beginTransaction_authorized("mIdTag", NULL);

        for (unsigned int i = 0; i < N_TX_IDLE; i++) {
            ocpp_loop();
        }

        ocpp_endTransaction("mIdTag", NULL);

        for (unsigned int i = 0; i < N_TX_IDLE; i++) {
            ocpp_loop();
        }

        vTaskDelay(1);
    }

    int64_t tick_tx = (
                esp_timer_get_time() - tick_tx_begin - //total execution time
                (2 * N_TX_IDLE * tick_avg) //substract number of loop calls per tx cycle times average loop time
            ) / N_TX; //average tx cycle time without idle loop time

    /*
     * Duration of deinitialization
     */
    int64_t tick_deinit_begin = esp_timer_get_time();
    ocpp_deinitialize();
    int64_t tick_deinit = esp_timer_get_time() - tick_deinit_begin;

    printf("\n\nBenchark results ===\n"
            "initalization=%" PRId64 "\n" 
            "loop_init_max=%" PRId64 "\n"
            "loop_idle=%" PRId64 "\n"
            "GetDiagnostics=%" PRId64 "\n"
            "transaction_cycle=%" PRId64 "\n"
            "deinitialization=%" PRId64 "\n"
            "\n",
            tick_init,
            tick_loop_max,
            tick_avg,
            tick_gdiag,
            tick_tx,
            tick_deinit);

    while (1) {
        vTaskDelay(1);
    }

    /* Enter infinite loop */
    while (1) {
        mg_mgr_poll(&mgr, 10);
        ocpp_loop();
    }

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    
    /* Deallocate ressources */
    ocpp_deinitialize();
    ocpp_deinitConnection(osock);
    mg_mgr_free(&mgr);
    return;
}

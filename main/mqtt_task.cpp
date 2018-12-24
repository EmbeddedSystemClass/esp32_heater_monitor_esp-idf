#include "config.h" //if missing, declare SSID, PSK, and HEATER_NAME
#include "mqtt_task.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <iterator>
#include "cJSON.h"

#include "queue_struct.h"
#include "state_struct.h"

const static int CONNECTED_BIT = BIT0;

Queue * desired_queue;
struct heater_state_t * desired_state ;
cJSON * root;

void MQTTClient::connect(string server, string username, string password){
    auto mqtt_cfg = new esp_mqtt_client_config_t();
    mqtt_cfg->uri = server.c_str();
    mqtt_cfg->username = username.c_str();
    mqtt_cfg->password = password.c_str();
    mqtt_cfg->event_handle = this->mqtt_event_handler;
    this->mqtt_cfg = mqtt_cfg;

    ESP_LOGI(HEATER_NAME, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    this->mqtt_client_handle = esp_mqtt_client_init(this->mqtt_cfg);
    esp_mqtt_client_start(this->mqtt_client_handle);
}

void send_json(const char* data){
    root = cJSON_Parse(data);
    desired_state->thermostat_setpoint = cJSON_GetObjectItem(root,"thermostat_setpoint")->valueint;
    desired_state->hold_mode = cJSON_IsTrue(cJSON_GetObjectItem(root,"hold_mode")) == 1;
    desired_state->power = cJSON_IsTrue(cJSON_GetObjectItem(root,"power")) == 1;
    desired_state->idle = false;
    desired_state->set_hold_mode = false;

    desired_queue->Enqueue((void *) desired_state);
}

esp_err_t MQTTClient::mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, TOPIC_IN, 0);
            ESP_LOGI(HEATER_NAME, "sent subscribe successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_publish(client, TOPIC_OUT, "true", 0, 0, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            send_json(event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(HEATER_NAME, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

EventGroupHandle_t wifi_event_group;

void WiFiClient::connect(string ssid, string psk){

    this->ssid = ssid;

    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init((system_event_cb_t)this->wifi_event_handler, NULL));
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT()
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    this->wifi_config = new wifi_config_t();
    this->wifi_sta = new wifi_sta_config_t();
    copy(ssid.begin(), ssid.end(), this->wifi_sta->ssid);
    copy(psk.begin(), psk.end(), this->wifi_sta->password);
    memcpy(&this->wifi_config->sta, this->wifi_sta, sizeof(wifi_sta_config_t));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, this->wifi_config));
    ESP_LOGI(HEATER_NAME, "start the WIFI SSID:[%s] password:[%s]", this->ssid.c_str() , "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(HEATER_NAME, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

esp_err_t WiFiClient::wifi_event_handler(void *ctx, system_event_t *event){
        switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

WiFiClient * wifi_client;
MQTTClient * mqtt_client;

void mqttTask(void * queuePtr){

    struct queues_t * queues = (queues_t*) queuePtr; //casts the void* mess to a queue since we pass it through
    desired_queue = queues->desired_state;
    desired_state = new heater_state_t();

    nvs_flash_init();
    wifi_client = new WiFiClient();
    wifi_client->connect(SSID, PSK);
    mqtt_client = new MQTTClient();
    mqtt_client->connect(MQTT_SERVER, MQTT_USER, MQTT_PASS);
    while(1){
    }
}
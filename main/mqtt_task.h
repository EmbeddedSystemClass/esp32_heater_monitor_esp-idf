#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "freertos/event_groups.h"

#include <string>

using namespace std;

void mqttTask(void *);

class MQTTClient {
    public:
        void connect(string server, string username, string password);
    private:
        static esp_mqtt_client_handle_t esp_mqtt_client_handle;
        static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
        esp_mqtt_client_handle_t mqtt_client_handle;
        const esp_mqtt_client_config_t * mqtt_cfg;
};

class WiFiClient {
    public:
        void connect(string ssid, string psk);
        string ssid;
    private:
        static void wifi_init();
        static esp_err_t wifi_event_handler(void *, system_event_t*);
        wifi_config_t * wifi_config;
        wifi_sta_config_t * wifi_sta;
};
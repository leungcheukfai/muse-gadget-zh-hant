#pragma once

#include <stdbool.h>

#include "esp_err.h"

typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST = 1,
} esp_http_client_method_t;

typedef enum {
    HTTP_EVENT_ERROR = 0,
    HTTP_EVENT_ON_DATA = 4,
} esp_http_client_event_id_t;

struct fake_esp_http_client;
typedef struct fake_esp_http_client *esp_http_client_handle_t;

typedef struct {
    esp_http_client_event_id_t event_id;
    void *user_data;
    void *data;
    int data_len;
} esp_http_client_event_t;

typedef esp_err_t (*esp_http_client_event_cb_t)(esp_http_client_event_t *event);

typedef struct {
    const char *url;
    int timeout_ms;
    int buffer_size;
    int buffer_size_tx;
    bool disable_auto_redirect;
    esp_http_client_event_cb_t event_handler;
    void *user_data;
    void *crt_bundle_attach;
} esp_http_client_config_t;

typedef struct fake_esp_http_client {
    esp_http_client_config_t config;
    esp_http_client_method_t method;
    char header_name[32];
    char header_value[256];
    char post_body[2048];
    int post_length;
    int status_code;
    bool cleaned;
} fake_esp_http_client_t;

extern fake_esp_http_client_t fake_http_client;
extern const char *fake_http_response;
extern int fake_http_status_code;

esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config);
esp_err_t esp_http_client_set_method(esp_http_client_handle_t client,
                                     esp_http_client_method_t method);
esp_err_t esp_http_client_set_header(esp_http_client_handle_t client,
                                     const char *key, const char *value);
esp_err_t esp_http_client_set_post_field(esp_http_client_handle_t client,
                                         const char *data, int len);
esp_err_t esp_http_client_perform(esp_http_client_handle_t client);
int esp_http_client_get_status_code(esp_http_client_handle_t client);
esp_err_t esp_http_client_cleanup(esp_http_client_handle_t client);
const char *esp_err_to_name(esp_err_t error);

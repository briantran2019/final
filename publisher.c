#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>
#include <string.h>

void publish_message(struct mosquitto *mosq, const char *topic, const char *message)
{
    int ret = mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
    if (ret != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Error publishing message: %s\n", mosquitto_strerror(ret));
    }
}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq;
    char *topic = "test/topic";
    const char *json_string;

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto runtime instance with a random client ID
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq)
    {
        fprintf(stderr, "Could not create Mosquitto instance\n");
        exit(-1);
    }

    // Connect to an MQTT broker
    if (mosquitto_connect(mosq, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Could not connect to broker\n");
        exit(-1);
    }

    // Prepare the JSON message
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "example");
    cJSON_AddNumberToObject(root, "number", 42);

    // Task
    cJSON_AddStringToObject(root, "task", "get_temperature");
    // cJSON_AddStringToObject(root, "task", "get_pressure");
    // cJSON_AddStringToObject(root, "string_msg", "hello world");
    cJSON_AddNumberToObject(root, "int_msg", 12);

    json_string = cJSON_Print(root);

    // Publish the message
    publish_message(mosq, topic, json_string);

    // Cleanup
    free((void *)json_string);
    cJSON_Delete(root);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

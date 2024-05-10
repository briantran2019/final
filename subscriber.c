#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <cJSON.h>
#include <string.h>
#include <wiringx.h>
#include "ssd1306.h"
#include "linux_i2c.h"
#include "bmp280_i2c.h"

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if (message->payloadlen)
    {
        printf("%s %s\n", message->topic, (char *)message->payload);
        cJSON *root = cJSON_Parse(message->payload);
        if (root)
        {
            const cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
            if (cJSON_IsString(name) && (name->valuestring != NULL))
            {
                printf("Name: %s\n", name->valuestring);
            }

            const cJSON *number = cJSON_GetObjectItemCaseSensitive(root, "number");
            if (cJSON_IsNumber(number))
            {
                printf("Number: %d\n", number->valueint);
            }

            const cJSON *string_msg = cJSON_GetObjectItemCaseSensitive(root, "string_msg");
            if (cJSON_IsString(string_msg) && (string_msg->valuestring != NULL)) 
            {
                printf("Message: %s\n", string_msg->valuestring);
                char buf[200];
                sprintf(buf, "%s", string_msg->valuestring);
                ssd1306_oled_clear_screen();
                ssd1306_oled_set_XY(0, 0);
                ssd1306_oled_write_string(0, buf);
            }

            const cJSON *int_msg = cJSON_GetObjectItemCaseSensitive(root, "int_msg");
            if (cJSON_IsNumber(int_msg)) 
            {
                printf("Int Num: %d\n", int_msg->valueint);
                char buf[10];
                sprintf(buf, "%d", int_msg->valueint);
                ssd1306_oled_clear_screen();
                ssd1306_oled_set_XY(0, 0);
                ssd1306_oled_write_string(0, buf);
            }

            const cJSON *task = cJSON_GetObjectItemCaseSensitive(root, "task");
            if (cJSON_IsString(task) && (task->valuestring != NULL)) {
                if(strcmp(task->valuestring, "get_temperature") == 0) 
                {
                    printf("Reading temperature\n");
                    struct bmp280_i2c result = read_temp_pressure();
                    char buf[25] = {0};
                    printf("Temperature is %.2f F\n", result.temperature_F);
                    sprintf(buf, "Temp. is %.2f F", result.temperature_F);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buf);
                }
                else if(strcmp(task->valuestring, "get_pressure") == 0) 
                {
                    printf("Reading pressure\n");
                    struct bmp280_i2c result = read_temp_pressure();
                    char buf[25] = {0};
                    printf("Pressure is %.3f psi\n", result.pressure_psi);
                    sprintf(buf, "Pres. is %.3f psi", result.pressure_psi);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buf);
                }
                else if(strcmp(task->valuestring, "get_temperature_pressure") == 0)
                {
                    printf("Reading temperature and pressure\n");
                    struct bmp280_i2c result = read_temp_pressure();
                    char buffer[100] = {0};
                    printf("Temperature: %.2f \nPressure: %.3f psi\n", result.temperature_F, result.pressure_psi);
                    sprintf(buffer, "Temp: %.2fF \\n Pres: %.2f psi", result.temperature_F, result.pressure_psi);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buffer);
            }      

            cJSON_Delete(root);
        }

            
        else
        {
            printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        }
        }
        else
        {
            printf("%s (null)\n", message->topic);
        }
    }
}

void system_init()
{
    if(wiringXSetup("duo", NULL) == -1) 
    {
    wiringXGC();
    }

    bmp280_i2c_init();

    ssd1306_init(0);
    ssd1306_oled_default_config(64, 128);
    ssd1306_oled_clear_screen();
    ssd1306_oled_set_XY(0, 0);

    sleep(3);

}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq;

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    system_init();

    // Create a new Mosquitto runtime instance with a random client ID
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq)
    {
        fprintf(stderr, "Could not create Mosquitto instance\n");
        exit(-1);
    }

    // Assign the message callback
    mosquitto_message_callback_set(mosq, message_callback);

    // Connect to an MQTT broker
    if (mosquitto_connect(mosq, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Could not connect to broker\n");
        exit(-1);
    }

    // Subscribe to the topic
    mosquitto_subscribe(mosq, NULL, "test/topic", 0);

    // Start the loop
    mosquitto_loop_start(mosq);

    printf("Press Enter to quit...\n");
    getchar();

    // Cleanup
    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

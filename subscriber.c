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
                    char buf[25];
                    printf("Temperature is %.2f F\n", result.temperature_F);
                    sprintf(buf, "Temperature is %.2f F\n", result.temperature_F);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buf);
                }
                else if(strcmp(task->valuestring, "get_pressure") == 0) 
                {
                    printf("Reading pressure\n");
                    struct bmp280_i2c result = read_temp_pressure();
                    char buf[25];
                    printf("Pressure is %.3f psi\n", result.pressure_psi);
                    sprintf(buf, "Pressure is %.3f psi\n", result.pressure_psi);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buf);
                }
                else if(strcmp(task->valuestring, "get_temperature_pressure") == 0)
                {
                    printf("Reading temperature and pressure\n");
                    struct bmp280_i2c result = read_temp_pressure();
                    char buf[50];
                    printf("Temperature: %.2f F\nPressure: %.3f psi\n", result.temperature_F, result.pressure_psi);
                    sprintf(buf, "Temperature: %.2f F\nPressure:: %.3f psi", result.temperature_F, result.pressure_psi);
                    ssd1306_oled_clear_screen();
                    ssd1306_oled_set_XY(0, 0);
                    ssd1306_oled_write_string(0, buf);
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

void system_init(){
    /*Initializing WiringX*/
    if(wiringXSetup("duo", NULL) == -1) {
    wiringXGC();
    }

    /*Initializing the sensor*/
    bmp280_i2c_init();
    
    /*Settting the i2c channel for the OLED display*/
    uint8_t i2c_node_address = 0;

    /*Initializing the OLED Display*/
    ssd1306_init(i2c_node_address);

    /*Testing the system*/
    if(true){
        printf("Testing BMP280 Sensor and OLED Display printing.\n");
        struct bmp280_i2c result = read_temp_pressure();
        char buffer[50];

        //Printing current temperature and pressure to the terminal
        printf("Current Temperature: %.2f F\n", result.temperature_F);
        printf("Current Pressure: %.3f psi\n", result.pressure_psi);

        //Format the data into a string and save it to a buffer
        sprintf(buffer, "Temperature: %.2f F \\nPressure: %.3f psi\\n", result.temperature_F, result.pressure_psi);
        
        //Clear the screen before printing
        ssd1306_oled_clear_screen();

        /*Turning display on*/
        ssd1306_oled_onoff(1);

        //Set starting coordinates to (0,0)
        ssd1306_oled_set_XY(0, 0);

        //Print the string to the display
        ssd1306_oled_write_string(0, buffer);

        //Sleep for 3 seconds so you can see the screen
        sleep(3);
    }
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

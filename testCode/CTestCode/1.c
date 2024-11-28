#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    // Initialize the random number seed
    srand(time(NULL));

    // Create a sufficiently large character array to store the JSON data
    char json_data[4096] = "{\"allLEDs\": [\n";

    // Loop to generate data for each LED and append it to json_data
    for (int i = 0; i < 12; i++) {
        char led_info[256]; // Temporary buffer for each LED's JSON data

        int R = rand() % 256; // Randomly generate an R value between 0-255
        int G = rand() % 256; // Randomly generate a G value between 0-255
        int B = rand() % 256; // Randomly generate a B value between 0-255
        int W = 0;            // W value is fixed at 0

        // Format the JSON data for a single LED
        sprintf(led_info, "    {\n"
                          "      \"pixelid\": %d,\n"
                          "      \"R\": %d,\n"
                          "      \"G\": %d,\n"
                          "      \"B\": %d,\n"
                          "      \"W\": %d\n"
                          "    }",
                i, R, G, B, W);

        // Append the LED data to the main JSON string
        strcat(json_data, led_info);

        // Add a comma and newline after each LED entry except the last one
        if (i < 11) {
            strcat(json_data, ",\n");
        }
    }

    // Add the closing brackets for the JSON array and object
    strcat(json_data, "\n  ]\n}");

    // Print the complete JSON data to the console
    printf("%s\n", json_data);

    return 0;
}

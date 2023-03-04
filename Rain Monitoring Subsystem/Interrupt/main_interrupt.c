/**
 * @file    main_basic.c
 * @author  B929164 (Ajay Varghese)
 * @brief   This program is for the rain monitoring sub system.
 *          
*/

// ################################# [ Includes ] #################################

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/sleep.h"
#include <stdio.h>
#include <math.h>

// ############################# [ Global Variables ] #############################

// I2C address of the accelerometer
static const uint8_t ADXL343_ADDR = 0x53;

// Pins Used on the Pi Pico
const uint SDA_PIN_ACC = 4; // I2C SDA Pin for the accelerometer
const uint SCL_PIN_ACC = 5; // I2C SCL Pin for the accelerometer
i2c_inst_t *i2c_ACC = i2c0; // I2C bus for the accelerometer
const uint LED_PIN = 25;    // LED Pin for the Pi Pico
const uint TRIGGER = 10;    // LED Pin for the Pi Pico


const uint SDA_PIN_ZERO = 18;   // I2C SDA Pin for the Zero
const uint SCL_PIN_ZERO = 19;   // I2C SCL Pin for the Zero
i2c_inst_t *i2c_ZERO = i2c1;    // I2C bus for the Zero
const uint WARNING_PIN = 3;     // Warning Pin for the Zero
const uint ACK_PIN = 2;         // Acknowledge Pin for the Zero


// ############################## [ Function Prototypes ] ##########################

/**
 * @brief This sets up the warning pin and the acknowledge pin and sends a 
 * high signal to the warning pin. It then waits for the acknowledge pin to
 * go high before returning.
 * 
 * @param WARNING_PIN The pin to send the warning signal on
 * @param ACK_PIN The pin to wait for the acknowledge signal on
 * @return int 1 if successful 0 if failed
 */
int issue_warning(uint WARNING_PIN, uint ACK_PIN);


int main() 
{
    // Initialize Pi Pico
    stdio_init_all();

    // Setting up the LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Setup the warning pin as an output
    gpio_init(WARNING_PIN);
    gpio_set_dir(WARNING_PIN, GPIO_OUT);

    // Setup the ack pin as an input
    gpio_init(ACK_PIN);
    gpio_set_dir(ACK_PIN, GPIO_IN);

    // Setup the trigger pin as an input
    gpio_init(TRIGGER);
    gpio_set_dir(TRIGGER, GPIO_IN);

    int count = 0;

    // Sets up the pico to be able to go into deep sleep.
    sleep_run_from_xosc();

    // Take measurements from the accelerometer and issue warnings as necessary
    while (1) 
    {

        // Go to deep sleep until high signal is received on the trigger pin
        sleep_goto_dormant_until_level_high(TRIGGER);

        // Print going to sleep to the terminal
        printf("Going to sleep\n");
        uart_default_tx_wait_blocking();

        count++;

        // Print the count to the terminal
        printf("Count: %d\n", count);
        uart_default_tx_wait_blocking();

        // Wait for the trigger pin to go low
        while (gpio_get(TRIGGER) == 1)
        {
            // Set LED to flash quickly to indicate a measurement is being taken
            gpio_put(LED_PIN, 1);
            sleep_ms(100);
            gpio_put(LED_PIN, 0);
            sleep_ms(100);
        }


        // If the count is greater than 2
        if (count > 2)
        {

            // Print warning to the terminal
            printf("Warning\n");
            uart_default_tx_wait_blocking();

            // Issue a warning
            issue_warning(WARNING_PIN, ACK_PIN);

            // Reset the count
            count = 0;
        }

    }
    
}



int issue_warning(uint WARNING_PIN, uint ACK_PIN)
{

    // Setup the warning pin as an output
    gpio_init(WARNING_PIN);
    gpio_set_dir(WARNING_PIN, GPIO_OUT);

    // Set the warning pin high
    gpio_put(WARNING_PIN, 1);

    // Wait for the ack pin to go high
    while (gpio_get(ACK_PIN) == 0)
    {
        // Set LED to flash slowly to indicate warning has been issued
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }

    // Set the warning pin back to high impedance
    gpio_set_dir(WARNING_PIN, GPIO_IN);

    // Turn off LED
    gpio_put(LED_PIN, 0);

    return 1;
}
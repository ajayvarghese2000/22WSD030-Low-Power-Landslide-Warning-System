/**
 * @file    main_basic.c
 * @author  B929164 (Ajay Varghese)
 * @brief   This program is for the Soil Monitoring subsystem. It will be used to
 *          monitor the soil moisture. If the soil moisture is above a certain
 *          threshold, it will send a warning to the Zero. The program will wait
 *          for the Zero to acknowledge the warning before continuing. The soil
 *          moisture sensor will be connected to the Pi Pico via UART.
 * 
 *          This version of the program utilizes the pico's deep sleep mode to 
 *          save power. It is woken up from this state by a timer interrupt.
 *          
*/

// ################################# [ Includes ] #################################

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/sleep.h"
#include <stdio.h>
#include <stdlib.h>

// ############################# [ Global Variables ] #############################

// Pins Used on the Pi Pico
const uint UART_TX_SOIL = 4;    // UART TX Pin for the Soil Sensor
const uint UART_RX_SOIL = 5;    // UART RX Pin for the Soil Sensor
uart_inst_t *uart_SOIL = uart1; // UART bus for the Soil Sensor


const uint LED_PIN = 25;        // LED Pin for the Pi Pico

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

/**
 * @brief Set the up soil sensor object
 * 
 * @param UART_TX_SOIL The UART TX pin that the RX pin of the soil sensor is connected to
 * @param UART_RX_SOIL The UART RX pin that the TX pin of the soil sensor is connected to
 * @param uart_SOIL    The UART bus that the soil sensor is connected to
 * @return int         1 if successful 0 if failed
 */
int setup_soil_sensor(uint UART_TX_SOIL, uint UART_RX_SOIL, uart_inst_t *uart_SOIL);

/**
 * @brief Get the soil moisture object
 * 
 * @param UART_TX_SOIL The UART TX pin that the RX pin of the soil sensor is connected to
 * @param UART_RX_SOIL The UART RX pin that the TX pin of the soil sensor is connected to
 * @param uart_SOIL    The UART bus that the soil sensor is connected to
 * @return int         The soil moisture value or -1 if failed
 */
int get_soil_moisture(uint UART_TX_SOIL, uint UART_RX_SOIL, uart_inst_t *uart_SOIL);


static void sleep_callback(void) 
{

    // for loop that takes 10 readings
    for (int i = 0; i < 10; i++)
    {
        // Get the soil moisture if reading fails, try again
        int soil_moisture = -1;
        while (soil_moisture == -1)
        {
            soil_moisture = get_soil_moisture(UART_TX_SOIL, UART_RX_SOIL, uart_SOIL);
        }

        // print the soil moisture
        printf("Soil Moisture: %d\r\n", soil_moisture);
        uart_default_tx_wait_blocking();

        // Check if the soil moisture is above the threshold
        if (soil_moisture > 50)
        {
            // Issue a warning
            issue_warning(WARNING_PIN, ACK_PIN);
            break;
        }
    }

}

static void rtc_sleep(void) {
    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };

    // Alarm 10 seconds later
    datetime_t t_alarm = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 10
    };

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);

    printf("Sleeping for 10 seconds\n");
    uart_default_tx_wait_blocking();

    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}


int main() 
{
    // Initialize Pi Pico
    stdio_init_all();

    // Sets up the pico to be able to go into deep sleep.
    sleep_run_from_xosc();

    // Setting up the LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Setup the warning pin as an output
    gpio_init(WARNING_PIN);
    gpio_set_dir(WARNING_PIN, GPIO_OUT);

    // Setup the ack pin as an input
    gpio_init(ACK_PIN);
    gpio_set_dir(ACK_PIN, GPIO_IN);

    // Setup the soil sensor
    setup_soil_sensor(UART_TX_SOIL, UART_RX_SOIL, uart_SOIL);

    // Get the soil moisture forever
    while(1)
    {
        // Print message saying that the Pi Pico is going to sleep
        printf("Going to sleep until next interrupt\r\n");
        uart_default_tx_wait_blocking();

        // Go to deep sleep until high signal is received on the trigger pin
        rtc_sleep();

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
        busy_wait_ms(500);
        gpio_put(LED_PIN, 0);
        busy_wait_ms(500);
    }

    // Set the warning pin back to high impedance
    gpio_set_dir(WARNING_PIN, GPIO_IN);

    // Turn off LED
    gpio_put(LED_PIN, 0);

    return 1;
}



int setup_soil_sensor(uint UART_TX_SOIL, uint UART_RX_SOIL, uart_inst_t *uart_SOIL)
{
    // Set up the UART bus
    uart_init(uart_SOIL, 9600);
    gpio_set_function(UART_TX_SOIL, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_SOIL, GPIO_FUNC_UART);

    // Wait for the soil sensor to boot up
    busy_wait_ms(2000);

    // Send a l to the soil sensor to set it up
    uart_putc_raw(uart_SOIL, 'l');

    return 1;
}

int get_soil_moisture(uint UART_TX_SOIL, uint UART_RX_SOIL, uart_inst_t *uart_SOIL)
{
    // Send the command to get the soil moisture
    uart_putc_raw(uart_SOIL, 'w');

    // Wait for the response
    busy_wait_ms(100);

    // Get the response
    char soil_moisture[8];

    int soil_moisture_val = 0;

    // Keep reading until a "=" is found
    while (uart_getc(uart_SOIL) != '=')
    {
        // Do nothing
    }

    // Read in the next character will be the first digit of the soil moisture
    soil_moisture[0] = uart_getc(uart_SOIL);

    // Read in the next character will be the second digit of the soil moisture or a newline
    soil_moisture[1] = uart_getc(uart_SOIL);

    // Check if the character is a newline
    if (soil_moisture[1]== '\n')
    {
        // Add the null terminator
        soil_moisture[1] = '\0';

        // Return the soil moisture
        return atoi(soil_moisture);
    }

    // Read in the next character will be the third digit of the soil moisture or a newline
    soil_moisture[2] = uart_getc(uart_SOIL);

    // Check if the character is a newline
    if (soil_moisture[2]== '\n')
    {
        // Add the null terminator
        soil_moisture[2] = '\0';

        // Return the soil moisture
        return atoi(soil_moisture);
    }
    else if (soil_moisture[2]== '0')
    {
        // Add the null terminator
        soil_moisture[3] = '\0';
        
        return atoi(soil_moisture);
    }
    

    // Read in the next character
    soil_moisture[3] = uart_getc(uart_SOIL);

    // Check if the character is a newline
    if (soil_moisture[3]== '\n')
    {
        // Add the null terminator
        soil_moisture[3] = '\0';

        // Return the soil moisture
        return atoi(soil_moisture);
    }

    // Print error
    printf("Error reading soil moisture\r\n");

    // Return error
    return -1;
}
/**
 * @file    main_basic.c
 * @author  B929164 (Ajay Varghese)
 * @brief   This 
 *          
*/

// ################################# [ Includes ] #################################

#include "pico/stdlib.h"
#include "hardware/uart.h"

// ############################# [ Global Variables ] #############################

// Pins Used on the Pi Pico
const uint UART_TX_SOIL = 4;
const uint UART_RX_SOIL = 5;


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
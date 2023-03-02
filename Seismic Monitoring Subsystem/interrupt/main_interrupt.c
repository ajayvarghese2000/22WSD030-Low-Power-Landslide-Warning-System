/**
 * @file    main_basic.c
 * @author  B929164 (Ajay Varghese)
 * @brief   This program is for the seismic subsystem. It will use the ADXL343
 *          accelerometer to measure the relative acceleration of the system.
 *          If the acceleration exceeds a certain threshold, the system will
 *          issue a warning to the data analysis subsystem then wait until the
 *          warning is acknowledged.
 * 
 *          This version of the program utilizes the pico's deep sleep mode to 
 *          save power. It is woken up from this state by a high level on the
 *          trigger_pin.
 *          
*/

// ################################# [ Includes ] #################################

#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <math.h>

// ############################# [ Global Variables ] #############################

// I2C address of the accelerometer
static const uint8_t ADXL343_ADDR = 0x53;

// Registers Locations on the accelerometer
static const uint8_t REG_DEVID = 0x00;
static const uint8_t REG_POWER_CTL = 0x2D;
static const uint8_t REG_DATAX0 = 0x32;

// Constants to be used in the program for the accelerometer
static const uint8_t DEVID = 0xE5;
static const float SENSITIVITY_2G = 1.0 / 256;  // (g/LSB)
static const float EARTH_GRAVITY = 9.80665;     // Earth's gravity in [m/s^2]

// Pins Used on the Pi Pico
const uint SDA_PIN_ACC = 4; // I2C SDA Pin for the accelerometer
const uint SCL_PIN_ACC = 5; // I2C SCL Pin for the accelerometer
i2c_inst_t *i2c_ACC = i2c0; // I2C bus for the accelerometer
const uint LED_PIN = 25;    // LED Pin for the Pi Pico

const uint SDA_PIN_ZERO = 18;   // I2C SDA Pin for the Zero
const uint SCL_PIN_ZERO = 19;   // I2C SCL Pin for the Zero
i2c_inst_t *i2c_ZERO = i2c1;    // I2C bus for the Zero
const uint WARNING_PIN = 3;     // Warning Pin for the Zero
const uint ACK_PIN = 2;         // Acknowledge Pin for the Zero
bool zero_setup = false;        // Flag to check if the Zero has been setup

const uint trigger_pin = 10;     // Trigger Pin for the Vibration Sensor


// ############################## [ Function Prototypes ] ##########################

/**
 * @brief Allows the user to write to a register of a device on the I2C bus
 * 
 * @param i2c Pointer to I2C bus to use on the pico, will be either i2c or i2c1
 * @param addr The address of the device to write to
 * @param reg The register to write to
 * @param buf Pointer to the data to write
 * @param nbytes The number of bytes to write
 * @return int check if the write was successful 0 if fail 1 if success
*/
int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

/**
 * @brief Allows the user to read from a register of a device on the I2C bus
 * 
 * @param i2c Pointer to I2C bus to use on the pico, will be either i2c or i2c1
 * @param addr The address of the device to read from
 * @param reg The register to read from
 * @param buf Pointer to the buffer to store the data in
 * @param nbytes The number of bytes to read
 * @return int the number of bytes read or 0 if it failed 1 if success
 */
int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

/**
 * @brief Sets up the accelerometer to be used and starts taking measurements
 * 
 * @param i2c The I2C bus to use
 * @param sda_pin The SDA pin to use
 * @param scl_pin The SCL pin to use
 * @param ADXL343_ADDR The address of the accelerometer
 * @return int 1 if successful blocked in a while loop if failed
*/
int accelerometer_setup(i2c_inst_t *i2c, const uint sda_pin, const uint scl_pin, const uint8_t ADXL343_ADDR);

/**
 * @brief Reads the accelerometer and returns if there is a landslide risk
 * 
 * @param i2c The I2C bus to use
 * @param ADXL343_ADDR The address of the accelerometer
 * @return int 1 if there is a landslide risk 0 if there is not
*/
int accelerometer_read(i2c_inst_t *i2c, const uint8_t ADXL343_ADDR);

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

    // Initialize accelerometer
    accelerometer_setup(i2c_ACC, SDA_PIN_ACC, SCL_PIN_ACC, ADXL343_ADDR);

    // Sets up the pico to be able to go into deep sleep.
    sleep_run_from_xosc();

    // Take measurements from the accelerometer and issue warnings as necessary
    while (1) 
    {   

        // Print message saying that the Pi Pico is going to sleep
        printf("Going to sleep until vibration is detected");
        uart_default_tx_wait_blocking();
        
        // Go to deep sleep until high signal is received on the trigger pin
        sleep_goto_dormant_until_level_high(trigger_pin);

        // Print message saying that the Pi Pico is awake
        printf("Vibration detected, checking for landslide risk");
        uart_default_tx_wait_blocking();

        // Takes 200 measurements from the accelerometer and issues a warning if necessary
        for (int i = 0; i < 200; i++)
        {
            if (accelerometer_read(i2c_ACC, ADXL343_ADDR) == 1)
            {
                // Issue warning to the Zero
                issue_warning(WARNING_PIN, ACK_PIN);

                // Break out of the loop
                break;
            }
        }
        
    }
    
}



int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf,const uint8_t nbytes) 
{
    
    int return_val = 0;

    // Create a buffer to hold the data to be written
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1) 
    {
        // Send back a fail
        return 0;
    }

    // Append register address to front of data packet
    msg[0] = reg;

    // Add the data to the data packet
    for (int i = 0; i < nbytes; i++) 
    {
        msg[i + 1] = buf[i];
    }

    // Write data to register over I2C bus given
    return_val = i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    // Check for error from write
    if (return_val == PICO_ERROR_GENERIC) 
    {
        // Output a print error
        printf("Error writing to register %d, of device %d\r\n", reg, addr);
        uart_default_tx_wait_blocking();

        // Return a fail
        return 0;
    }

    return 1;
}



int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes) 
{

    // Variable to return the number of bytes read
    int num_bytes_read = 0;

    // Check to make sure caller is asking for 1 or more bytes
    if (nbytes < 1) 
    {
        return 0;
    }

    // Issue a write command to tell the device to send the data
    i2c_write_blocking(i2c, addr, &reg, 1, true);

    // Read the data from the device
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

    // Check for a read error
    if (num_bytes_read == PICO_ERROR_GENERIC) 
    {   
        // Output a print error
        printf("Error reading from register %d, of device %d\r\n", reg, addr);
        uart_default_tx_wait_blocking();

        // Return a fail
        return 0;
    }

    // Return the number of bytes read
    return num_bytes_read;
}



int accelerometer_setup(i2c_inst_t *i2c, const uint sda_pin, const uint scl_pin, const uint8_t ADXL343_ADDR)
{
    // Buffer to store raw reads
    uint8_t data[6];

    // Initialize I2C at 400kHz
    i2c_init(i2c, 400 * 1000);

    // Set GPIO pins to I2C mode
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Read device ID to make sure that we can communicate with the ADXL343
    reg_read(i2c, ADXL343_ADDR, REG_DEVID, data, 1);
    if (data[0] != DEVID) 
    {
        printf("ERROR: Could not communicate with ADXL343\r\n");
        uart_default_tx_wait_blocking();

        while (true)
        {
            // Set LED to flash rapidly to indicate error
            gpio_put(LED_PIN, 1);
            sleep_ms(100);
            gpio_put(LED_PIN, 0);
            sleep_ms(100);
        }
    }

    // Tell ADXL343 to start taking measurements by setting Measure bit to high
    data[0] |= (1 << 3);
    reg_write(i2c, ADXL343_ADDR, REG_POWER_CTL, &data[0], 1);

    return 1;
}



int accelerometer_read(i2c_inst_t *i2c, const uint8_t ADXL343_ADDR)
{
    // Buffer to store raw reads
    uint8_t data[6];

    // Variables to store raw accelerometer readings
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
    float acc_x_f;
    float acc_y_f;
    float acc_z_f;

    // Read raw accelerometer data
    reg_read(i2c, ADXL343_ADDR, REG_DATAX0, data, 6);

    // Convert raw data to signed 16-bit integers
    acc_x = (data[1] << 8) | data[0];
    acc_y = (data[3] << 8) | data[2];
    acc_z = (data[5] << 8) | data[4];

    // Convert raw data to g's
    acc_x_f = acc_x * SENSITIVITY_2G;
    acc_y_f = acc_y * SENSITIVITY_2G;
    acc_z_f = acc_z * SENSITIVITY_2G;

    // Calculate the magnitude of the acceleration vector
    float acc_mag = sqrt(acc_x_f * acc_x_f + acc_y_f * acc_y_f + acc_z_f * acc_z_f);

    // Print the magnitude of the acceleration vector
    printf("Acceleration: %f g\r\n", acc_mag);
    uart_default_tx_wait_blocking();

    // if acceleration is above 2g
    if (acc_mag > 2.0) 
    {
        return 1;
    } 

    return 0;

}



int issue_warning(uint WARNING_PIN, uint ACK_PIN)
{

    // Check if Zero has been setup
    if (zero_setup == false)
    {
        // Setup the warning pin as an output
        gpio_init(WARNING_PIN);
        gpio_set_dir(WARNING_PIN, GPIO_OUT);

        // Setup the ack pin as an input
        gpio_init(ACK_PIN);
        gpio_set_dir(ACK_PIN, GPIO_IN);

        // Set zero_setup to true
        zero_setup = true;
    }

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
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <math.h>

// I2C address
static const uint8_t ADXL343_ADDR = 0x53;

// Registers
static const uint8_t REG_DEVID = 0x00;
static const uint8_t REG_POWER_CTL = 0x2D;
static const uint8_t REG_DATAX0 = 0x32;

// Other constants
static const uint8_t DEVID = 0xE5;
static const float SENSITIVITY_2G = 1.0 / 256;  // (g/LSB)
static const float EARTH_GRAVITY = 9.80665;     // Earth's gravity in [m/s^2]

// Pins
const uint sda_pin = 4;
const uint scl_pin = 5;
// Pi Pico LED pin
const uint LED_PIN = 25;

/*******************************************************************************
 * Function Declarations
 */
int reg_write(i2c_inst_t *i2c, 
                const uint addr, 
                const uint8_t reg, 
                uint8_t *buf,
                const uint8_t nbytes);

int reg_read(   i2c_inst_t *i2c,
                const uint addr,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes);

/*******************************************************************************
 * Function Definitions
 */

// Write 1 byte to the specified register
int reg_write(  i2c_inst_t *i2c, 
                const uint addr, 
                const uint8_t reg, 
                uint8_t *buf,
                const uint8_t nbytes) {

    int num_bytes_read = 0;
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1) {
        return 0;
    }

    // Append register address to front of data packet
    msg[0] = reg;
    for (int i = 0; i < nbytes; i++) {
        msg[i + 1] = buf[i];
    }

    // Write data to register(s) over I2C
    i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    return num_bytes_read;
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
int reg_read(  i2c_inst_t *i2c,
                const uint addr,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes) {

    int num_bytes_read = 0;

    // Check to make sure caller is asking for 1 or more bytes
    if (nbytes < 1) {
        return 0;
    }

    // Read data from register(s) over I2C
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

    return num_bytes_read;
}

void accelerometer_setup()
{
    // Buffer to store raw reads
    uint8_t data[6];

    // Initialize I2C at 400kHz
    i2c_init(i2c0, 400 * 1000);

    // Set GPIO pins to I2C mode
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Read device ID to make sure that we can communicate with the ADXL343
    reg_read(i2c0, ADXL343_ADDR, REG_DEVID, data, 1);
    if (data[0] != DEVID) {
        printf("ERROR: Could not communicate with ADXL343\r\n");
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
    reg_write(i2c0, ADXL343_ADDR, REG_POWER_CTL, &data[0], 1);
}

int accelerometer_read()
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
    reg_read(i2c0, ADXL343_ADDR, REG_DATAX0, data, 6);

    // Convert raw data to signed 16-bit integers
    acc_x = (data[1] << 8) | data[0];
    acc_y = (data[3] << 8) | data[2];
    acc_z = (data[5] << 8) | data[4];

    // Convert raw data to g's
    acc_x_f = acc_x * SENSITIVITY_2G;
    acc_y_f = acc_y * SENSITIVITY_2G;
    acc_z_f = acc_z * SENSITIVITY_2G;

    // Compensate for gravity on z-axis
    //acc_z_f -= EARTH_GRAVITY;

    // Calculate the magnitude of the acceleration vector
    float acc_mag = sqrt(acc_x_f * acc_x_f + acc_y_f * acc_y_f + acc_z_f * acc_z_f);

    // Print the magnitude of the acceleration vector
    printf("Acceleration: %f g\r\n", acc_mag);

    // if acceleration is above 2g
    if (acc_mag > 2.0) 
    {
        return 1;
    } else 
    {
        return 0;
    }

}

int main()
{

    

    // Initialize Pi Pico
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize accelerometer
    accelerometer_setup();

    // Take 20 readings
    for (int i = 0; i < 1000; i++) 
    {
        if (accelerometer_read() == 1)
        {
            // Setup GPIO pin 6 as an output
            gpio_init(6);
            gpio_set_dir(6, GPIO_OUT);

            // Set GPIO pin 6 to high
            gpio_put(6, 1);

            // Set LED to ON
            gpio_put(LED_PIN, 1);

            // Wait forever
            while (true)
            {
                sleep_ms(1000);
            }

        }
    }

    // Set GPIO pin 7 to output
    gpio_init(7);
    gpio_set_dir(7, GPIO_OUT);

    // Set GPIO pin 7 to high
    gpio_put(7, 1);
    sleep_ms(1000);

}
# A program that will take GPIO readings of GPIO16, 20 and 21 then 

import RPi.GPIO as GPIO
import time
import requests

RAIN_PIN = 16
SEISMIC_PIN = 20
SOIL_PIN = 21

RAIN_CLEAR_PIN = 8
SEISMIC_CLEAR_PIN = 7
SOIL_CLEAR_PIN = 1

SERVER = "http://192.168.3.50:5000/"

# Set up GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

# Set up GPIO pins with pull down resistors 
GPIO.setup(RAIN_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(SEISMIC_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(SOIL_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# Set up GPIO clear pins as outputs
GPIO.setup(RAIN_CLEAR_PIN, GPIO.OUT)
GPIO.setup(SEISMIC_CLEAR_PIN, GPIO.OUT)
GPIO.setup(SOIL_CLEAR_PIN, GPIO.OUT)


# Function that sends a get request to the server
def send_get_request(data):
    url = SERVER + data
    try:
        r = requests.get(url)
        print(r.text)
    except:
        print("Error sending get request")
    
# Take readings function
def take_readings():
    # Take readings
    Rain = GPIO.input(16)
    Seismic = GPIO.input(20)
    Soil = GPIO.input(21)
    
    if Rain == 1:
        send_get_request("rain")
        # Clear the rain sensor
        GPIO.output(RAIN_CLEAR_PIN, GPIO.HIGH)
        # Wait for the sensor to clear
        time.sleep(1)
        # Set the rain sensor back to low
        GPIO.output(RAIN_CLEAR_PIN, GPIO.LOW)
        return True
    
    if Seismic == 1:
        send_get_request("seismic")
        # Clear the seismic sensor
        GPIO.output(SEISMIC_CLEAR_PIN, GPIO.HIGH)
        # Wait for the sensor to clear
        time.sleep(1)
        # Set the seismic sensor back to low
        GPIO.output(SEISMIC_CLEAR_PIN, GPIO.LOW)
        return True
    
    if Soil == 1:
        send_get_request("soil")
        # Clear the soil sensor
        GPIO.output(SOIL_CLEAR_PIN, GPIO.HIGH)
        # Wait for the sensor to clear
        time.sleep(1)
        # Set the soil sensor back to low
        GPIO.output(SOIL_CLEAR_PIN, GPIO.LOW)
        return True

# Main function
def main():

    # Turn off Pi activity LED
    GPIO.setup(25, GPIO.OUT)
    GPIO.output(25, GPIO.HIGH)

    # Turn off Pi power LED
    GPIO.setup(24, GPIO.OUT)
    GPIO.output(24, GPIO.HIGH)

    # Take readings every 1 seconds
    while True:
        take_readings()
        time.sleep(1)

# Run main function
if __name__ == "__main__":
    main()
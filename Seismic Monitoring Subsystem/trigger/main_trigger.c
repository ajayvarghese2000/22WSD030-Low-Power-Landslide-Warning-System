// Flash the LED when every 1 second
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (true) {
        gpio_put(25, 1);
        sleep_ms(1000);
        gpio_put(25, 0);
        sleep_ms(1000);
    }
}
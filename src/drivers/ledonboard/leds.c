#include "leds.h"
#include "pico/cyw43_arch.h"

// Function to initialise GPIO to access LED
void led_init (void) {
  cyw43_arch_init();
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // [jo:230915] precisei colocar isso senão não começa a piscar
}

// Function to turn LED on
void led_on (void) {
cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);	
}

// Function to turn LED off
void led_off (void) {
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);	
}

// Function to invert current state of LED
void led_invert (void) {
  static bool led_value = 0;
  if (led_value == 0) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    led_value = 1;
  } else {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    led_value = 0;
  }
}


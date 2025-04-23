/**
 * @file main.cpp
 * @author Prof. Tute Ávalos (info@tute-avalos.com)
 * @brief Lectura de botones filtrando el rebote con técnica de ventana.
 * @version sin-objetos-v1
 * @date 2025-04-23
 *
 * Utilizando una técnica conocida de ventana, se leen los pulsadores haciendo un filtro pasa-bajos. En
 * este sistema se leen 3 pulsadores detectando los flancos descendentes `⁻⁻\_` (cuando la señal pasa de
 * 1 a 0). Esta técnica consta de desplazar una variable bit a bit, en cada lectura y reemplazando el
 * `bit0` de la misma y acumulando las lecturas descartando la más antigua (`bit7` en el caso de un
 * byte).
 *
 * @copyright Copyright (c) Matías S. Ávalos (@tute_avalos)
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 */
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// Variable que acumula los milisegundos transcurridos
volatile uint32_t millis{};

// cada 4ms se hace una lectura del botón:
const uint32_t BTN_TICKS{4};

int main() {
  // clock externo (HSE) de 8Mhz -> PLL -> 72Mhz
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  // Para poder usar los PB3 y PB4 (asociados al JTAG)
  // Periférico: Alternative Function I/O
  rcc_periph_clock_enable(RCC_AFIO);
  // Se inactiva el JTAG pero se mantiene el SW (para poder grabar y debuggear)
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

  // Periférico donde van a estar leds y pulsadores (en un mismo puerto para simplificar)
  rcc_periph_clock_enable(RCC_GPIOB);
  // LEDs en PB4 (led1) y PB5 (led2)
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO4 | GPIO5);
  gpio_set(GPIOB, GPIO4 | GPIO5);
  // Pulsadores PB12, PB13 y PB14
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO12 | GPIO13 | GPIO14);

  /* Se configura el timer del ARM Cortex-M */
  systick_set_frequency(1000, rcc_ahb_frequency); // Configuración del systick c/1ms
  systick_counter_enable();                       // se habilita la cuenta
  systick_interrupt_enable();                     // se habilita la interrupción

  uint8_t ventana_btn1{0xFF}, ventana_btn2{0xFF}, ventana_btn3{0xFF}; // ventanas de lectura
  uint8_t estado_btn1{}, estado_anterior_btn1{};
  uint8_t estado_btn2{}, estado_anterior_btn2{};
  uint8_t estado_btn3{}, estado_anterior_btn3{};
  uint32_t ticks_btns{BTN_TICKS};
  while (true) {
    if (millis >= ticks_btns) {
      // Se actualizan los ticks para la próxima lectura
      ticks_btns += BTN_TICKS;

      /******************** Lógica para la lectura del btn1 ********************/
      // desplazamos 1 bit la ventana del btn1 (corremos la ventana)
      ventana_btn1 <<= 1;
      // leemos el estado del btn1 y guardamos en el bit 0 de la ventana:
      ventana_btn1 |= (gpio_get(GPIOB, GPIO12) == GPIO12);

      if (ventana_btn1 == 0x00) { // si en la ventana hay ocho ceros (0b0000'0000)
        // hay un cero en el pulsador
        estado_btn1 = 0;
      } else if (ventana_btn1 == 0xFF) { // si en la ventana hay ocho unos (0b1111'1111)
        // hay un uno en el pulsador
        estado_btn1 = 1;
      }
      // si ahora hay un 0 y antes había un 1 (flanco descendente)
      if (estado_btn1 < estado_anterior_btn1) {
        // se presionó el botón
        gpio_toggle(GPIOB, GPIO4); // alternamos el "led1"
      }
      estado_anterior_btn1 = estado_btn1;
      /******************** Lógica para la lectura del btn2 ********************/
      // desplazamos 1 bit la ventana del btn2 (corremos la ventana)
      ventana_btn2 <<= 1;
      // leemos el estado del btn2 y guardamos en el bit 0 de la ventana:
      ventana_btn2 |= (gpio_get(GPIOB, GPIO13) == GPIO13);
      if (ventana_btn2 == 0x00) { // si en la ventana hay ocho ceros (0b0000'0000)
        // hay un cero en el pulsador
        estado_btn2 = 0;
      } else if (ventana_btn2 == 0xFF) { // si en la ventana hay ocho unos (0b1111'1111)
        // hay un uno en el pulsador
        estado_btn2 = 1;
      }
      // si ahora hay un 0 y antes había un 1 (flanco descendente)
      if (estado_btn2 < estado_anterior_btn2) {
        // se presionó el botón
        gpio_clear(GPIOB, GPIO5); // prendemos el "led2"
      }
      estado_anterior_btn2 = estado_btn2;
      /******************** Lógica para la lectura del btn3 ********************/
      // desplazamos 1 bit la ventana del btn3 (corremos la ventana)
      ventana_btn3 <<= 1;
      // leemos el estado del btn3 y guardamos en el bit 0 de la ventana:
      ventana_btn3 |= (gpio_get(GPIOB, GPIO14) == GPIO14);
      if (ventana_btn3 == 0x00) { // si en la ventana hay ocho ceros (0b0000'0000)
        // hay un cero en el pulsador
        estado_btn3 = 0;
      } else if (ventana_btn3 == 0xFF) { // si en la ventana hay ocho unos (0b1111'1111)
        // hay un uno en el pulsador
        estado_btn3 = 1;
      }
      // si ahora hay un 0 y antes había un 1 (flanco descendente)
      if (estado_btn3 < estado_anterior_btn3) {
        // se presionó el botón
        gpio_set(GPIOB, GPIO5); // apagamos el "led2"
      }
      estado_anterior_btn3 = estado_btn3;
    }
  }
}

/**
 * @brief Se incrementa la variable millis cada 1ms
 */
void sys_tick_handler() { millis++; }

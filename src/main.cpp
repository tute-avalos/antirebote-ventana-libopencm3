/**
 * @file main.cpp
 * @author Prof. Tute Ávalos (info@tute-avalos.com)
 * @brief Lectura de botones filtrando el rebote con técnica de ventana.
 * @version con-objetos-v3
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
#include "boton.hpp"
#include "led.hpp"

/**
 * @brief Inicialización del hardware (clock y cuestiones generales)
 */
void hw_init() {
  // clock externo (HSE) de 8Mhz -> PLL -> 72Mhz
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  // Para poder usar los PB3 y PB4 (asociados al JTAG)
  // Periférico: Alternative Function I/O
  rcc_periph_clock_enable(RCC_AFIO);
  // Se inactiva el JTAG pero se mantiene el SW (para poder grabar y debuggear)
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);
}

int main() {
  // Se inicializa el hardware
  hw_init();

  // Objetos instanciados (btn1, btn2, btn3) en un solo arreglo llamado botones:
  Boton botones[3]{{GPIOB, GPIO12}, {GPIOB, GPIO13}, {GPIOB, GPIO14}};

  // Objetos instanciados (led1 y led2)
  LED led1{GPIOB, GPIO4, 0}, led2{GPIOB, GPIO5, 0};

  while (true) {
    if (botones[0].huboFlancoDescendente()) {
      led1.alternar();
    }
    if (botones[1].estaActivo()) {
      led2.prender();
    } else if (botones[2].estaActivo()) {
      led2.apagar();
    }
  }
}

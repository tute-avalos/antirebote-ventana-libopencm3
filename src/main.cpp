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

/**
 * @brief Abstracción de un LED que puede prender, apagar o alternar.
 *
 */
class LED {
private:
  uint32_t _port;
  uint16_t _gpio;
  uint8_t _logica;
  bool _esta_prendido;

public:
  /**
   * @brief Constructor de un nuevo objeto LED
   *
   * @param port  puerto donde está el led (GPIOA, GPIOB, GPIOC)
   * @param gpio  Pin donde está el LED (GPIO0-GPIO15)
   * @param logica  Si el LED prende con (0 ó 1)
   * @param es_opendrain true: es salida Open Drain false: Salida Push/Pull
   */
  LED(uint32_t port, uint16_t gpio, uint8_t logica, bool es_opendrain = false)
      : _port(port), _gpio(gpio), _logica(logica), _esta_prendido(true) {
    rcc_periph_clock_enable((port == GPIOA) ? RCC_GPIOA : (port == GPIOB) ? RCC_GPIOB : RCC_GPIOC);
    gpio_set_mode(port, GPIO_MODE_OUTPUT_2_MHZ,
                  (es_opendrain) ? GPIO_CNF_OUTPUT_OPENDRAIN : GPIO_CNF_OUTPUT_PUSHPULL, gpio);
    apagar(); // LED inicialmente apagado.
  }
  /**
   * @brief Devuelve el estado actual del LED
   *
   * @return true El LED está prendido.
   * @return false El LED está apagado.
   */
  bool estaPrendido() const { return _esta_prendido; }
  /**
   * @brief Prende el LED (si está apagado)
   */
  void prender() {
    if (_esta_prendido) return;
    if (_logica)
      gpio_set(_port, _gpio);
    else
      gpio_clear(_port, _gpio);
    _esta_prendido = true;
  }
  /**
   * @brief Apaga el LED (si está prendido)
   */
  void apagar() {
    if (!_esta_prendido) return;
    if (_logica)
      gpio_clear(_port, _gpio);
    else
      gpio_set(_port, _gpio);
    _esta_prendido = false;
  }
  /**
   * @brief Alterna el estado del LED
   */
  void alternar() {
    _esta_prendido = !_esta_prendido;
    gpio_toggle(_port, _gpio);
  }
};

// Variable que acumula los milisegundos transcurridos
volatile uint32_t millis{};

// cada 4ms se hace una lectura del botón:
const uint32_t BTN_TICKS{4};

// Acciones a ejecutar según el botón presionado:
void accion_btn1() {
  gpio_toggle(GPIOB, GPIO4); // alternamos el "led1"
}

void accion_btn2() {
  gpio_clear(GPIOB, GPIO5); // prende el "led2"
}

void accion_btn3() {
  gpio_set(GPIOB, GPIO5); // apaga el "led2"
}

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
  gpio_set(GPIOB, GPIO4 | GPIO5);
  // Pulsadores PB12, PB13 y PB14
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO12 | GPIO13 | GPIO14);

  // Objetos instanciados (led1 y led2)
  LED led1{GPIOB, GPIO4, 0}, led2{GPIOB, GPIO5, 0};

  /* Se configura el timer del ARM Cortex-M */
  systick_set_frequency(1000, rcc_ahb_frequency); // Configuración del systick c/1ms
  systick_counter_enable();                       // se habilita la cuenta
  systick_interrupt_enable();                     // se habilita la interrupción

  uint16_t gpios[3]{GPIO12, GPIO13, GPIO14};
  uint8_t ventana_btns[3]{0xFF, 0xFF, 0xFF};
  uint8_t estado_btns[3]{};
  uint8_t estado_anterior_btns[3]{};
  uint32_t ticks_btns{BTN_TICKS};
  while (true) {
    if (millis >= ticks_btns) {
      // Se actualizan los ticks para la próxima lectura
      ticks_btns += BTN_TICKS;

      /******************** Lógica para la lectura de los botones ********************/
      for (int i{0}; i < 3; ++i) {
        // desplazamos 1 bit la ventana del btn[i] (corremos la ventana)
        ventana_btns[i] <<= 1;
        // leemos el estado del btn1 y guardamos en el bit 0 de la ventana:
        ventana_btns[i] |= (gpio_get(GPIOB, gpios[i]) == gpios[i]);

        if (ventana_btns[i] == 0x00) { // si en la ventana hay ocho ceros (0b0000'0000)
          // hay un cero en el pulsador
          estado_btns[i] = 0;
        } else if (ventana_btns[i] == 0xFF) { // si en la ventana hay ocho unos (0b1111'1111)
          // hay un uno en el pulsador
          estado_btns[i] = 1;
        }
        // si ahora hay un 0 y antes había un 1 (flanco descendente)
        if (estado_btns[i] < estado_anterior_btns[i]) {
          // se presionó el botón, se ejecuta la función correspondiente
          switch (i) {
          case 0: led1.alternar(); break;
          case 1: led2.prender(); break;
          case 2: led2.apagar(); break;
          }
        }
        estado_anterior_btns[i] = estado_btns[i];
      }
    }
  }
}

/**
 * @brief Se incrementa la variable millis cada 1ms
 */
void sys_tick_handler() { millis++; }

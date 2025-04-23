/**
 * @file main.cpp
 * @author Prof. Tute Ávalos (info@tute-avalos.com)
 * @brief Lectura de botones filtrando el rebote con técnica de ventana.
 * @version con-objetos-v2
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
#include "led.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

// Abstracción para mejorar la legibilidad
enum logic_t { ACTIV_BAJO = 0, ACTIV_ALTO = 1 };

/**
 * @brief Abstracción que modela un PushButton o botón con rebote
 */
class Boton {
private:
  uint32_t _port;
  uint16_t _gpio;
  uint8_t _estado;
  uint8_t _estado_anterior;
  logic_t _logica; // Activo en ALTO o BAJO
  uint8_t _ventana;
  bool _flanco_ascendente;
  bool _flanco_descendente;

public:
  /**
   * @brief Constructor para un nuevo objeto Boton
   *
   * @param port puerto donde está el botón (GPIOA, GPIOB, GPIOC)
   * @param gpio pin del puerto donde está el botón (GPIO0-GPIO15)
   * @param logica lógica con la que se activa el botón (ACTIV_BAJO o ACTIV_ALTO)
   * @param es_pull_up_down true habilita los pull-up/pull-down internos del micro;
   * false: utiliza pull-up/pull-down externos
   */
  Boton(uint32_t port, uint16_t gpio, logic_t logica = ACTIV_BAJO, bool es_pull_up_down = false);

  /**
   * @brief Devuelve el estado actual del pulsador
   *
   * @return true el pulsador está activo
   * @return false el pulsador no está activo
   */
  bool estaActivo() const { return _estado == _logica; }
  /**
   * @brief Hubo flanco ascendente
   *
   * Indica si hubo un flanco ascendente, funciona como un flag, una vez leído se resetea el valor
   *
   * @return true hubo un flanco (pudo haber sido más de uno si no se estuvo leyendo el valor)
   * @return false no hubo un flanco
   */
  bool huboFlancoAscendente();
  /**
   * @brief Hubo flanco descendente
   *
   * Indica si hubo un flanco descendente, funciona como un flag, una vez leído se resetea el valor
   *
   * @return true hubo un flanco (pudo haber sido más de uno si no se estuvo leyendo el valor)
   * @return false no hubo un flanco
   */
  bool huboFlancoDescendente();
  /**
   * @brief Se actualiza el valor del pin a sensar, debe ser llamado peridicamente
   *
   */
  void actualizar();
};

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

  // Objetos instanciados (btn1, btn2, btn3) en un solo arreglo llamado botones:
  Boton botones[3]{{GPIOB, GPIO12}, {GPIOB, GPIO13}, {GPIOB, GPIO14}};

  // Objetos instanciados (led1 y led2)
  LED led1{GPIOB, GPIO4, 0}, led2{GPIOB, GPIO5, 0};

  /* Se configura el timer del ARM Cortex-M */
  systick_set_frequency(1000, rcc_ahb_frequency); // Configuración del systick c/1ms
  systick_counter_enable();                       // se habilita la cuenta
  systick_interrupt_enable();                     // se habilita la interrupción

  uint32_t ticks_btns{BTN_TICKS};
  while (true) {
    if (millis >= ticks_btns) {
      // Se actualizan los ticks para la próxima lectura
      ticks_btns += BTN_TICKS;

      /************* Actualización del estado de los pines **************/
      for (auto &btn : botones) {
        btn.actualizar();
      }

      /************** Acciones a realizar con los botones ***************/
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
}

/**
 * @brief Se incrementa la variable millis cada 1ms
 */
void sys_tick_handler() { millis++; }

Boton::Boton(uint32_t port, uint16_t gpio, logic_t logica, bool es_pull_up_down)
    : _port(port), _gpio(gpio), _logica(logica), _ventana((_logica) ? 0x00 : 0xFF) {
  rcc_periph_clock_enable((port == GPIOA) ? RCC_GPIOA : (port == GPIOB) ? RCC_GPIOB : RCC_GPIOC);
  gpio_set_mode(
      port, GPIO_MODE_INPUT, es_pull_up_down ? GPIO_CNF_INPUT_PULL_UPDOWN : GPIO_CNF_INPUT_FLOAT, gpio);
  // Se habilita el pull-up/down
  if (es_pull_up_down) {
    if (logica) {             // ACTIV_ALTO:
      gpio_clear(port, gpio); // Pull-Down
    } else {                  // ACTIV_BAJO:
      gpio_set(port, gpio);   // Pull-Up
    }
  }
  this->_estado_anterior = this->_estado = !logica; // Inicialmente se considera que no está activo
  this->_flanco_ascendente = this->_flanco_descendente = false; // inicialmente no hay flancos
}

void Boton::actualizar() {
  _ventana <<= 1;
  _ventana |= (gpio_get(_port, _gpio) == _gpio);
  if (_ventana == 0xFF) {
    _estado = 1;
  } else if (_ventana == 0x00) {
    _estado = 0;
  }
  if (_estado_anterior < _estado) _flanco_ascendente = true;
  if (_estado_anterior > _estado) _flanco_descendente = true;
  _estado_anterior = _estado;
}

bool Boton::huboFlancoAscendente() {
  if (_flanco_ascendente) {
    _flanco_ascendente = false;
    return true;
  }
  return false;
}

bool Boton::huboFlancoDescendente() {
  if (_flanco_descendente) {
    _flanco_descendente = false;
    return true;
  }
  return false;
}

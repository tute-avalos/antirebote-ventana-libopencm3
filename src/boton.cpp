// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "boton.hpp"
#include "manejador_tareas.hpp"

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
  // Llamar a actualizar() cada 4ms:
  ManejadorTareasPeriodicas::obtenerInstancia().registrarTarea(this, 4);
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

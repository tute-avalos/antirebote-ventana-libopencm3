// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "led.hpp"

LED::LED(uint32_t port, uint16_t gpio, uint8_t logica, bool es_opendrain)
    : _port(port), _gpio(gpio), _logica(logica), _esta_prendido(true) {
  rcc_periph_clock_enable((port == GPIOA) ? RCC_GPIOA : (port == GPIOB) ? RCC_GPIOB : RCC_GPIOC);
  gpio_set_mode(port, GPIO_MODE_OUTPUT_2_MHZ,
                (es_opendrain) ? GPIO_CNF_OUTPUT_OPENDRAIN : GPIO_CNF_OUTPUT_PUSHPULL, gpio);
  apagar(); // LED inicialmente apagado.
}

void LED::prender() {
  if (_esta_prendido) return;
  if (_logica)
    gpio_set(_port, _gpio);
  else
    gpio_clear(_port, _gpio);
  _esta_prendido = true;
}

void LED::apagar() {
  if (!_esta_prendido) return;
  if (_logica)
    gpio_clear(_port, _gpio);
  else
    gpio_set(_port, _gpio);
  _esta_prendido = false;
}

void LED::alternar() {
  _esta_prendido = !_esta_prendido;
  gpio_toggle(_port, _gpio);
}

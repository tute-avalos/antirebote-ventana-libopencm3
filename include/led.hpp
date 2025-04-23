// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef __LED_HPP__
#define __LED_HPP__

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/**
 * @brief Abstracción de un LED que puede prender, apagar o alternar.
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
  LED(uint32_t port, uint16_t gpio, uint8_t logica, bool es_opendrain = false);

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
  void prender();
  /**
   * @brief Apaga el LED (si está prendido)
   */
  void apagar();
  /**
   * @brief Alterna el estado del LED
   */
  void alternar();
};

#endif // __LED_HPP__

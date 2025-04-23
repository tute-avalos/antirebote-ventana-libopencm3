// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef __BOTON_HPP__
#define __BOTON_HPP__

#include "actualizable.hpp"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// Abstracción para mejorar la legibilidad
enum logic_t { ACTIV_BAJO = 0, ACTIV_ALTO = 1 };

/**
 * @brief Abstracción que modela un PushButton o botón con rebote
 */
class Boton : Actualizable {
private:
  uint32_t _port;
  uint16_t _gpio;
  uint8_t _estado;
  uint8_t _estado_anterior;
  logic_t _logica; // Activo en ALTO o BAJO
  uint8_t _ventana;
  bool _flanco_ascendente;
  bool _flanco_descendente;

protected:
  /**
   * @brief Se actualiza el valor del pin a sensar, debe ser llamado peridicamente
   */
  void actualizar() override;

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
};

#endif // __BOTON_HPP__

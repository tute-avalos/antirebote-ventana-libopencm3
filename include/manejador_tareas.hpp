// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef __MANEJADOR_TAREAS_HPP__
#define __MANEJADOR_TAREAS_HPP__

#include "actualizable.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

#ifndef MAX_TASKS
#define MAX_TASKS 8
#endif

class ManejadorTareasPeriodicas {
private:
  Actualizable *_tareas[MAX_TASKS];
  uint32_t _ticks[MAX_TASKS];
  uint32_t _delta_ticks[MAX_TASKS];
  uint8_t _cont_tareas;
  /**
   * @brief constructor del objeto Manejador Tareas Periodicas
   * 
   * Este es privado ya que es un singleton.
   */
  ManejadorTareasPeriodicas();

public:
  /**
   * @brief Crea y/o devuelve la instancia del objeto ManejadorTareasPeriodicas
   * 
   * Inicializa el systick y la interrupción cada 1ms y llama a tick()
   * 
   * @return ManejadorTareasPeriodicas& 
   */
  static ManejadorTareasPeriodicas &obtenerInstancia();
  /**
   * @brief Añade una tarea a la lista de tareas periódicas
   * 
   * @param tarea alguna clase que implemente la interfaz actualizable
   * @param tick cada cuanto se repite la tarea periódica
   * @return true si se pudo agregar la tarea a la lista
   * @return false si no se pudo agregar la tarea a la lista
   */
  bool registrarTarea(Actualizable *tarea, uint32_t tick);
  /**
   * @brief Actualiza las tareas según el tick de cada una
   */
  void tick();
};

#endif // __MANEJADOR_TAREAS_HPP__
// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "manejador_tareas.hpp"

ManejadorTareasPeriodicas::ManejadorTareasPeriodicas() : _cont_tareas(0) {
  for (auto &t : _tareas) {
    t = nullptr;
  }
  systick_set_frequency(1000, rcc_ahb_frequency); // cada 1ms
  systick_interrupt_enable();
  systick_counter_enable();
}

ManejadorTareasPeriodicas &ManejadorTareasPeriodicas::obtenerInstancia() {
  static ManejadorTareasPeriodicas instancia{};
  return instancia;
}

bool ManejadorTareasPeriodicas::registrarTarea(Actualizable *tarea, uint32_t tick) {
  if (_cont_tareas >= MAX_TASKS) return false;
  _tareas[_cont_tareas] = tarea;
  _delta_ticks[_cont_tareas] = tick;
  _ticks[_cont_tareas++] = tick;
  return true;
}

void ManejadorTareasPeriodicas::tick() {
  static uint32_t millis{};
  for (int i{0}; i < _cont_tareas; ++i) {
    if ((millis - _ticks[i]) == 0) {
      _ticks[i] += _delta_ticks[i];
      if (_tareas[i]) _tareas[i]->actualizar();
    }
  }
  millis++;
}

void sys_tick_handler() { ManejadorTareasPeriodicas::obtenerInstancia().tick(); }

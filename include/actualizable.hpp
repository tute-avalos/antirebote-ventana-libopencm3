// Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef __ACTUALIZABLE_HPP__
#define __ACTUALIZABLE_HPP__

class Actualizable {
  friend class ManejadorTareasPeriodicas;

protected:
  virtual void actualizar() = 0;
};

#endif // __ACTUALIZABLE_HPP__

<!--
 Copyright (c) 2025 Matías S. Ávalos (@tute_avalos)
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->

# Programación de debounce por software

Utilizando una técnica conocida de ventana, se leen los pulsadores haciendo un filtro pasa-bajos. En este sistema se leen 3 pulsadores detectando los flancos descendentes `⁻⁻\_` (cuando la señal pasa de 1 a 0). Esta técnica consta de desplazar una variable bit a bit, en cada lectura y reemplazando el `bit0` de la misma y acumulando las lecturas descartando la más antigua (`bit7` en el caso de un byte).

En la siguiente imagen se ilustra el desplazamiento de la ventana que descarta la lectura más vieja (`bit7`) y acumula la nueva lectura en el `bit0`.

![Representación del desplazamiento de la ventana en el timepo.](./img/debounce_con_senal.gif)

Con esta técnica, daremos por válido el estado del pin cuando bien todos los bits estén en 1 (estado ALTO) o bien todos los bits estén en 0 (estado BAJO). Esto hará que evitemos el rebote del pulsador, ya que al leer sucesivamente cada una X cantidad de ms, cuando haya inestabilidades, esto no se tomará como un valor válido. El funcionamiento general en código es el siguiente:

```cpp
// ventana y estado_btn son `uint8_t` (byte)
// cara X ms:
ventana_btn <<= 1; // Se desplazan 1 bit a la izquierda
// leemos el estado del pin y guardamos en el bit 0 de la ventana:
ventana_btn |= leer_pin(pin_btn); // se guarda un 0 o 1 (según el estado del pin en el momento de la lectura)

if (ventana_btn == 0x00) { // si en la ventana hay ocho ceros (0b0000'0000)
  // hay un cero en el botón
  estado_btn = 0;
} else if (ventana_btn == 0xFF) { // si en la ventana hay ocho unos (0b1111'1111)
  // hay un uno en el botón
  estado_btn = 1;
}
```

## Funcionamiento

Se leen 3 botones ubicados en PB12 (btn1), PB13 (btn2) y PB14 (btn3). El btn1 alterna el PB4 (led1), cada vez que es presionado se alterna entre prendido y apagado. El btn2 pone en estado BAJO el PB5, prendiendo el led2, y el btn3 lo apaga poniendo en estado alto el mismo pin.

## Versión con objetos-v2

En esta iteración se agrega una nueva clase `Boton` y ahora se ha agregado un archivo `include/led.hpp` donde está la declaración de la clase `LED` y la implementación (lo que hacen los métodos) en `src/led.cpp`. En el main se incluye el archivo `"led.hpp"` para poder acceder al mismo. Para ver más rápidamente la inclusión de la clase `Boton` esta se dejó en el `main.cpp`, pero si se dividió su declaración de su implementación.

Para poder implementar algo de lo declarado en una clase por fuera de ella, debemos recurrir al operador de *ambito* `::`. Es decir, debemos especificar que estamos tratando con una función miembro de una clase y no una función cualquiera. Esto se hace siguiente el formato:

```cpp
// <tipo_dato_retorno> <clase>::<método>() {
//   ... <-- implementación
// }
// por ejemplo:
bool Boton::estaActivo() { 
  // ...
}
```

---
*Recordamos que...*\
Los objetos son abstracciones de la realidad que se plasman en código. Las **clases** son *modelos* o *plantillas* que indican qué propiedades tienen los mismos y las *funciones miembro* o *métodos* hacen al comportamiento del objeto (es decir, lo que sabe o puede hacer). Por lo tanto una clase define las 3 cuestiones básicas del modelo: Nombre, Estado (atributos) y Comportamiento (métodos).

![Diagrama de clase de LED](./img/diagrama-clases.png)


El objeto (o la instancia) son las variables, en este caso, los botones están en una variable del tipo *arreglo de Boton de 3 elementos*.

```cpp
// Arreglo de 3 *Botones* todos activos en bajo (por default)
Boton botones[3]{{GPIOB, GPIO12}, {GPIOB, GPIO13}, {GPIOB, GPIO14}};
```

Los objetos tienen identidad própia (nombre de la variable) y sus propios estados (definidos por el valor en sus atributos), posee un comportamiento que se activa al enviar alguno de los mensajes expuestos en su *API pública*, en este caso los mensajes son `estaActivo()`, `huboFlancoAscendente()`, `huboFlancoDescendente()` y `actualizar()`.

---

## Sobre la detección del flanco

Para detectar el flanco, se utiliza una variable auxilar que almacena el estado **establecido** (descartando el rebote) anterior, y lo compara con el recientemente leído (`estado_btnX`). Siguiendo la siguiente lógica:

| `estado_anterior_btnX` | `estado_btnX` |               | Acción                    |
| ---------------------- | ------------- | ------------- | ------------------------- |
| 0                      | 0             | -             | sin cambios               |
| 0                      | 1             | *(ant < act)* | flanco ascendente `_/⁻⁻`  |
| 1                      | 0             | *(ant > act)* | flanco descendente `⁻⁻\_` |
| 1                      | 1             | -             | sin cambios               |

# Resolución de ejercicio F6 de Guia de ejercitación de RTOS de CESE (2021)
Autor: Fernando Prokopiuk <fernandoprokopiuk@gmail.com>

# F.6 Pasaje de estructuras 2
Implementar tres tareas.

* Tarea 1: Medirá el tiempo de pulsación de un botón, aplicando anti-rebote. Envía el evento de pulsación incluyendo el dato del tiempo medido por la cola 1 y cola 2.

* Tarea 2: Periódicamente destellará LED1 o LED2. Si por cola1 recibe el evento de pulsación, entonces el proximo led a destellar sera el 1. En caso de no recibir el evento destellará LED2. En cualquier caso, enviará un evento de "blink" incluyendo el dato de que led destelló, a la cola2.

* Tarea 3 (ÚNICA!): Esperará evento por cola2 ("cola de impresión") e imprimirá un mensaje apropiado con todos sus elementos.
    * Evento de tecla pulsada "Se pulso tecla X, durante Y ms"
    * Evento de blink "Se encendió led X"





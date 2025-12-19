[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/y54ywNyp)
# Template for render-2025

This repository contains a template for the project assignment in the Computer
Architecture course at Universidad Carlos III de Madrid.

---- Comentarios importantes para la ejecución del programa!! ----
--Compilación para la ejecución de PAR--
cmake --build out/build/default --target render-par
(si no está la carpeta default, se crea: cmake --preset=default)
--Ejemplo de ejecución del programa--
./out/build/default/par/Release/render-par archivos_ejemplo/config1.cfg archivos_ejemplo/scene1.txt output_soa_ejemplo.ppm

Si no se añaden argumentos extra, el programa ejecuta la versión óptima
--Argumentos extra--
.-arg4: número de hilos
-.arg5: estrategia de partición (simple, auto ó static)
-.arg6: grano de filas
-.arg7: grano de columnas

--Script utest.py--

Configura, compila y ejecuta automáticamente todas las pruebas unitarias del proyecto, tanto comunes como específicas de AOS y SOA. Para ejecutar: python3 utest.py
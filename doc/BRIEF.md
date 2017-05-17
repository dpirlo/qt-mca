## Aplicación para el mantenimiento del AR-PET

**qt-mca** es una aplicación realizada en Qt C++ la cual permite la administración del primer tomógrafo por emisión de positrones argentino (AR-PET*).

### Versión: $Version

* Encendido y apagado del equipo
* Inicialización de las seis cámaras gamma (*cabezales*),
* Gráficos del espectro en cuentas (MCA),
* Información de la temperatura de cada fotomultiplicador (PMT)
* Configuración del valor de alta tensión (HV) en cada dinodo de los PMT
* Administración de la alta tensión de los cabezales (PSOC)
* Configuración de las tablas de calibración de cada PMT
* Inicialización de las tablas de calibración en energía, posición y tiempo
* Autocalibración de los parámetros del equipo
* Reconstrucción de imágenes
* Registración (*log*) de datos: tasa y temperatura de los cabezales seleccionados
* Terminal de prueba (con configuración de la trama enviada)
* Debug en consola (*runtime*)

## Pestaña de configuración

En esta sección se realiza la selección del cabezal y su inicialización a partir de las tablas de calibración correspondientes.

<p align="center"><img src="../../img/img_conf.png" width="600"></p>

## Pestaña de Multicanal (MCA)

Se recibe las tramas MCA y se grafican sus resultados.

### Fotomultiplicador

Se adquiere la trama MCA a un número de PMTs seleccionados previamente.

<p align="center"><img src="../../img/img_pmt_selection.png" width="600"></p>
<p align="center"><img src="../../img/img_mca.png" width="600"></p>

### Cabezal

Trama MCA a partir del cabezal seleccionado.

<p align="center"><img src="../../img/img_mca_head.png" width="600"></p>

### Temperatura

Se obtienen los valores de temperatura de cada PMT. Por otro lado se muestran el valor mayor, menor y promedio de las temperaturas obtenidas. Se descartan valores de temperaturas menores a 20ºC (se lo considera como un valor erróneo).

<p align="center"><img src="../../img/img_temp.png" width="600"></p>

## Calibración

### AutoCalibración (en progreso)

Se realiza la autocalibración de los valores de HV de cada uno de los fotomultiplicadores del equipo y la alta tensión de los cabezales.

<p align="center"><img src="../../img/img_autocalib.png" width="600"></p>

### Calibración fina

A partir de realizar un campo inundado se realiza la calibración en energía y posición del equipo.

<p align="center"><img src="../../img/img_calib_fina.png" width="600"></p>

### Visualización

Herramienta para visualizar todo el análisis de resultados.

<p align="center"><img src="../../img/img_analizar_planar.png" width="600"></p>

## Reconstrucción

Herramienta para reconstruir la imagen a partir de una adquisición con una fuente radioactiva.

<p align="center"><img src="../../img/img_recon.png" width="600"></p>

## Log

Función de registración de los datos de temperatura y tasa de adquisición para los cabezales seleccionados.

<p align="center"><img src="../../img/img_log.png" width="600"></p>

## Preferencias

A partir de este menú se configuran las rutas de los archivos de calibración del equipo. Por otro lado permite
habilitar o deshabilitar las opciones de _log_ y _debug_.

<p align="center"><img src="../../img/img_preferencias.png" width="600"></p>

## Terminal de pruebas

Consola para pruebas de envío y recepción de tramas serie.

<p align="center"><img src="../../img/img_terminal.png" width="600"></p>

## Debug

En el momento que se habilita la opción _debug_ desde el menú preferencias, el programa comienza a mostrar todos los mensajes recibidos y envíados al equipo por consola.

<p align="center"><img src="../../img/img_debug.png" width="600"></p>

*AR-PET: Primer Tomógrafo por Emsión de Positrones Argentino, C. Verrastro, D. Estryk, E. Venialgo, S. Marinsek, M. Belzunce, XXXV Reunión Anual de la Asociación Argentina de Tecnología Nuclear, Noviembre 2008.

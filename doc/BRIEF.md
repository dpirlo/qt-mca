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

## Terminal de pruebas

Consola para pruebas de envío y recepción de tramas serie.

<p align="center"><img src="../../img/img_terminal.png" width="600"></p>

## Debug

En el momento que se habilita la opción _debug_ desde el menú preferencias, el programa comienza a mostrar todos los mensajes recibidos y envíados al equipo por consola.

<p align="center"><img src="../../img/img_debug.png" width="600"></p>

*AR-PET: Primer Tomógrafo por Emsión de Positrones Argentino, C. Verrastro, D. Estryk, E. Venialgo, S. Marinsek, M. Belzunce, XXXV Reunión Anual de la Asociación Argentina de Tecnología Nuclear, Noviembre 2008.

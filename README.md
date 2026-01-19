Water Pump Controller
Sistema de control automático para bombas de agua en instalaciones residenciales con cisterna y tanque elevado. Diseñado para optimizar el consumo energético y prevenir funcionamiento en seco de la bomba. La version mas actual es "MEF_IF_EEPROM".

![photo_2026-01-19 16 11 02](https://github.com/user-attachments/assets/e71ccc1b-bbbf-4ea1-8a62-451e9861eb20)

Sistema en producción implementado desde 2023

Estado: En Producción
Operando continuamente desde 2023 sin fallas en instalación residencial real.
REV. B: corregido invertidos pines Rx y Tx, resistencia pulldown de Q1. Agregado conector J10 para programar bootloader (no implementado todavia).

Problema que Resuelve
En sistemas tradicionales de cisterna + tanque elevado, las bombas funcionan con controles manuales, niveles flotantes o temporizadores simples, lo que genera:

Riesgo de funcionamiento en seco (daño a la bomba) en caso de que un sensor falle.
Desperdicio energético (bomba funcionando innecesariamente)
Falta de monitoreo del estado del sistema

Este controlador automatiza completamente el proceso con lógica inteligente. Brinda testigos luminicos para relevar el estado del tanque y de la cisterna (muy util tambien para verificar si los sensores estan funcionando correctamente en caso de aviso de falla).

Características
Hardware

Diseño de PCB custom con microcontrolador ATmega328.
Sensores de nivel de agua en cisterna y tanque
Relé para control de bomba (220V/15A)
Protección contra funcionamiento en seco
Indicadores LED de estado
Posibilidad de usar la placa como standalone sin microcontrolador integrado, sino conectandolo atraves de pines provistos en el etremo izquierdo. Facilidad para usar un ESP32 por ejemplo.

Firmware

Lógica de histéresis para evitar ciclos cortos
Protección multi-nivel:

Detección de cisterna vacía
Timeout de seguridad

Funcionamiento
Lógica básica:

Sensor detecta nivel bajo en tanque y nivel alto en cisterna → activa bomba
Sensor verifica nivel suficiente en cisterna
Bomba llena tanque hasta nivel alto
Sistema entra en standby hasta próximo ciclo

Proteccion contra funcionamiento en seco:
Por defecto, dado el caso para el que se diseñó este circuito, el tanque tiene una capacidad muy superior a la cisterna y la bomba es capaz de vaciar la cisterna en 2:15 minutos. Por lo cual se implementó un timeout que si a los 3 minutos el sensor de la cisterna no está en bajo, asume que es por falla del sensor, apaga la bomba y se queda en estado de ERROR con led rojo parpadeando

Funcion LEARN:
se puede programar que el sistema aprenda de forma automatica cuanto tiempo demora en vaciarse la cisterna. Para esto se debe tener el tanque completamente vacio y la cisterna completamente llena. Con la placa desenergizada, mantener apretado el pulsador "LEARN" y energizar la placa. Una vez que se haya encendido la bomba se puede soltar el pulsador. En este estado el microcontrolador contará el tiempo que demore en vaciarse la cisterna y lo guardará en memoria no volatil (por lo que se puede reiniciar o apagar sin problemas, el tiempo queda guardado). Asi podremos usar la proteccion contra funcionamiento en seco de forma adecuada sin necesidad de cronometrar y reprogramar el micrcocontrolador.

Especificaciones Técnicas

Microcontrolador      ATmega328
Sensores              Tipo: flotantes
Relé                  220V 15A
Alimentación          12V DC 1A
PCB                   2 capas. Dimensiones: 100mm x 80mm. 

Galería
PCB Diseñado

Instalación Real
TODO FOTO
![IMG_1010](https://github.com/user-attachments/assets/6b4fe1f5-71a2-4769-8080-581e868ae00e)


Requisitos de Hardware

Fuente de alimentación 12V 1A
Sensores de nivel tipo flotante de 3 cables.
Cableado eléctrico certificado para bomba.

Instalación

Montar sensores en cisterna y tanque. Ajustar niveles de la misma forma que en un sistema normal.
Conectar los 3 cables de cada sensor a su correspondiente borne en la PCB.
Verificar alimentando la PCB que las luces testigo esten correctas simulando sobre los sensores manualmente que esta "lleno" o "vacio".
Conectar bomba al relé del controlador.
Alimentar el sistema.
De ser necesario, realizar la rutina de entrenamiento LEARN para calcular y calibrar el tiempo de vaciado de la cisterna.

Resultados en Producción
Desde Marzo 2023:

-Cero fallas de hardware
-Cero activaciones de funcionamiento en seco

Desarrollo Futuro (Versión Comercial)
Mejoras planificadas para siguiente versión de producto:

 Conectividad WiFi para monitoreo remoto
 App móvil para visualización de estado
 Data logging de consumo y ciclos
 Múltiples perfiles de configuración
 Carcasa plástica para montaje industrial
 Certificación eléctrica

 Consideraciones de Seguridad
⚠️ ADVERTENCIA: Este sistema maneja voltaje de red (220V). 

La instalación debe ser realizada por electricista 
Cumplir normativa eléctrica local
Incluir protección térmica en línea de bomba

Licencia
GPL V3

✉️ Contacto
Rodrigo Maero - r.maero@gmail.com
Proyecto personal desarrollado en Rosario, Argentina.

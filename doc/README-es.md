<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">Multiplataforma / Bajo uso de memoria / Multi-protocolo / Multifunción / Sincronización en la nube</h2>
<h3 style="text-align: center;">
  [<a href="../README.md">English</a>] | 
  [<a href="README-zh_CN.md">简体中文</a>]
  [<a href="README-zh_TW.md">繁體中文</a>]
  [<a href="README-de.md">Deutsch</a>]
  [<a href="README-es.md">Español</a>]
  [<a href="README-fr.md">Français</a>]
  [<a href="README-hi.md">हिंदी</a>]
  [<a href="README-id.md">Bahasa Indonesia</a>]
  [<a href="README-ja.md">日本語</a>]
  [<a href="README-ko.md">한국어</a>]
  [<a href="README-ru.md">Русский</a>]
</h3>
  <h3 style="text-align: center;">[<a href="https://woterm.com">Sitio oficial WoTerm</a>]</a></h3>
</p>

# Resumen
Integrado con todos los principales protocolos de comunicación remota para satisfacer sus necesidades: soporta SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, puerto serie, TCP, UDP y más — no más cambios entre herramientas. Maneje fácilmente diversos escenarios de acceso remoto en una plataforma unificada.

## Descripción de funciones[<a href="https://en.woterm.com/versions/">&gt;&gt;&gt;Haga clic aquí para ver más funciones&lt;&lt;&lt;</a>]
- Soporta Windows / Linux / MacOSX / Android.  
- Soporta protocolos de comunicación principales: SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP, etc.  
- Varias pieles integradas para elegir, manteniendo escalabilidad para que los usuarios avanzados puedan personalizar libremente.  
- Modo administrador que permite ver contraseñas de sesión y prevenir robos.  
- Protocolo de sincronización en la nube basado en SFTP con 8 algoritmos de cifrado de alta seguridad.  
- Terminal SSH con asistente SFTP integrado, también soporta pestañas independientes SFTP.  
- Diseño de pestañas flotantes para colaboración entre varias ventanas.  
- Segmentación de ventanas ilimitada.  
- Agrupación de sesiones y filtros de búsqueda por palabras clave.  
- Decenas de esquemas de colores de terminal.

- Soporta escritorio remoto VNC, funciones avanzadas requieren [WoVNCServer](http://wovnc.com).  
  - Soporta protocolos estándar RFB 3.3/3.7/3.8.  
  - Soporta codificaciones estándar: ZRLE / TRLE / Hextile / CopyRect / RRE / Raw.  
  - Soporta codificaciones extendidas: H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2.  
  - Bloqueo automático de pantalla al desconectarse la red o finalizar la operación remota.  
  - Modo de pantalla negra remota.  
  - Reproducción de sonido del escritorio remoto.  
  - Cambio libre de calidad de imagen: sin pérdida, HD, normal, 16/15/8 bits.  
  - Soporta modo multimonitor y modos de estiramiento, pantalla compartida y pantalla dividida.  

- Soporte de terminal remoto.  
  - Acceso remoto a terminales Windows/Linux/MacOSX.  
  - Soporte Proxy jump.  
  - Gestión de certificados.  
  - Configuración de atajos de teclado.  
  - Subida y descarga de archivos ZModem.  
  - Interacción colaborativa SFTP integrada.  
  - Varios esquemas de colores para el terminal.  

## Descripción del módulo
El código cliente es completamente open source (excepto el módulo de control de versiones kxver), el código de terceros proviene de GitHub, CodeProject u otras comunidades abiertas. Algunos módulos provienen de [WoVNC](http://wovnc.com).  

## WoVNCServer
Se recomienda [WoVNCServer](http://wovnc.com) para funciones avanzadas.  

## Más información:
<a href="http://www.woterm.com">http://www.woterm.com</a>  

## Descarga de binarios:
<a href="http://woterm.com">http://woterm.com</a>  

## Rendimiento

Motor de análisis de terminal desarrollado internamente, analiza y muestra decenas de millones de registros en segundos manteniendo un uso de memoria extremadamente bajo. [Ver proceso detallado](Performance-es.md)

![](timeseq1.png)  
![](urandom_test_speed.png)  
![](urandom_test_memory.png)  

Búsqueda y localización ultrarrápida en millones de líneas de texto  
![](search.gif)

---

## Vista previa UI

**Interfaz principal**  
![](main.gif)  

**Gestión de claves SSH**  
![](keymgr2.gif)  

**Puerto serie**  
![](serialport.gif)  

**Túnel**  
![](tunnel.png)  

**Varias pieles**  
![](skins.png)  

**Playbook**  
![](playbook.gif)  

**Combinar y separar**  
![](merge.gif)  

**Sincronización en la nube**  
![](sync.gif)  

**Filtro por palabras clave**  
![](filter.gif)  

**Transferencia de archivos**  
![](sftp.gif)  

**Pestañas flotantes**  
![](float.gif)  

**División de ventanas**  
![](split.gif)  

**Tema de terminal**  
![](patten.gif)  

**Resaltado de sintaxis**  
![](highlight.gif)  

**Terminal VNC**  
![](vnc.gif)


# Declaración de Código Abierto
Desde la versión v10, hemos ajustado nuestra licencia de código abierto. En el futuro, ya sea que mantengamos el código abierto parcial o avancemos hacia un código completamente abierto, mantendremos siempre una actitud abierta y transparente.  

Cabe señalar que este repositorio no contiene bibliotecas de terceros ni el código fuente/archivos de biblioteca del **módulo kxver**.  
El **módulo kxver** se utiliza principalmente para el control de versiones, especialmente relacionado con las funciones de la edición profesional, como el cifrado de datos.  
También es la única fuente de financiación del equipo y una garantía importante para el mantenimiento saludable y la actualización continua del producto.  
Mientras el problema de financiación no se resuelva, no planeamos abrir este módulo.  

En general, nuestros objetivos son dos:  
1. **Lograr ingresos razonables a través de una comercialización adecuada** — para proporcionar un impulso a largo plazo para el desarrollo y mantenimiento continuos;  
2. **Mantener el código disponible para la auditoría de los usuarios** — para que todos puedan confiar en que el software cumple con el principio de “no hacer el mal” y puedan usarlo con tranquilidad.  

Esperamos mantenernos fieles a nuestra intención original y al mismo tiempo garantizar un desarrollo sostenible, de modo que el software pueda avanzar de manera estable y llegar lejos.  

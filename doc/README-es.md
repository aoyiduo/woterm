*Este repositorio no contiene bibliotecas de terceros ni código fuente del módulo kxver.  
El módulo kxver se utiliza principalmente para el control de versiones, especialmente para funciones relacionadas con la versión ultimate, como el cifrado de datos.  
Es la única fuente de financiación del equipo y una garantía importante para el mantenimiento y la actualización del producto.  
Hasta que se resuelva la fuente de financiación, no hay planes de abrir el código fuente de este módulo.*  
***
<p align="center">
  <img src="doc/woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
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

## Descripción de funciones
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

## Vista previa UI:
<div>Interfaz principal<br><img src="doc/main.gif"/></div>
<div>Gestión de claves SSH<br><img src="doc/keymgr2.gif"></div>
<div>Puerto serie<br><img src="doc/serialport.gif"></div>
<div>Túnel<br><img src="doc/tunnel.png"></div>
<div>Varias pieles<br><img src="doc/skins.png"></div>
<div>Playbook<br><img src="doc/playbook.gif"></div>
<div>Combinar y separar<br><img src="doc/merge.gif"></div>
<div>Sincronización en la nube<br><img src="doc/sync.gif"></div>
<div>Filtro por palabras clave<br><img src="doc/filter.gif"></div>
<div>Transferencia de archivos<br><img src="doc/sftp.gif"></div>
<div>Pestañas flotantes<br><img src="doc/float.gif"></div>
<div>División de ventanas<br><img src="doc/split.gif"></div>
<div>Tema de terminal<br><img src="doc/patten.gif"></div>
<div>Terminal VNC<br><img src="doc/vnc.gif"/></div>

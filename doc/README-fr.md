<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">Multi-plateforme / faible mémoire / multi-protocoles / multi-fonctions / synchronisation cloud</h2>
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
  <h3 style="text-align: center;">[<a href="https://woterm.com">Site officiel WoTerm</a>]</a></h3>
</p>

# Aperçu
Intégré avec tous les principaux protocoles de communication à distance : prend en charge SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, port série, TCP, UDP et plus — plus besoin de changer d’outil. Gérez facilement divers scénarios d’accès à distance sur une plateforme unifiée.

## Description des fonctions[<a href="https://en.woterm.com/versions/">&gt;&gt;&gt;Cliquez ici pour voir plus de fonctions&lt;&lt;&lt;</a>]
- Supporte Windows / Linux / MacOSX / Android.  
- Supporte les protocoles de communication principaux : SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP, etc.  
- Plusieurs thèmes intégrés et extensibles pour personnalisation.  
- Mode administrateur pour voir les mots de passe des sessions et prévenir le vol.  
- Protocole de synchronisation cloud basé sur SFTP avec 8 algorithmes de chiffrement.  
- Terminal SSH avec assistant SFTP intégré, supporte également les onglets SFTP indépendants.  
- Onglets flottants pour collaboration multi-fenêtres.  
- Segmentation de fenêtres illimitée.  
- Groupement de sessions et filtres de recherche par mots-clés.  
- Des dizaines de couleurs de terminal disponibles.

- Support du bureau à distance VNC, fonctionnalités avancées nécessitent [WoVNCServer](http://wovnc.com).  
  - Support des protocoles RFB 3.3/3.7/3.8.  
  - Support des encodages standards : ZRLE / TRLE / Hextile / CopyRect / RRE / Raw.  
  - Support des encodages étendus : H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2.  
  - Verrouillage automatique de l’écran en cas de déconnexion ou fin de session.  
  - Mode écran noir à distance.  
  - Lecture sonore du bureau distant.  
  - Changement libre de qualité d’image : sans perte, HD, normale, 16/15/8 bits.  
  - Support multi-écran et modes étiré, écran partagé, écran divisé.  

- Support du terminal distant.  
  - Accès aux terminaux Windows/Linux/MacOSX.  
  - Support Proxy jump.  
  - Gestion des certificats.  
  - Configuration des raccourcis clavier.  
  - Upload/Download fichiers ZModem.  
  - Interaction collaborative SFTP intégrée.  
  - Divers thèmes de terminal.

## Description du module
Le code client est entièrement open source (sauf le module kxver), le code tiers provient de GitHub, CodeProject ou autres communautés ouvertes. Certains modules proviennent de [WoVNC](http://wovnc.com).  

## WoVNCServer
Recommandé [WoVNCServer](http://wovnc.com) pour fonctionnalités avancées.  

## Plus d'informations:
<a href="http://www.woterm.com">http://www.woterm.com</a>  

## Téléchargement des binaires:
<a href="http://woterm.com">http://woterm.com</a>  

## Performances

Moteur d’analyse de terminal auto-développé, capable d’analyser et d’afficher des dizaines de millions d’enregistrements en quelques secondes tout en maintenant une utilisation mémoire extrêmement faible. [Voir le processus détaillé](Performance-fr.md)

![](timeseq1.png)  
![](urandom_test_speed.png)  
![](urandom_test_memory.png)  

Recherche et localisation ultra-rapides dans des millions de lignes de texte  
![](search.gif)

---

## Aperçu UI

**Interface principale**  
![](main.gif)  

**Gestion des clés SSH**  
![](keymgr2.gif)  

**Port série**  
![](serialport.gif)  

**Tunnel**  
![](tunnel.png)  

**Multiples thèmes**  
![](skins.png)  

**Playbook**  
![](playbook.gif)  

**Fusion et séparation**  
![](merge.gif)  

**Synchronisation cloud**  
![](sync.gif)  

**Filtre par mot-clé**  
![](filter.gif)  

**Transfert de fichiers**  
![](sftp.gif)  

**Onglets flottants**  
![](float.gif)  

**Division de fenêtres**  
![](split.gif)  

**Thème terminal**  
![](patten.gif)  

**Surlignage de la syntaxe**  
![](highlight.gif)  

**Terminal VNC**  
![](vnc.gif)



# Déclaration Open Source
Depuis la version v10, nous avons ajusté notre licence open source. À l’avenir, que nous continuions à maintenir une partie du code en open source ou que nous progressions vers un open source complet, nous conserverons toujours une attitude ouverte et transparente.  

Il convient de préciser que ce dépôt ne contient ni bibliothèques tierces, ni le code source/fichiers du module **kxver**.  
Le **module kxver** est principalement utilisé pour le contrôle de version, notamment en lien avec les fonctionnalités de l’édition professionnelle telles que le chiffrement des données.  
Il constitue également la seule source de financement de l’équipe et une garantie essentielle pour la maintenance et l’évolution continue du produit.  
Tant que la question du financement n’est pas résolue, nous ne prévoyons pas d’ouvrir ce module.  

Globalement, nos objectifs sont au nombre de deux :  
1. **Obtenir des revenus raisonnables grâce à une commercialisation appropriée** — afin d’assurer une motivation à long terme pour le développement et la maintenance continus ;  
2. **Maintenir le code disponible pour l’audit des utilisateurs** — afin que chacun puisse avoir la certitude que le logiciel respecte le principe de “ne pas nuire” et puisse l’utiliser en toute confiance.  

Nous espérons rester fidèles à notre intention initiale tout en garantissant un développement durable, pour que le logiciel avance de manière stable et sur le long terme.  

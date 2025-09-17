---

# WoTerm Leistungstestbericht

Dieser Test basiert auf **Version v10**, da danach tiefgreifende Kernel-Optimierungen erfolgten.  
Dank an alle Nutzer für die Unterstützung sowie an den **WindTerm**-Autor: wir haben uns nie getroffen und technisch nicht ausgetauscht, aber WindTerm ist ein exzellentes Programm und diente WoTerm als Inspiration für einige Funktionen.

---

## 🖥 Testumgebung

| Komponente        | Konfiguration                    |
|-------------------|----------------------------------|
| **WoTerm-Host**   | i5 12. Gen, 48 GB RAM            |
| **Test-Server**   | 2 Kerne, 8 GB RAM, Fedora, VM (NAT) |

---

## ⚙ Testbefehle

| Befehl              | Beschreibung                  |
|---------------------|-------------------------------|
| `time seq 10000000` | 10 Mio. Zeilen erzeugen       |
| `urandom_test.sh`   | 100 MB Zufallsdaten erzeugen  |

---

## 📦 Programmversionen

| Programm    | Version |
|-------------|---------|
| **WoTerm**   | v10.2.1 |
| **WindTerm** | v2.7.0  |
| **PuTTY**    | v0.83   |

---

## ⏱ Methodik

- **Testdatum:** 18. April 2025  
- Jeder Befehl mind. **10 Mal** ausgeführt  
- **Max./Min. entfernt**, Durchschnitt gebildet  

---

## Befehlsdauer

Befehl `time seq 10000000` auf Server ausgeführt, Gesamtdauer per Handy-Stoppuhr gemessen.  
Vom Befehl ausgegebene Zeit spiegelt nur SSH-Empfangsende wider, daher ungerecht.  
Datenpfad:

*SSH-Thread → Haupt-Thread → Terminal-Thread → Haupt-Thread Anzeige*

Deswegen Stoppuhr.  
<div>Befehlsdauer<br><img src="timeseq1.png"/></div>

## urandom_test Durchsatz

Vergleich der **Datensätze pro Sekunde (Durchsatz)**.  
<div>Durchsatz<br><img src="urandom_test_speed.png"/></div>

## Speicherverbrauch nach urandom_test

Zusätzlicher Speicher **nach Abzug der Basislast** nach dem Start.  
<div>Speicherverbrauch<br><img src="urandom_test_memory.png"/></div>

*Hinweis: Ergebnisse beziehen sich auf obige Konfiguration, andere Hardware kann abweichen.*

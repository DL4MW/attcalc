### Umgebung ###
* Das Projekt ist Programm in C++ und benutzt Qt
* Die Zielplattform ist Windows, Linux und Android
* Exceptions sollen vermieden werden

### Programmzweck ###

* Das Programm berechnet die Dämpfung eines Koaxialkabels anhand des Kabeltyps, der Länge und der Frequenz
* Jedes Kabel wird durch eine Datei des Dateityps .cbl beschrieben, in der die relevanten Parameter und Metadaten in Form einer INI-Datei abgelegt sind
* Der Nutzer kann selbst neue kabeltypen definieren. anhand derr Frequenz-Dämpfungs-Tabelle wird über ein Least-mean-Square Verfahren die relevanten Parameter k0, k1 und k2 errechnet
* 

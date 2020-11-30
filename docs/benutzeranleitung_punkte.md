Anleitung SmartRelief konfigurieren
===================================

Um zu Beginnen führe die untenstehenden Schritte aus und suche das Kapitel welches die gewünschten Änderungen bietet:

1.  «data» Ordner öffnen

2.  «variables.js» öffnen (In Visual Studio Code)

Die Datei beinhaltet 2 Datengruppen: «var points» und «var groups»,\
in var points befinden sich die der Name und die Nummer von allen
Punkten\
in var groups befinden sich sämtliche Infos zu den Gruppen, die Punkte
werden dort mit Ihren Nummern repräsentiert (Daher befindet sich der
Name nur in «var points»). Jede Punkt Nummer sollte also in beiden
Gruppen vorhanden sein (ansonsten kommt es zu Fehlern). Im Moment wird
dies noch nicht automatisch überprüft (Ich arbeite noch an einer
Möglichkeit). Daher lohnt es sich die Sache 2 mal anzuschauen.

Einen Punkt Namen ändern:
-------------------------

1.  Den Namen suchen den man ändern will (z.B Lauberhorn):

    ![](.//media/image1.PNG)

2.  Dann den Teil zwischen den beiden Anführungszeichen  " anpassen
    (In userem Fall ersetzen wir Lauberhorn durch den Text «Das ist ein
    Gifpel»

    ![](.//media/image2.PNG)

3.  Dabei zu beachten ist, dass man auch die Nummern ändern kann, dies
    aber nicht empfehlenswert ist, da man schneller die Übersicht
    verliert. Darum besser die Namen anpassen, damit die Reihenfolge
    bestehen bleibt.

Einen Gruppennamen ändern:
--------------------------

1.  «var groups» suchen

![](.//media/image3.PNG)

2.  Die gewünschte Gruppe suchen (In userem Fall Orte)

3.  Den Text zwischen den Anführungszeichen " anpassen (In userem
    Fall wird Orte zu «newGroup»

    ![](.//media/image4.PNG)

Um einen Punkt hinzuzufügen:
----------------------------

1.  var points = { suchen:

![](.//media/image1.PNG)

2.  Den Schluss suchen

![](.//media/image5.PNG)

3.  Hier kurz einige Infos:

    a.  Jede Zeile ist wie folgt aufgebaut: NUMMER : ''NAME'',

    - Die Anführungszeichen mit Shift und 2 machen.

    b.  Nur die letzte Zeile hat kein Komma (Im Bild Klein
        Grünhornlücke) am Schluss

    c.  Die Gruppe wird mit « }; » abgeschlossen (daran nichts ändern).

4.  Wenn man nun einen neuen Punkt hinzufügen möchte (z.B Nummer 64 mit
    dem Namen «Ein Gipfel» geht man wie folgt vor:

    a.  Beim bisher letzten Eintrag ein Komma setzen:

    ![](.//media/image6.PNG)

    b.  Anschliessend mit Einfügen eine neue Zeile erzeugen

    c.  Folgenden Text einfügen: 64 :
        \"Ein Gipfel\"\
        (Bei der letzten Zeile wird kein Komma mehr gesetzt)

    ![](.//media/image7.PNG)

5.  Nun muss man den Punkt noch einer Gruppe zuordnen, die Gruppen sind
    in «var groups» gespeichert:
    
    ![](.//media/image3.PNG)

6.  Nun sucht man die Gruppe in die man den Punkt hinzufügen möchte, in
    unserem Fall werden wir den Punkt in die Gruppe «Gewaesser»
    schreiben (obwohl der Punkt «ein Gipfel» ist )

    a.  Nun sucht man den Teil in dem die Punkte aufgelistet sind (Der
        letzte Teil in Eckigen Klammern ( \[ \] ) auf einer Zeile (der
        markierte Bereich im
        Bild)
    
    ![](.//media/image8.PNG)

    b.  Anschliessend setzt man ein Komma hinter den letzten Punkt
        (Hier 26) und schreibt die neue Nummer (Hier 65)
        hinein:
    
    ![](.//media/image9.PNG)

    c.  Nun sollte der neue Punkt auf der Webseite verfügbar sein.

Um die Farbe einer Gruppe zu ändern:
------------------------------------

1.  «var groups» finden:

![](.//media/image3.PNG)

2.  Die Zeile finden bei der man die Farbe ändern möchte (Wir werden
    hier die Farbe der Gruppe Gletscher ändern).

3.  Die Eckigen Klammern mit 3 Zahlen dazwischen definieren die
    Farbwerte, daher werden wir den markierten Ausschnitt anpassen:

![](.//media/image10.PNG)

4.  Die 3 Werte auf die gewünschte Farbe
    anpassen (0,120,255 in userem Fall)

![](.//media/image11.PNG) 

Die 3 Werte beziehen sich auf RGB (Rot, Grün, Blau), die
    jeweilige Zahl repräsentiert also den jeweiligen Farbanteil (Die
    Werte reichen von 0 -- 255). Hier klicken um Farben zu
    generieren

5.  Die Farben sollten nun angepasst sein.

Um eine neue Gruppe hinzuzufügen:
---------------------------------

1.  «var groups» finden

> ![](.//media/image3.PNG)

2.  Auch bei den Gruppen ist ganz am Schluss jeder Zeile ein Komma (Bis
    auf die letzte), daher bevor wir eine neue Zeile einfügen, setzen
    wir am Schluss der letzten Zeile ein
    Komma:
    
    ![](.//media/image12.PNG)
3.  Am einfachsten ist es hier eine Gruppe zu kopieren (Zeile markieren
    -\> rechtsklick --\> Kopieren -\> Eine neue Zeile erstellen -\> Auf
    neue Zeile klicken -\> Einfügen

![](.//media/image13.PNG)

4.  Das Komma auf der letzten Zeile
    entfernen:
    
    ![](.//media/image14.PNG)

5.  Anschliessend die Werte anpassen (Der erste Text bezieht sich auf
    den Namen der Gruppe, Die folgende Zahlengruppe repräsentieren die
    Farbwerte (Siehe auch Anleitung für Farbänderung) und die letzte
    Zahlenmenge sind sämtliche Punkte

    ![](.//media/image15.PNG)

6.  Natürlich dürfen die Gruppen nur Punkte enthalten die man auch
    erstellt hat, für unser Beispiel müsste man also noch die Punkte 64,
    65, 66 und 67 erstellen. Anschliessend ist die neue Gruppe fertig
    erstellt.

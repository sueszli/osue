Wie hängen `fflush()` und `fclose()` zusammen?
---
`fclose()` ruft `fflush()` auf welches das Schreiben gepufferter Daten erzwingt. Danach wird Stream und File Descriptor geschlossen.
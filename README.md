\
    Flappy DS - Progetto completo (sorgente)

    Contenuto:
    - source/main.c : codice sorgente (libnds) con menu, modalità dual/single, logica tubi, transizione tra schermi
    - Makefile : Makefile minimale per devkitPro/devkitARM (potrebbe richiedere aggiustamenti)
    - data/ : cartella per gli asset (sprite, palette, suoni) - file placeholder
    - .github/workflows/build.yml : workflow per compilare su GitHub Actions (usa docker image con devkitPro)

    **Non posso compilare il .nds qui.** Scarica questo progetto e compila localmente o tramite GitHub Actions:
    - Per compilare in locale: installa devkitPro (https://devkitpro.org), apri una shell nella cartella e lancia `make`.
    - Per compilare su GitHub Actions: crea un repo, carica questo progetto, push su main; la Action creerà l'artefatto.

    NOTE SULLA LOGICA IMPLEMENTATA:
    - Menu con due opzioni: "Gioca con due schermi" e "Gioca con schermo superiore".
    - Modalità dual: campo verticale unico 384px; zona invisibile (barra) tra gli schermi; quando l'uccello entra nella barra sparisce per un breve intervallo e riappare nello schermo opposto; i tubi scorrono continuamente.
    - Modalità single: gameplay solo nel top; bottom mostra comandi; dopo gameover bottom mostra risultati.
    - Tutte le grafiche/suoni sono placeholder; punti di integrazione sono documentati nei commenti del sorgente.

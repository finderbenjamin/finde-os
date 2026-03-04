# CLI-Spezifikation (verbindlich)

Diese Spezifikation ist die **verbindliche Basis für neue Commands**.
Jeder neue CLI-Befehl muss dieses Format und diese Regeln einhalten.

## 1) Befehls-Syntax pro Command

Grundform:

```text
finde <command> <subcommand> [positionals...] [--flag value] [--bool-flag]
```

Regeln:
- `<command>` und `<subcommand>` sind kleingeschrieben und nutzen `kebab-case`.
- Flags sind lang (`--flag-name`) und ebenfalls `kebab-case`.
- Flags mit Wert akzeptieren genau ein Argument (`--output path/to/file`).
- Bool-Flags haben keinen Wert (`--verbose`).
- Wiederholbare Flags sind erlaubt, falls im jeweiligen Command dokumentiert.

Beispiele:

```text
finde image build --input ./rootfs --output ./image.iso
finde vm run --image ./image.iso --headless
finde cap list --format json
```

## 2) Einheitliche Exit-Codes

Alle Commands nutzen diese Exit-Codes:

- `0`: Erfolg
- `1`: Allgemeiner Laufzeitfehler (nicht weiter klassifiziert)
- `2`: Usage-/Syntax-Fehler (falsche Argumente, ungültige Flags)
- `13`: Permission denied (fehlende Rechte/Capabilities)
- `64`: Externe Abhängigkeit nicht verfügbar (z. B. Tool fehlt)
- `70`: Interner Fehler (Bug, Invariante verletzt)

## 3) Standard-Fehlerformat

Jeder Fehler gibt **mindestens eine menschenlesbare Zeile** auf `stderr` aus:

```text
ERROR <code> <kind>: <message>
```

Beispiel:

```text
ERROR 2 usage: unknown flag --ouptut
```

Optional kann zusätzlich maschinenlesbar ausgegeben werden (z. B. bei `--error-format json`):

```json
{"ok":false,"code":2,"kind":"usage","message":"unknown flag --output"}
```

## 4) Deterministische Kommandos (Test-relevant)

Diese Command-Arten **müssen deterministisch** sein, damit sie im Stil von `scripts/test.sh` zuverlässig per Log-Marker geprüft werden können:

- Alle `self-test`/`check`/`verify`-Kommandos
- Alle Kommandos, die in CI oder in `scripts/test.sh` eingebunden werden
- Alle Kommandos mit Erfolgsmarker-Ausgabe (z. B. `*_OK`)

Determinismus-Regeln:
- Keine zeit- oder zufallsabhängigen Ausgaben ohne feste Seeds.
- Stabile, exakt erwartbare Erfolgsmarker in `stdout`/`serial`.
- Fehlertexte für bekannte Fehlerfälle stabil halten.
- Reihenfolge von Listen-Ausgaben fest definieren (sortiert, falls nötig).
- Für Merge/PR-Freigabe gilt: die relevante Testausführung muss mit exakt `PASS` enden.

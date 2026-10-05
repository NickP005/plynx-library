#!/usr/bin/env python3
"""
La scheda dichiara quanti pin ha il suo pettine? (PLYNX_BOARD_PINS -> "pins")

🔴 PERCHE' ESISTE. Le DevKit ESP32 da 30 e da 38 pin montano lo stesso chip:
niente di quello che la scheda sa di se' le distingue, e finora l'app chiedeva
all'utente quale delle due avesse. Dalla 1.2.0 lo sketch lo scrive con
`#define PLYNX_BOARD_PINS 38` e la libreria lo manda nel blocco blnkinf, a ogni
collegamento. Questo cancello compila sketch veri con arduino-cli e legge il
blocco DENTRO IL BINARIO, che e' esattamente quello che la scheda spedisce (e
quello che il server legge da un .bin caricato per l'OTA).

Si prova nei due versi:
  - senza la define il blocco NON ha "pins" (le schede di oggi non cambiano);
  - con 30 e con 38 lo ha, col valore giusto, su ESP32 e su AVR (dove il blocco
    sta in PROGMEM e viene copiato sullo stack: 8 byte in piu');
  - con 36 la compilazione SI FERMA con il messaggio della libreria: un numero
    sbagliato metterebbe a schermo un pettine che la scheda non ha.

Uso:
  python3 verifica/pin_dichiarati.py [--out CARTELLA]

Con --out scrive anche i corpi dei blocchi (i byte esatti che la scheda manda
dopo "blnkinf\\0") come blnkinf-<scheda>-<pins>.bin: il server li rigioca sul
filo con plynx-server/verifica/pin_dichiarati_sul_filo.sh --blnkinf <file>.

Esiti: 0 tutto vero; 1 qualcosa non torna; 2 CIECO (arduino-cli o un core
mancano: non si esce mai 0 per non aver guardato).

⚠️ Nel percorso delle librerie c'e' SOLO questa Plynx (un collegamento a questo
albero): con la Plynx installata nello sketchbook accanto, arduino-cli puo'
sceglierne un'altra e la prova misurerebbe il codice sbagliato. Ethernet arriva
dalle librerie incluse nei dati di arduino-cli.
"""
from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

REPO = pathlib.Path(__file__).resolve().parent.parent
DATI_ARDUINO = pathlib.Path.home() / "Library" / "Arduino15"
MARCATORE = b"blnkinf\x00"

SCHEDE = {
    # nome: (fqbn, esempio)
    "esp32": ("esp32:esp32:esp32", "examples/Boards_WiFi/ESP32_WiFi/ESP32_WiFi.ino"),
    "uno": ("arduino:avr:uno", "examples/Boards_Ethernet/Arduino_Ethernet/Arduino_Ethernet.ino"),
}

rotte = 0


def dico(cosa: str, bene: bool, dettaglio: str = "") -> None:
    global rotte
    if not bene:
        rotte += 1
    print(("  ✓ " if bene else "  ✗ ") + cosa + ("" if bene or not dettaglio else f"  ->  {dettaglio}"))


def cieco(perche: str) -> None:
    print(f"QUESTO CANCELLO E' CIECO: {perche}", file=sys.stderr)
    sys.exit(2)


def config(banco: pathlib.Path) -> pathlib.Path:
    libs = banco / "user" / "libraries"
    libs.mkdir(parents=True)
    os.symlink(REPO, libs / "Plynx")
    yaml = banco / "arduino-cli.yaml"
    # Le librerie incluse (Ethernet per l'AVR) vanno nominate: arduino-cli
    # non le cerca da solo nella cartella dei dati.
    yaml.write_text(
        "directories:\n"
        f"    data: {DATI_ARDUINO}\n"
        f"    user: {banco / 'user'}\n"
        "    builtin:\n"
        f"        libraries: {DATI_ARDUINO / 'libraries'}\n"
    )
    return yaml


def sketch(banco: pathlib.Path, scheda: str, pins: int | None) -> pathlib.Path:
    """Una copia dell'esempio, con la define come la scrive una persona: prima
    dell'include."""
    _, esempio = SCHEDE[scheda]
    nome = f"Prova_{scheda}_{pins or 'senza'}"
    cartella = banco / "sketch" / nome
    cartella.mkdir(parents=True)
    testo = (REPO / esempio).read_text()
    # Via la riga di esempio commentata, se c'e': la define la mettiamo noi.
    righe = [r for r in testo.split("\n") if "PLYNX_BOARD_PINS" not in r]
    if pins is not None:
        primo_include = next(i for i, r in enumerate(righe) if r.startswith("#include"))
        righe.insert(primo_include, f"#define PLYNX_BOARD_PINS {pins}")
    (cartella / f"{nome}.ino").write_text("\n".join(righe))
    return cartella


def compila(yaml: pathlib.Path, scheda: str, cartella: pathlib.Path) -> tuple[int, str, pathlib.Path]:
    fqbn, _ = SCHEDE[scheda]
    uscita = cartella / "build"
    p = subprocess.run(
        ["arduino-cli", "--config-file", str(yaml), "compile", "--fqbn", fqbn,
         "--build-path", str(uscita), str(cartella)],
        capture_output=True, text=True)
    return p.returncode, p.stdout + p.stderr, uscita


def flash(log: str) -> int | None:
    """I byte di programma che arduino-cli dice di aver usato."""
    for riga in log.splitlines():
        if riga.startswith("Sketch uses "):
            return int(riga.split()[2].replace(",", "").replace(".", ""))
    return None


def blocco(build: pathlib.Path) -> bytes | None:
    """Il corpo che la scheda spedisce: i byte dopo "blnkinf\\0" fino al doppio
    zero che chiude il letterale (PlynxApiArduino.h: profile+8, lungo
    sizeof-8-2)."""
    elf = next(build.glob("*.elf"), None)
    if elf is None:
        return None
    dati = elf.read_bytes()
    trovati = []
    inizio = 0
    while (i := dati.find(MARCATORE + b"ver\x00", inizio)) >= 0:
        fine = dati.find(b"\x00\x00", i + len(MARCATORE))
        trovati.append(dati[i + len(MARCATORE):fine + 1])
        inizio = i + 1
    # Un letterale solo: due copie diverse vorrebbero dire che non sappiamo
    # quale parte.
    return trovati[0] if len(set(trovati)) == 1 else None


def coppie(corpo: bytes) -> dict[str, str]:
    pezzi = corpo.split(b"\x00")
    if pezzi and pezzi[-1] == b"":
        pezzi = pezzi[:-1]
    return {pezzi[i].decode(): pezzi[i + 1].decode() for i in range(0, len(pezzi) - 1, 2)}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=pathlib.Path)
    args = ap.parse_args()

    if shutil.which("arduino-cli") is None:
        cieco("arduino-cli non c'e'.")
    cores = subprocess.run(["arduino-cli", "core", "list"], capture_output=True, text=True).stdout
    for core in ("esp32:esp32", "arduino:avr"):
        if core not in cores:
            cieco(f"manca il core {core}.")

    with tempfile.TemporaryDirectory(prefix="plynx-pins-") as t:
        banco = pathlib.Path(t)
        yaml = config(banco)
        lunghezze: dict[tuple[str, int | None], int] = {}
        pesi: dict[tuple[str, int | None], int] = {}

        for scheda in SCHEDE:
            print(f"== {scheda} ({SCHEDE[scheda][0]})")
            for pins in (None, 30, 38):
                cartella = sketch(banco, scheda, pins)
                codice, log, build = compila(yaml, scheda, cartella)
                etichetta = f"PLYNX_BOARD_PINS {pins}" if pins else "senza la define"
                if codice != 0:
                    dico(f"{etichetta}: compila", False, log.strip().splitlines()[-1] if log.strip() else "")
                    continue
                corpo = blocco(build)
                if corpo is None:
                    dico(f"{etichetta}: il blocco blnkinf si legge nel binario", False)
                    continue
                kv = coppie(corpo)
                lunghezze[(scheda, pins)] = len(corpo)
                if (peso := flash(log)) is not None:
                    pesi[(scheda, pins)] = peso
                if pins is None:
                    dico(f"{etichetta}: niente \"pins\" ({len(corpo)} byte, chiavi {', '.join(kv)})",
                         "pins" not in kv, str(kv))
                else:
                    dico(f"{etichetta}: \"pins\" = {kv.get('pins')} ({len(corpo)} byte)",
                         kv.get("pins") == str(pins), str(kv))
                dico(f"{etichetta}: la prima chiave resta \"ver\" (il server smista sulla prima lettera)",
                     next(iter(kv), None) == "ver")
                dico(f"{etichetta}: la versione nel blocco e' quella di library.properties",
                     kv.get("ver") == versione(), f"{kv.get('ver')} contro {versione()}")
                if args.out:
                    args.out.mkdir(parents=True, exist_ok=True)
                    (args.out / f"blnkinf-{scheda}-{pins or 'senza'}.bin").write_bytes(corpo)

            for pins in (30, 38):
                if (scheda, None) in lunghezze and (scheda, pins) in lunghezze:
                    delta = lunghezze[(scheda, pins)] - lunghezze[(scheda, None)]
                    dico(f"con {pins} il blocco cresce di 8 byte (\"pins\\0{pins}\\0\")", delta == 8, f"{delta}")
                if (scheda, None) in pesi and (scheda, pins) in pesi:
                    print(f"    programma: {pesi[(scheda, None)]} byte senza, {pesi[(scheda, pins)]} con "
                          f"{pins} ({pesi[(scheda, pins)] - pesi[(scheda, None)]:+d})")

        print("== un numero che non e' un pettine")
        for scheda in ("esp32",):
            cartella = sketch(banco, scheda, 36)
            codice, log, _ = compila(yaml, scheda, cartella)
            dico("PLYNX_BOARD_PINS 36 non compila", codice != 0)
            dico("e il motivo e' scritto", "PLYNX_BOARD_PINS must be 30 or 38" in log,
                 log.strip().splitlines()[-1] if log.strip() else "")

    print()
    print("tutto vero" if rotte == 0 else f"{rotte} cose non tornano")
    return 0 if rotte == 0 else 1


def versione() -> str:
    for riga in (REPO / "library.properties").read_text().splitlines():
        if riga.startswith("version="):
            return riga.split("=", 1)[1].strip()
    return "?"


if __name__ == "__main__":
    sys.exit(main())

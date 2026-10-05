#!/usr/bin/env python3
"""
La scheda dice quale ESP32 e'? (CONFIG_IDF_TARGET -> "cpu", libreria 1.3.0)

🔴 PERCHE' ESISTE. ESP32, ESP32-C3 ed ESP32-S3 dichiaravano tutte e tre
dev "ESP32" e nessun "cpu": per il server erano la stessa scheda, e un
aggiornamento compilato per una partiva verso l'altra, si scaricava per intero
e veniva rifiutato in fondo dal bootloader («mismatch chip ID»), tre volte.
Dalla 1.3.0 la scheda manda "cpu" con il nome che ESP-IDF da' al chip, e "dev"
resta "ESP32" (i server e le app di prima leggono quella parola).

Questo cancello compila l'esempio ESP32_WiFi per i tre chip con arduino-cli e
legge il blocco blnkinf DENTRO IL .bin, cioe' quello che il server legge da un
binario caricato per l'OTA e che la scheda manda a ogni collegamento. Accanto
legge il chip_id dell'intestazione dell'immagine (byte 12-13), il fatto che il
bootloader controlla: le due cose devono dire lo stesso chip.

Nei due versi:
  - con la 1.3.0 "cpu" c'e' ed e' quello dell'intestazione, su tutti e tre;
  - "dev" resta "ESP32" su tutti e tre (compatibilita');
  - l'esempio ESP32_BT (Bluetooth classico) su una C3 si FERMA con la frase
    della libreria, e sull'ESP32 classico compila ancora;
  - PLYNX_LIB=<albero di una libreria vecchia> fa girare le stesse prove su
    quell'albero: con la 1.2.0 devono diventare rosse ("cpu" assente, errore
    oscuro sul BT).

Uso:
  python3 verifica/chip_dichiarato.py [--out CARTELLA] [--solo esp32c3,...]

Con --out scrive i corpi dei blocchi (i byte che la scheda manda dopo
"blnkinf\\0") come blnkinf-<chip>.bin: il server li rigioca sul filo con
plynx-server/verifica/modelli_scheda_sul_filo.sh --blnkinf CARTELLA.

Esiti: 0 tutto vero; 1 qualcosa non torna; 2 CIECO (arduino-cli o il core
esp32 mancano: non si esce mai 0 per non aver guardato).

⚠️ Nel percorso delle librerie c'e' SOLO questa Plynx (un collegamento
all'albero): con un'altra Plynx accanto arduino-cli potrebbe scegliere quella
e la prova misurerebbe il codice sbagliato. Una compilazione alla volta, con
nice 19: la macchina e' condivisa.
"""
from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import struct
import subprocess
import sys
import tempfile

REPO = pathlib.Path(os.environ.get("PLYNX_LIB") or pathlib.Path(__file__).resolve().parent.parent)
DATI_ARDUINO = pathlib.Path.home() / "Library" / "Arduino15"
MARCATORE = b"blnkinf\x00"

# chip: (fqbn, chip_id atteso nell'intestazione dell'immagine)
CHIP = {
    "esp32": ("esp32:esp32:esp32", 0),
    "esp32c3": ("esp32:esp32:esp32c3", 5),
    "esp32s3": ("esp32:esp32:esp32s3", 9),
}
WIFI = "examples/Boards_WiFi/ESP32_WiFi/ESP32_WiFi.ino"
BT = "examples/Boards_Bluetooth/ESP32_BT/ESP32_BT.ino"

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
    yaml.write_text(
        "directories:\n"
        f"    data: {DATI_ARDUINO}\n"
        f"    user: {banco / 'user'}\n"
        "    builtin:\n"
        f"        libraries: {DATI_ARDUINO / 'libraries'}\n"
    )
    return yaml


def sketch(banco: pathlib.Path, esempio: str, nome: str) -> pathlib.Path:
    cartella = banco / "sketch" / nome
    cartella.mkdir(parents=True)
    (cartella / f"{nome}.ino").write_text((REPO / esempio).read_text())
    return cartella


def compila(yaml: pathlib.Path, fqbn: str, cartella: pathlib.Path) -> tuple[int, str, pathlib.Path]:
    uscita = cartella / "build"
    p = subprocess.run(
        ["nice", "-n", "19", "arduino-cli", "--config-file", str(yaml), "compile", "--fqbn", fqbn,
         "--build-path", str(uscita), str(cartella)],
        capture_output=True, text=True)
    return p.returncode, p.stdout + p.stderr, uscita


def immagine(build: pathlib.Path) -> bytes | None:
    """Il .bin dell'applicazione: quello che si carica per l'OTA."""
    for b in sorted(build.glob("*.ino.bin")):
        return b.read_bytes()
    return None


def blocco(dati: bytes) -> bytes | None:
    """Il corpo che la scheda spedisce: i byte dopo "blnkinf\\0" fino al doppio
    zero che chiude il letterale (PlynxApiArduino.h)."""
    trovati = []
    inizio = 0
    while (i := dati.find(MARCATORE + b"ver\x00", inizio)) >= 0:
        fine = dati.find(b"\x00\x00", i + len(MARCATORE))
        trovati.append(dati[i + len(MARCATORE):fine + 1])
        inizio = i + 1
    return trovati[0] if len(set(trovati)) == 1 else None


def coppie(corpo: bytes) -> dict[str, str]:
    pezzi = corpo.split(b"\x00")
    if pezzi and pezzi[-1] == b"":
        pezzi = pezzi[:-1]
    return {pezzi[i].decode(): pezzi[i + 1].decode() for i in range(0, len(pezzi) - 1, 2)}


def versione() -> str:
    for riga in (REPO / "library.properties").read_text().splitlines():
        if riga.startswith("version="):
            return riga.split("=", 1)[1].strip()
    return "?"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=pathlib.Path)
    ap.add_argument("--solo", help="chip separati da virgola (esp32,esp32c3,esp32s3)")
    ap.add_argument("--senza-bt", action="store_true", help="salta le due compilazioni dell'esempio BT")
    args = ap.parse_args()

    if shutil.which("arduino-cli") is None:
        cieco("arduino-cli non c'e'.")
    cores = subprocess.run(["arduino-cli", "core", "list"], capture_output=True, text=True).stdout
    if "esp32:esp32" not in cores:
        cieco("manca il core esp32:esp32.")
    scelti = args.solo.split(",") if args.solo else list(CHIP)
    print(f"libreria: {REPO} (versione {versione()})")

    with tempfile.TemporaryDirectory(prefix="plynx-chip-") as t:
        banco = pathlib.Path(t)
        yaml = config(banco)
        for chip in scelti:
            fqbn, chip_id = CHIP[chip]
            print(f"== {chip} ({fqbn})")
            codice, log, build = compila(yaml, fqbn, sketch(banco, WIFI, f"Wifi_{chip}"))
            if codice != 0:
                dico("ESP32_WiFi compila", False, log.strip().splitlines()[-1] if log.strip() else "")
                continue
            dati = immagine(build)
            if dati is None or len(dati) < 24:
                dico("il .bin c'e'", False)
                continue
            letto = struct.unpack("<H", dati[12:14])[0]
            dico(f"l'intestazione dell'immagine dice chip_id {letto} (atteso {chip_id})",
                 dati[0] == 0xE9 and letto == chip_id, f"magic 0x{dati[0]:02X}")
            corpo = blocco(dati)
            if corpo is None:
                dico("il blocco blnkinf si legge nel .bin", False)
                continue
            kv = coppie(corpo)
            dico(f"\"cpu\" = {kv.get('cpu')!r} (atteso {chip!r})", kv.get("cpu") == chip, str(kv))
            dico("\"dev\" resta \"ESP32\" (server e app di prima leggono questa parola)",
                 kv.get("dev") == "ESP32", str(kv.get("dev")))
            dico("la prima chiave resta \"ver\" (il server smista sulla prima lettera)",
                 next(iter(kv), None) == "ver")
            dico(f"la versione nel blocco e' quella di library.properties ({versione()})",
                 kv.get("ver") == versione(), str(kv.get("ver")))
            if args.out:
                args.out.mkdir(parents=True, exist_ok=True)
                (args.out / f"blnkinf-{chip}.bin").write_bytes(corpo)
                (args.out / f"immagine-{chip}.bin").write_bytes(dati)
            for riga in log.splitlines():
                if riga.startswith("Sketch uses "):
                    print("    " + riga)

        if not args.senza_bt:
            print("== Bluetooth classico (esempio ESP32_BT)")
            if "esp32c3" in scelti:
                codice, log, _ = compila(yaml, CHIP["esp32c3"][0], sketch(banco, BT, "Bt_esp32c3"))
                dico("su una C3 non compila", codice != 0)
                dico("e la libreria dice perche' (Bluetooth classico solo sull'ESP32)",
                     "needs Classic Bluetooth" in log,
                     next((r for r in log.splitlines() if "error" in r), "")[:200])
            if "esp32" in scelti:
                codice, log, _ = compila(yaml, CHIP["esp32"][0], sketch(banco, BT, "Bt_esp32"))
                dico("sull'ESP32 classico compila ancora", codice == 0,
                     log.strip().splitlines()[-1] if log.strip() else "")

    print()
    print("tutto vero" if rotte == 0 else f"{rotte} cose non tornano")
    return 0 if rotte == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

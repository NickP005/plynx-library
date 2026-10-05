#!/usr/bin/env python3
"""
La 1.3.0 eseguita sul chip, non letta nel sorgente: QEMU di Espressif fa girare
verifica/qemu_testa_chip/qemu_testa_chip.ino su una ESP32-C3 e una ESP32-S3.

🔴 PERCHE' ESISTE. Due pezzi della 1.3.0 non si vedono da un .bin:
  - plynxOtaHeadChipOk (Plynx/PlynxOta.h) rifiuta ai primi 14 byte un'immagine
    compilata per un altro chip. Senza, la scheda la scaricava e la scriveva
    tutta e il bootloader la rifiutava in fondo («mismatch chip ID»);
  - "ota-err" (PlynxApiArduino.h) dice al server, al collegamento dopo, perche'
    il download e' fallito: una volta sola, e mai a meta' (una chiave senza
    valore sposterebbe tutte le coppie che il server legge).
Qui girano sul codice compilato per quel chip: CONFIG_IDF_FIRMWARE_CHIP_ID,
Update.end() e il buffer di 160 byte veri.

Ogni chip fa da straniero all'altro: l'immagine della S3 va nella spiffs della
C3 e viceversa, quindi bastano le due compilazioni.

Nei due versi: lo sketch fa anche il giro SENZA controllo (com'era la 1.2.0) e
la prova chiede che li' l'immagine straniera passi tutta e si fermi solo in
fondo. Con PLYNX_LIB=<albero 1.2.0> lo sketch si ferma prima: non c'e'
PLYNX_OTA_CHECKS_CHIP ne' PLYNX_INFO_OTA_ERR, e le righe diventano rosse.

Uso:
  PLYNX_QEMU=<cartella di QEMU Espressif> python3 verifica/qemu_testa_chip.py [--solo esp32c3]
  ... --solo esp32c3 --straniera immagine-esp32s3.bin   (una compilazione sola)

PLYNX_QEMU e' la cartella con bin/qemu-system-riscv32 e bin/qemu-system-xtensa
(github.com/espressif/qemu, release esp_develop_9.2.2). Senza: CIECO, esce 2.

Esiti: 0 tutto vero; 1 qualcosa non torna; 2 CIECO.

⚠️ Una compilazione alla volta e QEMU con nice 19: la macchina e' condivisa.
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
import time

REPO = pathlib.Path(os.environ.get("PLYNX_LIB") or pathlib.Path(__file__).resolve().parent.parent)
SKETCH = pathlib.Path(__file__).resolve().parent / "qemu_testa_chip" / "qemu_testa_chip.ino"
DATI_ARDUINO = pathlib.Path.home() / "Library" / "Arduino15"
# FlashMode=dio: QEMU non emula l'accensione del QIO, e con il modo di serie
# l'avvio si ferma in init_flash («flash_ret == ESP_OK») e riparte all'infinito.
# Il modo della flash non tocca il codice che si prova.
CHIP = {
    "esp32c3": ("esp32:esp32:esp32c3:FlashMode=dio", "qemu-system-riscv32", ["-icount", "3"]),
    "esp32s3": ("esp32:esp32:esp32s3:FlashMode=dio", "qemu-system-xtensa", []),
}
STRANIERO = {"esp32c3": "esp32s3", "esp32s3": "esp32c3"}
SPIFFS = 0x290000          # partizioni di serie a 4 MB: spiffs 0x290000, 0x160000
SPIFFS_SIZE = 0x160000
FLASH = 4 * 1024 * 1024
ATTESA_S = 600


def cieco(perche: str) -> None:
    print(f"QUESTO CANCELLO E' CIECO: {perche}", file=sys.stderr)
    sys.exit(2)


def compila(banco: pathlib.Path, yaml: pathlib.Path, chip: str) -> tuple[bytes, bytes] | None:
    cartella = banco / f"qemu_testa_chip_{chip}"
    cartella.mkdir()
    (cartella / f"{cartella.name}.ino").write_text(SKETCH.read_text())
    build = cartella / "build"
    t0 = time.time()
    p = subprocess.run(["nice", "-n", "19", "arduino-cli", "--config-file", str(yaml), "compile",
                        "--fqbn", CHIP[chip][0], "--build-path", str(build), str(cartella)],
                       capture_output=True, text=True)
    log = (p.stdout + p.stderr).strip().splitlines()
    if p.returncode != 0:
        print(f"  ✗ {chip}: lo sketch non compila ({round(time.time() - t0)} s)")
        for riga in [r for r in log if "error" in r.lower()][:8] or log[-5:]:
            print("      " + riga)
        return None
    uso = next((r for r in log if r.startswith("Sketch uses")), "")
    print(f"  {chip}: compilato in {round(time.time() - t0)} s. {uso}")
    app = next(build.glob("*.ino.bin")).read_bytes()
    unita = next(build.glob("*.ino.merged.bin")).read_bytes()
    return app, unita


def flash(unita: bytes, straniera: bytes, propria: int) -> bytes:
    m = bytearray(unita)
    if len(m) < FLASH:
        m += b"\xff" * (FLASH - len(m))
    assert 0x1000 + len(straniera) <= SPIFFS_SIZE, "l'immagine straniera non sta nella spiffs"
    m[SPIFFS:SPIFFS + 16] = b"ALIENIMG" + struct.pack("<II", len(straniera), propria)
    m[SPIFFS + 0x1000:SPIFFS + 0x1000 + len(straniera)] = straniera
    return bytes(m)


def esegui(qemu: pathlib.Path, chip: str, immagine: pathlib.Path, log: pathlib.Path) -> str:
    binario, extra = CHIP[chip][1], CHIP[chip][2]
    p = subprocess.Popen(["nice", "-n", "19", str(qemu / "bin" / binario), "-nographic", *extra,
                          "-machine", chip, "-drive", f"file={immagine},if=mtd,format=raw",
                          "-serial", f"file:{log}", "-monitor", "none"],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    t0 = time.time()
    try:
        while time.time() - t0 < ATTESA_S and p.poll() is None:
            time.sleep(2)
            letto = log.read_text(errors="replace") if log.exists() else ""
            # Lo sketch non si riavvia mai da solo: un "Rebooting..." e' un
            # avvio che cade, e aspettare la scadenza non cambierebbe niente.
            if "BANCO fine" in letto or "Rebooting..." in letto:
                break
    finally:
        p.kill()
        p.wait()
    testo = log.read_text(errors="replace") if log.exists() else ""
    print(f"  {chip}: QEMU fermato dopo {round(time.time() - t0)} s")
    return testo


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--solo", help="esp32c3 o esp32s3 (lo straniero si compila lo stesso, se non c'e' --straniera)")
    ap.add_argument("--straniera", type=pathlib.Path,
                    help="con --solo: l'immagine di un altro chip gia' pronta (per esempio chip_dichiarato.py --out)")
    ap.add_argument("--log", type=pathlib.Path, help="cartella dove lasciare i log seriali")
    args = ap.parse_args()

    qemu = pathlib.Path(os.environ.get("PLYNX_QEMU", ""))
    if not os.environ.get("PLYNX_QEMU") or not (qemu / "bin" / "qemu-system-riscv32").exists():
        cieco("serve PLYNX_QEMU, la cartella di QEMU Espressif con bin/qemu-system-riscv32.")
    if shutil.which("arduino-cli") is None:
        cieco("arduino-cli non c'e'.")
    versione = next((r.split("=", 1)[1] for r in (REPO / "library.properties").read_text().splitlines()
                     if r.startswith("version=")), "?")
    print(f"libreria: {REPO} (versione {versione})")
    eseguiti = [args.solo] if args.solo else list(CHIP)

    rotte = 0
    with tempfile.TemporaryDirectory(prefix="plynx-qemu-") as t:
        banco = pathlib.Path(t)
        libs = banco / "user" / "libraries"
        libs.mkdir(parents=True)
        os.symlink(REPO, libs / "Plynx")
        yaml = banco / "arduino-cli.yaml"
        yaml.write_text("directories:\n"
                        f"    data: {DATI_ARDUINO}\n"
                        f"    user: {banco / 'user'}\n"
                        "    builtin:\n"
                        f"        libraries: {DATI_ARDUINO / 'libraries'}\n")
        compilati = {}
        da_compilare = list(CHIP)
        if args.solo and args.straniera:
            da_compilare = [args.solo]
            compilati[STRANIERO[args.solo]] = (args.straniera.read_bytes(), b"")
        for chip in da_compilare:
            c = compila(banco, yaml, chip)
            if c is None:
                rotte += 1
            else:
                compilati[chip] = c
        for chip in eseguiti:
            if chip not in compilati or STRANIERO[chip] not in compilati:
                print(f"  ✗ {chip}: senza le due compilazioni non si esegue")
                rotte += 1
                continue
            straniera = compilati[STRANIERO[chip]][0]
            img = banco / f"flash-{chip}.bin"
            img.write_bytes(flash(compilati[chip][1], straniera, len(compilati[chip][0])))
            log = banco / f"seriale-{chip}.log"
            testo = esegui(qemu, chip, img, log)
            if args.log:
                args.log.mkdir(parents=True, exist_ok=True)
                shutil.copy(log, args.log / log.name)
            righe = [r for r in testo.splitlines() if r.startswith("BANCO ")]
            print(f"== {chip}")
            for r in righe:
                if r.startswith("BANCO ok "):
                    print("  ✓ " + r[9:])
                elif r.startswith("BANCO ko "):
                    print("  ✗ " + r[9:])
                    rotte += 1
                else:
                    print("    " + r[6:])
            fine = next((r for r in righe if r.startswith("BANCO fine")), None)
            if fine is None:
                print("  ✗ lo sketch non e' arrivato in fondo (BANCO fine manca)")
                caduta = [r for r in testo.splitlines() if "assert failed" in r or "Guru Meditation" in r]
                print("    " + (caduta[0] if caduta else "ultime righe: " + " | ".join(testo.strip().splitlines()[-6:])))
                rotte += 1
            elif sum(1 for r in righe if r.startswith("BANCO ok ")) < 12:
                print("  ✗ meno di 12 prove verdi: lo sketch ha saltato qualcosa")
                rotte += 1
    print()
    print("tutto vero" if rotte == 0 else f"{rotte} cose non tornano")
    return 0 if rotte == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

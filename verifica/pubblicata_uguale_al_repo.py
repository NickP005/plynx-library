#!/usr/bin/env python3
"""
Quello che c'e' nel repo e quello che la gente installa sono la stessa cosa?

🔴 **PERCHE' ESISTE.** Il 03/09/2026 lo sketch Ethernet generato dall'app non
compilava per chi installava la libreria dal Library Manager: la 1.0.5
pubblicata **non dichiara `depends`**, quindi Ethernet non veniva scaricata. Nel
repo quella riga c'e' da sempre. Repo e indice portavano lo **stesso numero** e
non erano lo stesso codice, e non esisteva niente che se ne accorgesse: la prova
si faceva compilando (`Plynx/verifica/libreria_compila.sh`), che pero' misura
quello che c'e' installato su QUESTA macchina, non quello che scarica una
persona.

Il fatto che conta lo dice l'indice ufficiale di Arduino, non noi.

Uso:
  python3 verifica/pubblicata_uguale_al_repo.py [--indice FILE.json.gz]

Esiti:
  0  il repo e' avanti (versione non ancora pubblicata) e lo dice, oppure
     la versione del repo e' pubblicata e coincide;
  1  la stessa versione dice due cose diverse, oppure il repo dichiara
     dipendenze che la versione pubblicata non ha (il difetto del 03/09);
  2  CIECO: l'indice non si e' potuto leggere. Non si esce mai 0 per non aver
     guardato.

⚠️ **La taratura.** «Non l'ho trovato» e «non c'e'» sono la stessa risposta per
uno strumento che non riesce a scaricare. Qui senza rete si esce 2, e la riga lo
dice a chiare lettere: un cancello che diventa verde quando il wifi cade e' un
cancello che protegge solo quando non serve.
"""
from __future__ import annotations

import gzip
import hashlib
import io
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import zipfile

INDICE = "https://downloads.arduino.cc/libraries/library_index.json.gz"
NOME = "Plynx"
RADICE = pathlib.Path(__file__).resolve().parents[1]

# I campi di `library.properties` che cambiano cosa succede a chi installa.
# ⚠️ Non tutti: `sentence` e `paragraph` sono prosa da vetrina, e una virgola
# in piu' li' non rompe niente a nessuno. `depends` invece decide se lo sketch
# compila, ed e' il campo che ci e' costato.
CHE_CONTANO = ["name", "version", "depends", "architectures", "includes", "category"]


def cieco(perche: str) -> int:
    print("QUESTO CANCELLO E' CIECO: %s" % perche)
    print("Non esco 0: non aver guardato non e' «va tutto bene».")
    return 2


def proprieta(testo: str) -> dict:
    fuori = {}
    for riga in testo.splitlines():
        riga = riga.strip()
        if not riga or riga.startswith("#") or "=" not in riga:
            continue
        chiave, valore = riga.split("=", 1)
        fuori[chiave.strip()] = valore.strip()
    return fuori


def scarica(url: str, dove: str) -> bool:
    esito = subprocess.run(["curl", "-sS", "--max-time", "300", "-L", "-o", dove, url],
                           capture_output=True)
    return esito.returncode == 0 and os.path.getsize(dove) > 0


def main() -> int:
    locale = None
    if "--indice" in sys.argv:
        locale = sys.argv[sys.argv.index("--indice") + 1]

    mie = proprieta((RADICE / "library.properties").read_text(encoding="utf-8"))
    versione = mie.get("version")
    if not versione:
        return cieco("in library.properties non c'e' `version=`")
    print("repo: versione %s, depends=%s" % (versione, mie.get("depends") or "(nessuna)"))

    tmp = tempfile.mkdtemp(prefix="plynx-indice")
    try:
        if locale:
            gz = locale
        else:
            gz = os.path.join(tmp, "indice.json.gz")
            if not scarica(INDICE, gz):
                return cieco("non ho potuto scaricare l'indice di Arduino (rete?)")
        try:
            with gzip.open(gz) as f:
                indice = json.load(f)
        except Exception as e:                                   # noqa: BLE001
            return cieco("l'indice non si legge: %s" % e)

        pubblicate = [l for l in indice.get("libraries", []) if l.get("name") == NOME]
        if not pubblicate:
            return cieco("nell'indice di Arduino non c'e' nessuna libreria «%s»: o il "
                         "nome e' cambiato, o l'indice non e' quello giusto" % NOME)

        numeri = sorted({l["version"] for l in pubblicate},
                        key=lambda v: [int(x) for x in v.split(".")])
        print("indice: %d versioni pubblicate, l'ultima e' la %s"
              % (len(numeri), numeri[-1]))

        stessa = [l for l in pubblicate if l["version"] == versione]
        guasti = []

        if not stessa:
            # Il repo e' avanti. Non e' un guasto, ma va DETTO: e' lo stato in cui
            # la riparazione esiste e nessuno ce l'ha.
            ultima = [l for l in pubblicate if l["version"] == numeri[-1]][0]
            suo = ", ".join(ultima.get("dependencies") and
                            [d["name"] for d in ultima["dependencies"]] or []) or "(nessuna)"
            print("\n⏸ La %s NON e' ancora pubblicata: chi installa oggi prende la %s."
                  % (versione, numeri[-1]))
            print("   quella versione dichiara depends=%s" % suo)
            if (mie.get("depends") or "") and suo == "(nessuna)":
                print("   ⚠️ e il repo ne dichiara: finche' non pubblichi, chi installa "
                      "non se le vede scaricare.")
            return 0

        pubblicata = stessa[0]
        print("\nla %s e' pubblicata: la scarico e la confronto." % versione)
        zip_path = os.path.join(tmp, pubblicata.get("archiveFileName") or "lib.zip")
        if not scarica(pubblicata["url"], zip_path):
            return cieco("non ho potuto scaricare %s" % pubblicata["url"])

        # ⚠️ Si controlla la firma che l'indice dichiara: se il file che ho in mano
        # non e' quello che l'indice promette, il confronto non dice niente su
        # quello che scarica una persona.
        atteso = (pubblicata.get("checksum") or "").split(":")[-1].lower()
        visto = hashlib.sha256(open(zip_path, "rb").read()).hexdigest()
        if atteso and atteso != visto:
            return cieco("lo zip scaricato non ha la firma che l'indice dichiara "
                         "(%s invece di %s)" % (visto[:12], atteso[:12]))

        with zipfile.ZipFile(zip_path) as z:
            interni = [n for n in z.namelist() if n.endswith("library.properties")]
            if not interni:
                return cieco("dentro lo zip pubblicato non c'e' library.properties")
            sue = proprieta(z.read(sorted(interni, key=len)[0]).decode("utf-8"))

        for campo in CHE_CONTANO:
            mio = mie.get(campo, "")
            suo = sue.get(campo, "")
            if mio != suo:
                guasti.append("`%s`: nel repo «%s», nella %s pubblicata «%s»"
                              % (campo, mio or "(niente)", versione, suo or "(niente)"))

        if guasti:
            print("\nROSSO: la %s dice due cose diverse a seconda di dove la si prende."
                  % versione)
            for g in guasti:
                print("  - %s" % g)
            print("\nUna versione gia' pubblicata non si corregge: si alza il numero e si "
                  "pubblica. Chi ha installato la %s ha quello che c'e' nell'indice, non "
                  "quello che c'e' qui." % versione)
            return 1

        print("\nVERDE: la %s pubblicata e il repo dicono le stesse cose sui campi che "
              "cambiano cosa succede a chi installa (%s)."
              % (versione, ", ".join(CHE_CONTANO)))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())


# ─────────────────────────────────────────────────────────────────────────────
# LA CODA: come si prova questo cancello, nei tre esiti.
#
#   # A) VERDE/⏸: com'e' adesso (repo 1.0.6, indice fermo alla 1.0.5)
#   python3 verifica/pubblicata_uguale_al_repo.py
#
#   # B) ROSSO: si finge che il repo sia gia' alla 1.0.5, che e' pubblicata
#   #    SENZA depends mentre il repo le dichiara -> deve dire che divergono
#   sed -i '' 's/^version=1.0.6/version=1.0.5/' library.properties
#   python3 verifica/pubblicata_uguale_al_repo.py     # esito 1
#   sed -i '' 's/^version=1.0.5/version=1.0.6/' library.properties
#
#   # C) CIECO: senza rete (o con un indice illeggibile) -> esito 2, mai 0
#   python3 verifica/pubblicata_uguale_al_repo.py --indice /dev/null
#
# 🪤 **Quello che questo cancello NON guarda.** Il contenuto di `src/`: due
# versioni possono dichiarare gli stessi campi e portare codice diverso. Si
# guardano i campi perche' sono quelli che decidono se lo sketch COMPILA, ed e'
# li' che ci siamo fatti male. Chi vuole il confronto dei sorgenti lo aggiunga
# qui, non in un secondo script.

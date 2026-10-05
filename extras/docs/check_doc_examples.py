#!/usr/bin/env python3
"""
Compile every example written in the documentation comments of the library.

Every @code ... @endcode block inside a /** ... */ comment under src/ is a
small sketch. People copy these blocks into their own sketches, and the Plynx
app shows them, so a block that does not compile teaches a mistake. This
script turns each block into a real sketch and compiles it with arduino-cli
against THIS copy of the library (a link to this folder is the only Plynx
library arduino-cli can see). A block that does not compile makes it fail,
and the output names the header file and the line of the block.

The rules a block follows are in CONTRIBUTING.md. In short:
  - top-level code only: globals, functions, handlers, setup(), loop();
  - #define and #include lines, when present, come first;
  - a block that includes no Plynx board header is compiled for ESP32 (with
    <PlynxSimpleEsp32.h>) and for AVR, on an Arduino Mega (with
    <PlynxSimpleStream.h>);
  - a block that includes a board header is compiled for that board only;
  - empty setup() and loop() are added when the block has none.

To keep the run short, blocks that share the same first lines are compiled
together, each inside its own namespace. When such a group fails it is split
in two and each half is compiled again, down to single blocks, which are
compiled on their own as a normal sketch. Only a block that fails alone counts
as broken. --each skips the grouping.

Usage:
  python3 extras/docs/check_doc_examples.py [--list] [--each]
          [--only TEXT] [--config-file FILE] [--lib DIR ...] [--keep DIR]

  --list         print the blocks and their boards, compile nothing
  --each         compile every block on its own (slower, no grouping)
  --only TEXT    only the blocks whose "file:line" contains TEXT
  --config-file  the arduino-cli configuration to take the cores from
  --lib DIR      an extra library the examples need (repeatable)
  --keep DIR     keep the generated sketches and build folders in DIR, and
                 reuse them on the next run (faster)

Exit status: 0 every block compiled; 1 at least one block does not compile
or breaks the rules; 2 BLIND (arduino-cli, a core or a block's board is
missing, or no block was found). It never exits 0 without having compiled
every block it found.
"""
from __future__ import annotations

import argparse
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field

REPO = pathlib.Path(__file__).resolve().parents[2]
SRC = REPO / "src"

# Where each target is compiled, and the board header a block without one gets.
TARGETS = {
    "esp32": ("esp32:esp32:esp32", "esp32:esp32", "PlynxSimpleEsp32.h"),
    "avr": ("arduino:avr:mega", "arduino:avr", "PlynxSimpleStream.h"),
    "esp8266": ("esp8266:esp8266:nodemcuv2", "esp8266:esp8266", "PlynxSimpleEsp8266.h"),
}
DEFAULT_TARGETS = ("esp32", "avr")

# A block that includes one of these headers is compiled for those targets only.
BOARD_HEADERS = {
    "PlynxSimpleEsp32.h": ("esp32",),
    "PlynxSimpleEsp32_SSL.h": ("esp32",),
    "PlynxSimpleEsp32_BLE.h": ("esp32",),
    "PlynxSimpleEsp32_NimBLE.h": ("esp32",),
    "PlynxSimpleEsp32_BT.h": ("esp32",),
    "PlynxSimpleEsp8266.h": ("esp8266",),
    "PlynxSimpleEsp8266_SSL.h": ("esp8266",),
    "PlynxSimpleStream.h": ("avr",),
    "PlynxSimpleEthernet.h": ("avr",),
    "PlynxSimpleSerialBLE.h": ("avr",),
}

# How many blocks share one sketch. AVR has 8 KB of RAM on a Mega, and a
# group whose globals do not fit would fail for a reason no block has.
GROUP_SIZE = {"esp32": 40, "avr": 6, "esp8266": 20}

INCLUDE_BOARD = re.compile(r'^\s*#\s*include\s*[<"](PlynxSimple\w+\.h)[>"]')
HAS_SETUP = re.compile(r"\bvoid\s+setup\s*\(")
HAS_LOOP = re.compile(r"\bvoid\s+loop\s*\(")
DOC_COMMENT = re.compile(r"/\*\*(?!/)(.*?)\*/", re.S)
STAR = re.compile(r"^\s*\*? ?")


@dataclass
class Block:
    file: str            # path relative to the repository
    line: int            # line of the @code tag in the file
    first: int           # line of the first line of code in the file
    code: list[str]
    defines: list[str] = field(default_factory=list)
    includes: list[str] = field(default_factory=list)
    body: list[str] = field(default_factory=list)
    body_first: int = 0  # line in the file of body[0]
    board: str | None = None
    problem: str | None = None

    @property
    def name(self) -> str:
        return f"{self.file}:{self.line}"

    def targets(self) -> tuple[str, ...]:
        if self.board is None:
            return DEFAULT_TARGETS
        return BOARD_HEADERS.get(self.board, ())


def find_blocks() -> list[Block]:
    blocks = []
    for path in sorted(SRC.rglob("*.h")):
        text = path.read_text(encoding="utf-8", errors="replace")
        rel = str(path.relative_to(REPO))
        for comment in DOC_COMMENT.finditer(text):
            start_line = text.count("\n", 0, comment.start()) + 1
            lines = comment.group(1).split("\n")
            inside = None
            for i, raw in enumerate(lines):
                line = STAR.sub("", raw, count=1) if i else raw
                tag = line.strip()
                if inside is None and re.match(r"^[@\\]code(\{[^}]*\})?$", tag):
                    inside = (start_line + i, [])
                elif inside is not None and re.match(r"^[@\\]endcode$", tag):
                    at, code = inside
                    blocks.append(Block(rel, at, at + 1, code))
                    inside = None
                elif inside is not None:
                    inside[1].append(line.rstrip())
            if inside is not None:
                blocks.append(Block(rel, inside[0], inside[0] + 1, inside[1],
                                    problem="@code without @endcode"))
    for b in blocks:
        split(b)
    return blocks


def split(b: Block) -> None:
    """The leading # lines are the preamble, the rest is the body."""
    i = 0
    while i < len(b.code):
        s = b.code[i].strip()
        if s.startswith("#define") or s.startswith("# define"):
            b.defines.append(b.code[i])
        elif s.startswith("#include") or s.startswith("# include"):
            b.includes.append(b.code[i])
            m = INCLUDE_BOARD.match(b.code[i])
            if m:
                b.board = m.group(1)
        elif s == "" or s.startswith("//"):
            pass
        else:
            break
        i += 1
    b.body = b.code[i:]
    b.body_first = b.first + i
    for s in b.body:
        if s.strip().startswith("#include"):
            b.problem = "an #include after the first line of code (see CONTRIBUTING.md)"
    if b.board is not None and b.board not in BOARD_HEADERS:
        b.problem = None  # reported as BLIND, not as a broken block


def preamble(b: Block, target: str) -> list[str]:
    """What goes above the body: the block's defines, the board header (the
    block's own, or the default for the target), then the other includes."""
    head = list(b.defines)
    if b.board is None:
        head.append(f"#include <{TARGETS[target][2]}>")
    head += b.includes
    return head


def solo_sketch(b: Block, target: str) -> tuple[str, int]:
    """The block as a sketch of its own, and the .ino line of body[0]."""
    head = preamble(b, target)
    lines = head + [""] + b.body
    text = "\n".join(lines)
    if not HAS_SETUP.search(text):
        lines += ["", "void setup() {}"]
    if not HAS_LOOP.search(text):
        lines += ["", "void loop() {}"]
    return "\n".join(lines) + "\n", len(head) + 2


def group_sketch(blocks: list[Block], target: str) -> str:
    lines = preamble(blocks[0], target) + [""]
    for n, b in enumerate(blocks):
        lines.append(f"namespace plynx_doc_example_{n} {{  // {b.name}")
        lines += b.body
        lines.append(f"}}  // {b.name}")
        lines.append("")
    lines += ["void setup() {}", "void loop() {}"]
    return "\n".join(lines) + "\n"


class Compiler:
    def __init__(self, cfg: pathlib.Path, work: pathlib.Path):
        self.cfg = cfg
        self.work = work
        self.count = 0
        self.seconds = 0.0

    def compile(self, target: str, sketch: str) -> tuple[bool, str]:
        folder = self.work / target / "DocExample"
        folder.mkdir(parents=True, exist_ok=True)
        (folder / "DocExample.ino").write_text(sketch)
        fqbn = TARGETS[target][0]
        t0 = time.monotonic()
        p = subprocess.run(
            ["arduino-cli", "--config-file", str(self.cfg), "compile", "--fqbn", fqbn,
             "--build-path", str(self.work / target / "build"), "--warnings", "default",
             str(folder)],
            capture_output=True, text=True)
        self.count += 1
        self.seconds += time.monotonic() - t0
        return p.returncode == 0, p.stdout + p.stderr


def where_is_arduino_data(user_cfg: list[str]) -> tuple[pathlib.Path, pathlib.Path | None]:
    def get(key: str) -> str:
        p = subprocess.run(["arduino-cli", *user_cfg, "config", "get", key],
                           capture_output=True, text=True)
        return p.stdout.strip() if p.returncode == 0 else ""
    data = get("directories.data") or str(pathlib.Path.home() / "Library" / "Arduino15")
    user = get("directories.user")
    return pathlib.Path(data), (pathlib.Path(user) if user else None)


def library_name(folder: pathlib.Path) -> str:
    props = folder / "library.properties"
    if props.is_file():
        for row in props.read_text(errors="replace").splitlines():
            if row.startswith("name="):
                return row[5:].strip()
    return folder.name


def private_config(work: pathlib.Path, data: pathlib.Path, sketchbook: pathlib.Path | None,
                   extra: list[pathlib.Path]) -> pathlib.Path:
    """A configuration where this tree is the only Plynx library. The other
    libraries of the sketchbook stay visible, because examples may need them
    (TimeLib for WidgetRTC, NimBLE-Arduino for the NimBLE shell)."""
    libs = work / "user" / "libraries"
    libs.mkdir(parents=True)
    os.symlink(REPO, libs / "Plynx")
    seen = {"Plynx"}
    for lib in extra:
        if lib.name not in seen:
            os.symlink(lib.resolve(), libs / lib.name)
            seen.add(lib.name)
    if sketchbook is not None and (sketchbook / "libraries").is_dir():
        for lib in sorted((sketchbook / "libraries").iterdir()):
            if not lib.is_dir() or lib.name in seen:
                continue
            if library_name(lib).lower().startswith("plynx"):
                continue
            os.symlink(lib.resolve(), libs / lib.name)
            seen.add(lib.name)
    cfg = work / "arduino-cli.yaml"
    cfg.write_text(
        "directories:\n"
        f"    data: {data}\n"
        f"    user: {work / 'user'}\n"
        "    builtin:\n"
        f"        libraries: {data / 'libraries'}\n")
    return cfg


def errors_of(log: str, b: Block | None, ino_body_line: int) -> list[str]:
    """The error lines of a compile log, with the sketch line turned into the
    line of the header the block comes from."""
    out = []
    for row in log.splitlines():
        if "error" not in row.lower():
            continue
        m = re.search(r"(\S*/)?DocExample\.ino:(\d+):(\d+:)?", row)
        if m and b is not None:
            n = int(m.group(2))
            if n >= ino_body_line:
                row = row.replace(m.group(0), f"{b.file}:{b.body_first + n - ino_body_line}:")
        out.append(row.strip())
    return out[:6]


def main() -> int:
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--each", action="store_true")
    ap.add_argument("--only")
    ap.add_argument("--config-file", type=pathlib.Path)
    ap.add_argument("--lib", type=pathlib.Path, action="append", default=[])
    ap.add_argument("--keep", type=pathlib.Path)
    args = ap.parse_args()

    blocks = find_blocks()
    if args.only:
        blocks = [b for b in blocks if args.only in b.name]
    if not blocks:
        print("BLIND: no @code block found", file=sys.stderr)
        return 2

    if args.list:
        for b in blocks:
            print(f"{b.name}  {','.join(b.targets()) or 'BLIND (' + str(b.board) + ')'}"
                  + (f"  PROBLEM: {b.problem}" if b.problem else ""))
        print(f"{len(blocks)} blocks")
        return 0

    if shutil.which("arduino-cli") is None:
        print("BLIND: arduino-cli is not installed", file=sys.stderr)
        return 2
    user_cfg = ["--config-file", str(args.config_file)] if args.config_file else []
    data, sketchbook = where_is_arduino_data(user_cfg)
    cores = subprocess.run(["arduino-cli", *user_cfg, "core", "list"],
                           capture_output=True, text=True).stdout

    broken: list[tuple[str, str, list[str]]] = []
    blind: list[str] = []
    passed = 0

    with tempfile.TemporaryDirectory(prefix="plynx-doc-examples-") as tmp:
        work = args.keep or pathlib.Path(tmp)
        if args.keep:
            # The build folders stay, so a second run reuses the compiled core
            # and library. The library links are made again every time.
            work.mkdir(parents=True, exist_ok=True)
            shutil.rmtree(work / "user", ignore_errors=True)
        cfg = private_config(work, data, sketchbook, args.lib)
        cc = Compiler(cfg, work)

        jobs: dict[str, list[Block]] = {}
        for b in blocks:
            if b.problem:
                broken.append((b.name, "rules", [b.problem]))
                continue
            targets = b.targets()
            if not targets:
                blind.append(f"{b.name} includes {b.board}, a board this script does not compile")
                continue
            for t in targets:
                if TARGETS[t][1] not in cores:
                    blind.append(f"{b.name} needs the core {TARGETS[t][1]}")
                    continue
                jobs.setdefault(t, []).append(b)

        for target, todo in jobs.items():
            print(f"== {target} ({TARGETS[target][0]}): {len(todo)} blocks")
            groups: dict[tuple, list[Block]] = {}
            for b in todo:
                alone = args.each or any(s.strip().startswith("#") for s in b.body)
                key = (b.name,) if alone else tuple(preamble(b, target))
                groups.setdefault(key, []).append(b)
            def check(chunk: list[Block]) -> int:
                """Compiles a list of blocks, splitting it while it fails.
                Returns how many blocks passed."""
                if len(chunk) > 1:
                    ok, _ = cc.compile(target, group_sketch(chunk, target))
                    if ok:
                        for b in chunk:
                            print(f"  ok    {b.name}")
                        return len(chunk)
                    print(f"  a group of {len(chunk)} did not compile, splitting it")
                    half = len(chunk) // 2
                    return check(chunk[:half]) + check(chunk[half:])
                b = chunk[0]
                sketch, body_line = solo_sketch(b, target)
                ok, log = cc.compile(target, sketch)
                if ok:
                    print(f"  ok    {b.name}")
                    return 1
                errs = errors_of(log, b, body_line) or log.strip().splitlines()[-3:]
                broken.append((b.name, target, errs))
                print(f"  FAIL  {b.name}")
                for e in errs:
                    print(f"          {e}")
                return 0

            for key, members in groups.items():
                for i in range(0, len(members), GROUP_SIZE[target]):
                    passed += check(members[i:i + GROUP_SIZE[target]])

        print()
        print(f"{len(blocks)} blocks, {passed} compilations passed, "
              f"{cc.count} runs of arduino-cli in {cc.seconds:.0f} s")

    for name, target, errs in broken:
        print(f"BROKEN {name} ({target})")
        for e in errs:
            print(f"    {e}")
    for why in blind:
        print(f"BLIND  {why}")
    if broken:
        return 1
    if blind:
        return 2
    print("every example compiles")
    return 0


if __name__ == "__main__":
    sys.exit(main())

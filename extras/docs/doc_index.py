#!/usr/bin/env python3
"""
Turn the Doxygen XML of the library into one JSON file, one entry per
documented symbol, for programs that show the documentation (the Plynx app
reads it through the Plynx server).

    doxygen Doxyfile
    python3 extras/docs/doc_index.py build/doxygen-xml > plynx-docs.json
    python3 extras/docs/doc_index.py --check build/doxygen-xml

--check writes no JSON. It fails when a documented function or macro leaves
a parameter without @param, or a function that returns a value without
@return. Doxygen itself only catches a wrong @param, or one missing among
others, unless it is told to warn about every undocumented internal too.

Each entry keeps the sections of the comment apart, so a reader can show them
in its own layout:

    name        the word a person selects in a sketch: "virtualWrite",
                "PLYNX_WRITE", "WidgetTerminal", "flush", "V0"
    qualified   how a sketch writes it: "Plynx.virtualWrite",
                "WidgetTerminal::flush", "PlynxTimer::setInterval"
    kind        function, define, class, struct, typedef, variable, enum
    signature   the declaration, as written in the header
    file, line  where it is declared, relative to src/
    brief, details, params [{name, text}], returns, where (the "Where to
    call" paragraph), widget (the "App widget" paragraph), code [text],
    notes, warnings, see, deprecated, experimental

The functions of the Plynx object live in several classes (PlynxApi,
PlynxProtocol and one class per board). They all get the qualified name
"Plynx.<name>", which is how a sketch calls them.

The text is the English of the comments. Translations are not made here.

A member with a comment but no @brief (some internals use plain /// lines) is
left out and named on stderr.

Exit status: 0 written (or, with --check, every documented symbol complete);
1 with --check, something is missing; 2 the folder is not a Doxygen XML
output.
"""
from __future__ import annotations

import json
import pathlib
import re
import sys
import xml.etree.ElementTree as ET

# Classes whose members a sketch calls on the global object `Plynx`.
PLYNX_OBJECT = re.compile(
    r"^(PlynxApi|PlynxProtocol|PlynxWifi|PlynxWifiCommon|PlynxEthernet|PlynxStream|"
    r"PlynxEsp32_BLE|PlynxEsp32_NimBLE|PlynxEsp32_BT|PlynxCC3000|PlynxWildFire|PlynxWiFly|"
    r"PlynxSIM|PlynxLinkItOneWifi|PlynxFishino|PlynxYun|PlynxCurieBLE|PlynxSimpleRFduinoBLE|"
    r"PlynxRedBearLab_BLE_Nano|PlynxRedBearDuoBLE|PlynxSimpleSimbleeBLE|PlynxWioTerminal_BLE|"
    r"PlynxUserDefined|PlynxParticle)$")

SKIP = {"parameterlist", "simplesect", "programlisting", "xrefsect"}


def text_of(el, skip=SKIP) -> str:
    """The readable text of an element, without the nested sections."""
    if el is None:
        return ""
    out = []

    def walk(e):
        if e.tag in skip:
            if e.tail:
                out.append(e.tail)
            return
        if e.tag == "sp":
            out.append(" ")
        elif e.tag == "linebreak":
            out.append("\n")
        elif e.text:
            out.append(e.text)
        for c in e:
            walk(c)
        if e.tag == "para":
            out.append("\n\n")
        if e.tail:
            out.append(e.tail)

    if el.text:
        out.append(el.text)
    for c in el:
        walk(c)
    text = "".join(out)
    text = re.sub(r"[ \t]+", " ", text)
    text = re.sub(r" *\n *", "\n", text)
    text = re.sub(r"\n{3,}", "\n\n", text)
    return text.strip()


def code_of(listing) -> str:
    lines = []
    for codeline in listing.iter("codeline"):
        parts = []

        def walk(e):
            if e.tag == "sp":
                parts.append(" ")
            elif e.text:
                parts.append(e.text)
            for c in e:
                walk(c)
                if c.tail:
                    parts.append(c.tail)

        walk(codeline)
        lines.append("".join(parts).rstrip())
    return "\n".join(lines).strip("\n")


def sections(detailed) -> dict:
    s = {"details": "", "params": [], "returns": "", "where": "", "widget": "",
         "code": [], "notes": [], "warnings": [], "see": [], "deprecated": "",
         "experimental": False}
    if detailed is None:
        return s
    s["details"] = text_of(detailed)
    for pl in detailed.iter("parameterlist"):
        if pl.attrib.get("kind") != "param":
            continue
        for item in pl.iter("parameteritem"):
            names = [text_of(n, set()) for n in item.iter("parametername")]
            desc = text_of(item.find("parameterdescription"))
            for n in names:
                s["params"].append({"name": n, "text": desc})
    for ss in detailed.iter("simplesect"):
        kind = ss.attrib.get("kind")
        body = text_of(ss)
        if kind == "return":
            s["returns"] = body
        elif kind == "note":
            s["notes"].append(body)
        elif kind == "warning":
            s["warnings"].append(body)
        elif kind == "see":
            s["see"] += [x.strip() for x in re.split(r",\s*", body) if x.strip()]
        elif kind == "par":
            title = text_of(ss.find("title"), set()).strip().lower()
            ss_body = text_of(ss, SKIP | {"title"})
            if title == "where to call":
                s["where"] = ss_body
            elif title == "app widget":
                s["widget"] = ss_body
            else:
                s["notes"].append(ss_body)
    for listing in detailed.iter("programlisting"):
        s["code"].append(code_of(listing))
    for x in detailed.iter("xrefsect"):
        title = text_of(x.find("xreftitle"), set()).lower()
        if "deprecated" in title:
            s["deprecated"] = text_of(x.find("xrefdescription")) or "Deprecated."
        if "experimental" in title:
            s["experimental"] = True
    return s


def qualified(owner: str | None, name: str) -> str:
    if owner is None:
        return name
    if PLYNX_OBJECT.match(owner):
        return f"Plynx.{name}"
    return f"{owner}::{name}"


def object_definition(m, owner) -> bool:
    """True for a global object built with arguments, such as
    `PlynxWifi Plynx(_plynxTransport);`. Doxygen reads it as a function whose
    parameters have a type and no name."""
    params = m.findall("param")
    return (owner is None and m.attrib["kind"] == "function" and bool(params)
            and all(p.find("declname") is None for p in params))


def missing(m, s) -> list[str]:
    """What a documented function or macro leaves out: parameters without
    @param, and the return value without @return. Private members and
    template parameters are not asked for."""
    out = []
    if m.attrib.get("prot") == "private" or s["kind"] != m.attrib["kind"]:
        return out
    documented = {p["name"] for p in s["params"]}
    kind = m.attrib["kind"]
    if kind == "define":
        declared = [p.findtext("defname") for p in m.findall("param")]
    elif kind == "function":
        declared = [p.findtext("declname") for p in m.findall("param")]
    else:
        return out
    for name in declared:
        if name and name != "..." and name not in documented:
            out.append(f"@param {name}")
    if kind == "function":
        rtype = re.sub(r"\s+", " ", text_of(m.find("type"), set())).strip()
        rtype = rtype.replace("PLYNX_DEPRECATED", "").replace("virtual", "").replace("static", "").strip()
        if rtype and rtype != "void" and not s["returns"]:
            out.append("@return")
    return out


def main() -> int:
    args = [a for a in sys.argv[1:] if a != "--check"]
    check = "--check" in sys.argv[1:]
    if len(args) != 1:
        print(__doc__, file=sys.stderr)
        return 2
    folder = pathlib.Path(args[0])
    index = folder / "index.xml"
    if not index.is_file():
        print(f"{folder} is not a Doxygen XML output (no index.xml)", file=sys.stderr)
        return 2

    version = ""
    doxyfile = folder / "Doxyfile.xml"
    if doxyfile.is_file():
        for opt in ET.parse(doxyfile).getroot().iter("option"):
            if opt.attrib.get("id") == "PROJECT_NUMBER":
                version = "".join(v.text or "" for v in opt.iter("value"))

    entries = []
    broken = []
    incomplete = []
    seen = set()
    for f in sorted(folder.glob("*.xml")):
        if f.name in ("index.xml", "Doxyfile.xml"):
            continue
        for cd in ET.parse(f).getroot().iter("compounddef"):
            ckind = cd.attrib["kind"]
            cname = cd.findtext("compoundname")
            owner = cname if ckind in ("class", "struct") else None
            brief = text_of(cd.find("briefdescription"))
            loc = cd.find("location")
            if owner and brief and loc is not None:
                e = {"name": cname.split("::")[-1], "qualified": cname, "kind": ckind,
                     "signature": f"{ckind} {cname}", "file": loc.attrib.get("file"),
                     "line": int(loc.attrib.get("line", 0)), "brief": brief}
                e.update(sections(cd.find("detaileddescription")))
                entries.append(e)
            for m in cd.iter("memberdef"):
                mbrief = text_of(m.find("briefdescription"))
                mdetail = m.find("detaileddescription")
                if not mbrief:
                    if text_of(mdetail):
                        broken.append(f"{cname}::{m.findtext('name')}")
                    continue
                mloc = m.find("location")
                key = (m.attrib.get("id"), mloc.attrib.get("file"), mloc.attrib.get("line"))
                if key in seen:
                    continue
                seen.add(key)
                name = m.findtext("name")
                kind = m.attrib["kind"]
                if object_definition(m, owner):
                    kind = "variable"
                    args = ", ".join(p.findtext("type") or "" for p in m.findall("param"))
                    sig = f"{text_of(m.find('type'), set())} {name}({args})"
                elif kind == "define":
                    params = [p.findtext("defname") or "" for p in m.iter("param")]
                    sig = f"#define {name}" + (f"({', '.join(params)})" if m.find("param") is not None else "")
                else:
                    sig = " ".join(x for x in [m.findtext("type") or "", name + (m.findtext("argsstring") or "")] if x)
                e = {"name": name, "qualified": qualified(owner, name), "kind": kind,
                     "signature": re.sub(r"\s+", " ", sig).strip(),
                     "file": mloc.attrib.get("file"), "line": int(mloc.attrib.get("line", 0)),
                     "brief": mbrief}
                e.update(sections(mdetail))
                entries.append(e)
                for what in missing(m, e):
                    incomplete.append(f"{e['file']}:{e['line']}: {e['qualified']} has no {what}")

    if check:
        for row in sorted(set(incomplete)):
            print(row)
        for b in broken:
            print(f"note: left out, a comment without @brief: {b}")
        print(f"{len(entries)} documented symbols, {len(set(incomplete))} incomplete")
        return 1 if incomplete else 0

    entries.sort(key=lambda e: (e["file"] or "", e["line"]))
    json.dump({"library": "Plynx", "version": version, "language": "en",
               "source": "Doxygen comments in the headers of the library",
               "symbols": entries}, sys.stdout, indent=1, ensure_ascii=False)
    sys.stdout.write("\n")
    for b in broken:
        print(f"left out, a comment without @brief: {b}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())

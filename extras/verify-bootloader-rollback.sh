#!/bin/bash
#
# Verifies, by compiling, that the ESP32 bootloader-rollback guard in
# Plynx/PlynxOta.h cannot come apart.
#
# The rule the code exists to respect: an image is left on probation
# (PENDING_VERIFY) only if the code that will confirm it is in the same build
# and already running. Two earlier designs broke that rule in ways no test
# could see -- one needed a particular #include order, the other changed
# behaviour when a source file was renamed. Hence this script: it builds the
# exact configurations that used to break, and reads the ELF instead of
# trusting the source.
#
# It never touches the working tree: the library is copied to a temporary
# directory first. Needs arduino-cli plus the esp32 (and, for the untouched-
# ESP8266 check, esp8266) cores.
#
# Usage:  extras/verify-bootloader-rollback.sh [workdir]
# Exit:   0 = every expectation met.
#
set -u

LIB_SRC="$(cd "$(dirname "$0")/.." && pwd)"
W="${1:-${TMPDIR:-/tmp}/plynx-rollback-check}"
FQBN_ESP32=esp32:esp32:esp32
FQBN_C3=esp32:esp32:esp32c3
FQBN_8266=esp8266:esp8266:nodemcuv2
AUTH='"0123456789abcdef0123456789abcdef"'

fails=0
note()   { printf '   %s\n' "$*"; }
ok()     { printf '\033[32m  PASS\033[0m %s\n' "$*"; }
bad()    { printf '\033[31m  FAIL\033[0m %s\n' "$*"; fails=$((fails+1)); }

command -v arduino-cli >/dev/null || { echo "arduino-cli not found"; exit 2; }

rm -rf "$W"; mkdir -p "$W/libs" "$W/sk"
cp -R "$LIB_SRC" "$W/libs/Plynx"
rm -rf "$W/libs/Plynx/.git"
echo "library copy: $W/libs/Plynx"

# The toolchain nm, whatever its version directory is called.
NM=$(find "$HOME/Library/Arduino15/packages/esp32/tools" \
        -name 'xtensa-esp32-elf-nm' 2>/dev/null | head -1)
[ -z "$NM" ] && NM=$(command -v xtensa-esp32-elf-nm)

# ---------------------------------------------------------------- sketches --
sketch() {   # sketch <name> <body-file-contents-on-stdin>
    mkdir -p "$W/sk/$1"; cat > "$W/sk/$1/$1.ino"
}

sketch plain <<EOF
#define PLYNX_PRINT Serial
#include <PlynxSimpleEsp32.h>
void setup() { Serial.begin(115200); Plynx.begin($AUTH, "ssid", "pass"); }
void loop()  { Plynx.run(); }
EOF

# The documented opt-out: needed next to ESP RainMaker, which defines the same
# hook. Must leave the core's weak symbol alone and build no guard at all.
sketch optout <<EOF
#define PLYNX_PRINT Serial
#define PLYNX_NO_BOOTLOADER_ROLLBACK
#include <PlynxSimpleEsp32.h>
void setup() { Serial.begin(115200); Plynx.begin($AUTH, "ssid", "pass"); }
void loop()  { Plynx.run(); }
EOF

# Wrong include order: PlynxProtocol.h enters the translation unit before
# PLYNX_OTA_HAS_DEFAULT exists, so the protocol has no OTA dispatch. This used
# to compile clean and produce a firmware armed for rollback that nothing could
# confirm. It must now be a compile error.
sketch badorder <<EOF
#define PLYNX_PRINT Serial
#define PLYNX_OTA_HOST "fw.example.com"
#include <WiFi.h>
#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <PlynxSimpleEsp32.h>
void setup() { Serial.begin(115200); Plynx.begin($AUTH, "ssid", "pass"); }
void loop()  { Plynx.run(); }
EOF

# Two translation units instantiating the same template with different macro
# state: a textbook ODR violation that neither compiler nor linker reports. The
# linker keeps one COMDAT body, chosen by link order -- which is why the same
# sources are built twice below under two different file names.
#
# The names matter and are not decorative. Link order is the object file order,
# and in ASCII every capital letter comes before every lowercase one: "Comms"
# and "Zomms" BOTH sort before "twofile...ino", so both would land on the same
# side and a test that only compares them proves nothing. "zzz" sorts after.
# An earlier version of this script used Comms/Zomms and passed while both
# builds had lost the OTA dispatch entirely.
for second in Comms zzz; do
    sketch "twofile$second" <<EOF
#define PLYNX_PRINT Serial
#include <PlynxSimpleEsp32.h>
void sendTemp();
void setup() { Serial.begin(115200); Plynx.begin($AUTH, "ssid", "pass"); }
void loop()  { Plynx.run(); sendTemp(); }
PlynxProtocol<PlynxArduinoClient>& plynxRef() { return Plynx; }
EOF
    cat > "$W/sk/twofile$second/$second.cpp" <<'EOF'
#include <WiFi.h>
#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
extern PlynxProtocol<PlynxArduinoClient>& plynxRef();
void sendTemp() { plynxRef().run(false); (void)plynxRef().connected(); }
EOF
done

sketch esp8266 <<EOF
#define PLYNX_PRINT Serial
#include <PlynxSimpleEsp8266.h>
void setup() { Serial.begin(115200); Plynx.begin($AUTH, "ssid", "pass"); }
void loop()  { Plynx.run(); }
EOF

# ------------------------------------------------------------------ build ---
build() {    # build <name> <fqbn> <sketchdir> [extra-cpp-flags]
    rm -rf "$W/build_$1"
    if [ -n "${4:-}" ]; then
        arduino-cli compile -b "$2" --warnings all --libraries "$W/libs" \
            --build-property "compiler.cpp.extra_flags=$4" \
            --build-path "$W/build_$1" "$3" > "$W/log_$1.txt" 2>&1
    else
        arduino-cli compile -b "$2" --warnings all --libraries "$W/libs" \
            --build-path "$W/build_$1" "$3" > "$W/log_$1.txt" 2>&1
    fi
}

syms() {     # syms <name> -> "<type> <name>" lines for the symbols that matter
    local elf
    elf=$(ls "$W/build_$1"/*.elf 2>/dev/null | head -1)
    [ -z "$elf" ] && return
    "$NM" "$elf" | grep -E \
        'verifyRollbackLater|plynxOtaGuardTask|PlynxOtaAuthOk|PlynxOtaDispatch' \
        | awk '{print $2, $3}' | sort -k2
}

has() { syms "$1" | grep -q "$2"; }

echo
echo "=== 1. default build: armed, and the guard that confirms is in it ==="
if build plain "$FQBN_ESP32" "$W/sk/plain"; then
    syms plain | sed 's/^/   /'
    has plain '^T verifyRollbackLater' \
        && ok "verifyRollbackLater is a STRONG symbol (beats the core's weak one)" \
        || bad "verifyRollbackLater is not strong: the rollback is NOT armed"
    has plain 'plynxOtaGuardTask' \
        && ok "the guard task is in the binary" \
        || bad "armed with no guard: this is the silent-brick configuration"
else
    bad "default build failed"; grep -m3 error "$W/log_plain.txt" | sed 's/^/   /'
fi

echo
echo "=== 2. PLYNX_NO_BOOTLOADER_ROLLBACK: both halves off together ==="
if build optout "$FQBN_ESP32" "$W/sk/optout"; then
    syms optout | sed 's/^/   /'
    has optout '^W verifyRollbackLater' \
        && ok "the core's weak symbol is left alone" \
        || bad "the opt-out still overrides the hook"
    has optout 'plynxOtaGuardTask' \
        && bad "guard task built although the opt-out is on" \
        || ok "no guard task, nothing to confirm: consistent"
else
    bad "opt-out build failed"; grep -m3 error "$W/log_optout.txt" | sed 's/^/   /'
fi

echo
echo "=== 3. PlynxProtocol.h included too early: must be a compile ERROR ==="
if build badorder "$FQBN_ESP32" "$W/sk/badorder"; then
    bad "it compiled: an OTA nothing dispatches to would ship silently"
else
    grep -m1 'OTA dispatch is missing' "$W/log_badorder.txt" >/dev/null \
        && ok "refused at compile time, with the reason" \
        || bad "it failed, but not on the interlock -- check $W/log_badorder.txt"
    note "$(grep -m1 'error:' "$W/log_badorder.txt" | cut -c1-110)"
fi

echo
echo "=== 4. two translation units, ODR split -- and a rename must change nothing ==="
# Compare against the one-file build, not just the two against each other: two
# equally broken builds match each other perfectly. An earlier version of this
# script did exactly that and called it a pass while BOTH had lost the OTA
# dispatch.
for s in twofileComms twofilezzz; do
    if build "$s" "$FQBN_ESP32" "$W/sk/$s"; then
        has "$s" 'plynxOtaGuardTask' \
            && ok "$s: the guard survives the ODR split" \
            || bad "$s: armed with no guard"
        has "$s" 'PlynxOtaDispatch' \
            && ok "$s: the OTA dispatch is still linked in" \
            || bad "$s: no PlynxOtaDispatch -- this firmware ignores every update, silently"
        has "$s" 'PlynxOtaAuthOk' \
            && ok "$s: the synchronous confirm is still linked in" \
            || bad "$s: no PlynxOtaAuthOk -- the confirm is late by a poll interval"
    else
        bad "$s build failed"; grep -m3 error "$W/log_$s.txt" | sed 's/^/   /'
    fi
done
if [ "$(syms twofileComms)" = "$(syms twofilezzz)" ]; then
    ok "Comms.cpp (links first) and zzz.cpp (links last) produce the same symbols"
else
    bad "renaming the file changed the firmware -- the halves are separable again"
    diff <(syms twofileComms) <(syms twofilezzz) | sed 's/^/   /'
fi
if [ "$(syms twofileComms)" = "$(syms plain)" ]; then
    ok "and the same symbols as the ordinary one-file sketch"
else
    bad "the two-file build differs from the one-file build"
    diff <(syms plain) <(syms twofileComms) | sed 's/^/   /'
fi

# Under the opt-out there is no guard task, so PlynxOtaAuthOk() is the only
# thing that can ever confirm. The ODR split used to delete it while leaving
# the software watchdog armed and running: a healthy firmware rolled back.
sed 's/#define PLYNX_PRINT Serial/#define PLYNX_PRINT Serial\n#define PLYNX_NO_BOOTLOADER_ROLLBACK/' \
    "$W/sk/twofileComms/twofileComms.ino" > /tmp/_p.ino
mkdir -p "$W/sk/twofileOptout"
mv /tmp/_p.ino "$W/sk/twofileOptout/twofileOptout.ino"
cp "$W/sk/twofileComms/Comms.cpp" "$W/sk/twofileOptout/"
if build twofileOptout "$FQBN_ESP32" "$W/sk/twofileOptout"; then
    has twofileOptout 'PlynxOtaAuthOk' \
        && ok "opt-out + ODR split: the only confirm path is still linked in" \
        || bad "opt-out + ODR split: PlynxOtaAuthOk gone, nothing can confirm"
else
    bad "twofileOptout build failed"; grep -m3 error "$W/log_twofileOptout.txt" | sed 's/^/   /'
fi

echo
echo "=== 5. other targets: ESP32-C3 (RISC-V) and ESP8266 ==="
build c3 "$FQBN_C3" "$W/sk/plain" \
    && ok "ESP32-C3 builds" || { bad "ESP32-C3 build failed"; grep -m3 error "$W/log_c3.txt" | sed 's/^/   /'; }
if arduino-cli core list 2>/dev/null | grep -q esp8266; then
    build esp8266 "$FQBN_8266" "$W/sk/esp8266" \
        && ok "ESP8266 builds (single-slot: none of this applies there)" \
        || { bad "ESP8266 build failed"; grep -m3 error "$W/log_esp8266.txt" | sed 's/^/   /'; }
else
    note "esp8266 core not installed, skipped"
fi

echo
echo "=== 6. the library's own examples still build ==="
for e in Boards_WiFi/ESP32_Auto Boards_WiFi/ESP32_WiFi GettingStarted/PlynxBlink_ESP32; do
    n=$(basename "$e")
    build "ex_$n" "$FQBN_ESP32" "$W/libs/Plynx/examples/$e" \
        && ok "$n" || { bad "$n"; grep -m2 error "$W/log_ex_$n.txt" | sed 's/^/   /'; }
done
# Edgent refuses to build without a template id -- that is by design, so supply
# one. Worth covering: it is the only example on the SSL shell, which does NOT
# pull PlynxOta.h in, and so must show none of the new symbols.
EDGENT_DEFS='-DPLYNX_TEMPLATE_ID="TMPL0000000" -DPLYNX_DEVICE_NAME="check" -DPLYNX_FIRMWARE_VERSION="0.0.0"'
if build ex_Edgent "$FQBN_ESP32" "$W/libs/Plynx/examples/Plynx.Edgent/Edgent_ESP32" "$EDGENT_DEFS"; then
    # What must be true here: we did NOT override the core hook (so the symbol
    # is the core's own weak one, W and not T) and no guard task was built.
    # PlynxOtaAuthOk/PlynxOtaDispatch DO appear, as W: they are the weak no-ops
    # from PlynxProtocol.h, which is exactly what a shell without the handler
    # is supposed to link against.
    if has ex_Edgent '^T verifyRollbackLater' || has ex_Edgent 'plynxOtaGuardTask'; then
        bad "Edgent_ESP32: the SSL shell picked up the rollback code it should not have"
        syms ex_Edgent | sed 's/^/   /'
    else
        ok "Edgent_ESP32 (SSL shell: core hook untouched, no guard, weak no-ops)"
    fi
else
    bad "Edgent_ESP32"; grep -m2 error "$W/log_ex_Edgent.txt" | sed 's/^/   /'
fi
if arduino-cli core list 2>/dev/null | grep -q esp8266; then
    for e in Boards_WiFi/ESP8266_Auto Boards_WiFi/ESP8266_Standalone \
             Boards_WiFi/NodeMCU GettingStarted/PlynxBlink_ESP8266; do
        n=$(basename "$e")
        build "ex_$n" "$FQBN_8266" "$W/libs/Plynx/examples/$e" \
            && ok "$n" || { bad "$n"; grep -m2 error "$W/log_ex_$n.txt" | sed 's/^/   /'; }
    done
fi

echo
if [ "$fails" -eq 0 ]; then
    echo "All expectations met. Logs and ELFs kept in $W"
    echo
    echo "This proves the two halves cannot be separated at build time."
    echo "It proves NOTHING about the bootloader's behaviour on a real board:"
    echo "that still needs the ten-scenario hardware run."
else
    echo "$fails expectation(s) not met. Logs in $W"
fi
exit $((fails > 0))

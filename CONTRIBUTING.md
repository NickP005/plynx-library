# Contributing to the Plynx library

Thank you for helping. This page explains how the public API is documented,
because the same comments end up in three places: the HTML reference built by
Doxygen, the editor of the Plynx app (the "Documentation" action on a selected
word), and the help the Plynx server gives to its assistant. A comment that is
wrong or vague is wrong in all three.

## The format: Doxygen comments in the headers

Every function, class, macro and setting that a sketch can use carries a
Doxygen comment, written right above its declaration in the header:

```cpp
/**
 * @brief Sends one or more values from the board to a virtual pin in the app.
 *
 * One or two paragraphs that say what it does, in the words of someone
 * writing a first sketch.
 *
 * @param pin     The virtual pin, written V0, V1, V2 and so on.
 * @param values  One or more values to send, separated by commas.
 *
 * @par Where to call
 * In a function run by a PlynxTimer, never at every turn of loop().
 *
 * @par App widget
 * Every widget set to that virtual pin.
 *
 * @code
 * void sendTemperature()
 * {
 *   Plynx.virtualWrite(V5, 21.5);
 * }
 * @endcode
 *
 * @warning The usual mistake, and what happens when it is made.
 *
 * @see PlynxTimer::setInterval, PLYNX_READ
 */
template <typename... Args>
void virtualWrite(int pin, Args... values);
```

The rules, in the order the tags appear:

1. **One block per symbol**, a `/** ... */` comment that ends on the line just
   above the declaration. Every overload has its own block. Programs read these
   comments straight from the headers, and they find the symbol in the
   declaration that follows the comment.
2. **`@brief`** is one sentence that says what the thing is or does. It says
   what it is, never only what it is not.
3. **The paragraphs** explain what the code really does. Read the code before
   writing; describe the server or the app only for what you have checked in
   their code.
4. **`@param`** for every parameter, macros included, and **`@return`** for
   every function that returns a value. `make docs` fails on a missing one
   (see below).
5. **`@par Where to call`** says where the call belongs: `setup()`, `loop()`,
   a timer function, inside `PLYNX_WRITE()`, or before the board header for a
   setting.
6. **`@par App widget`** names the widget of the Plynx app the symbol talks to,
   with the name the app shows (Button, Slider, Value Display, Gauge, Chart,
   LED, LCD, Terminal, Map, Table, Notification, Email, RTC, Bridge, Image...).
   Leave it out when no widget is involved.
7. **`@code` ... `@endcode`**, a short example. Every example compiles (see
   below).
8. **`@note`** for a fact worth knowing, **`@warning`** for the typical mistakes
   (a `delay()` in `loop()`, `virtualWrite()` at every turn of `loop()`, a
   forgotten `flush()`) and what they cause.
9. **`@see`** for the related symbols.
10. **`@deprecated`** on what is kept only for old sketches, with the
    replacement.

A setting that a sketch defines itself, such as `PLYNX_PRINT`, has no
`#define` in the library. Its comment sits on a `#define` in
`src/Plynx/PlynxSettings.h`, inside `#ifdef DOXYGEN`. No code includes that
file, so the compiler never reads those lines and they cannot change what
Doxygen sees in the other headers. A macro whose `#define` lives in a place
that cannot take a comment is documented with a block that starts with
`@def NAME`, as `PLYNX_VERSION` at the end of `src/Plynx/PlynxConfig.h`.

## How to write

- **English**, plain and short, for a person who writes a first sketch. The
  Plynx app translates the comments for people who read another language, and
  it never translates the code of the examples.
- Name things the way a sketch writes them: `Plynx.run()`, `V5`,
  `PLYNX_WRITE(V1)`.
- No dashes to join sentences and no colon to introduce an explanation. Write
  two sentences instead.
- In the text, put code, file names and `#define` lines between backticks, so
  Doxygen does not read `#` or `<...>` as its own markup.
- Say only what the code does. If a behaviour depends on the server or on the
  app, say which, and check it.

## The examples compile

An example is a small sketch, and it follows these rules:

- Top-level code only: globals, functions, `PLYNX_WRITE()` handlers,
  `setup()`, `loop()`. A statement needs a function around it.
- `#define` and `#include` lines, when present, come first.
- An example with no Plynx board header is compiled for ESP32 (with
  `<PlynxSimpleEsp32.h>`) and for AVR (an Arduino Mega with
  `<PlynxSimpleStream.h>`), so it shows code that works on any board. Write
  `// Plynx.begin(...) as in the example for your board` where the connection
  would go.
- An example that only works on one family includes its board header, such
  as `#include <PlynxSimpleEsp32.h>`, and is compiled for that board.
- `setup()` and `loop()` are added when the example has none.
- Text that is not C++ (a serial monitor output, a URL) goes in `@verbatim`,
  which is never compiled.

Check the examples with:

```
python3 extras/docs/check_doc_examples.py
```

It needs `arduino-cli` with the `esp32:esp32` and `arduino:avr` cores. It uses
the libraries of your sketchbook (the Time library for `WidgetRTC`, for
example), and it hides any other copy of Plynx, so the examples are compiled
against your working tree. `--only PlynxApi.h` checks the examples of one
file, `--list` shows what would be compiled. Exit status 0 means every example
compiled, 1 means at least one did not, 2 means the script could not check
everything (a missing core, for example).

## The HTML and XML reference

```
doxygen Doxyfile
```

writes `build/doxygen-html/index.html` and `build/doxygen-xml/`. Any warning
makes the run fail, so a broken tag, an `@param` with a wrong name, or one
`@param` missing among others is caught before it is published.

Doxygen stays quiet when a documented function has no `@param` and no
`@return` at all, unless it also warns about every internal without a
comment. The second check covers that case:

```
python3 extras/docs/doc_index.py --check build/doxygen-xml
```

It names every documented public function or macro that leaves a parameter
without `@param`, or a returned value without `@return`, and then exits
with 1. `make docs` runs Doxygen and this check together.

It also names every documentation comment that does not become an entry.
Doxygen reads only the `#if` branches that `PREDEFINED` in the `Doxyfile`
turns on, so a function under a setting such as `PLYNX_EXPERIMENTAL` needs
that setting in `PREDEFINED`. Two classes with the same name in two headers
(`PlynxWifi` of the ESP32 and of the ESP8266) become one class in Doxygen,
so only one of them carries the documentation comment, written for both.

The same script, without `--check`, writes the documentation as one JSON
file, one entry per symbol, with the sections of each comment kept apart.
The Plynx server reads that file to show the documentation in the app.

```
python3 extras/docs/doc_index.py build/doxygen-xml > plynx-docs.json
```

## Versions

The version is written in `library.properties`, `library.json`,
`PLYNX_VERSION` in `src/Plynx/PlynxConfig.h` and `PROJECT_NUMBER` in the
`Doxyfile`. `make update-ver VER=x.y.z` changes all four. The documentation
shown in the app is read from the release with that version, so a change to a
comment reaches people with the next release.

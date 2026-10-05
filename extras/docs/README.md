## Documentation of the library

The API is documented with Doxygen comments in the headers under `src/`.

- `doxygen Doxyfile`, from the root of the repository, builds the reference in
  `build/doxygen-html` (open `index.html`) and `build/doxygen-xml`.
- `python3 extras/docs/doc_index.py --check build/doxygen-xml` names every
  documented function or macro that leaves a parameter without `@param` or a
  returned value without `@return`. Without `--check` it writes the whole
  documentation as JSON, one entry per symbol, for the Plynx server and app.
- `python3 extras/docs/check_doc_examples.py` compiles every example written
  in those comments with arduino-cli, for ESP32 and AVR (and ESP8266 where an
  example asks for it).
- `make docs` runs Doxygen and the `--check`, `make doc-examples` the examples.
- [CONTRIBUTING.md](../../CONTRIBUTING.md) explains how the comments are
  written.

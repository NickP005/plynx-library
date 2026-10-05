.PHONY: docs doc-examples format-examples cloc travis-build build-examples build-boards build-tools update-travis update-ver spaces clean check

export ARDUINO_IDE_PATH=/data2/arduino-1.8.5
export ENERGIA_IDE_PATH=/data2/energia-1.6.10E18


docs:
	doxygen Doxyfile
	python3 extras/docs/doc_index.py --check build/doxygen-xml

doc-examples:
	python3 extras/docs/check_doc_examples.py

format-examples:
	find examples tests -name '*.ino' -exec astyle --options=extras/formatter.conf {} \;

cloc:
	cloc ./

build-examples:
	python extras/build-examples.py

build-boards:
	python extras/build-boards.py

build-tools:
	cd ./extras/ide-tools/; ./build.sh PlynxUpdater
	cd ./extras/ide-tools/; ./build.sh PlynxUsbScript

spaces:
	find ./src ./linux -name '*.cpp' -o -name '*.h' -exec sed -i 's/\t/    /g' {} \;
	find ./examples ./tests -name '*.ino' -o -name '*.h' -exec sed -i 's/\t/  /g' {} \;

update-travis:
	python extras/travis-generate.py > .travis.yml.new
	meld .travis.yml .travis.yml.new

update-ver:
	sed -i 's/"version": "[0-9\.]*"/"version": "$(VER)"/g' library.json
	sed -i 's/version=[0-9\.]*/version=$(VER)/g' library.properties
	sed -i 's/PLYNX_VERSION        "[0-9\.]*"/PLYNX_VERSION        "$(VER)"/g' src/Plynx/PlynxConfig.h
	sed -i 's/^PROJECT_NUMBER         = [0-9\.]*/PROJECT_NUMBER         = $(VER)/g' Doxyfile

clean:
	-rm .travis.yml.new
	-find . -type f -name "*.orig" -exec rm -f {} \;

check: spaces format-examples
	@echo '******************************************'
	@echo ' Files with no "YourAuthToken":'
	@echo '******************************************'
	@find ./examples -name "*.ino" -exec grep -L "YourAuthToken" {} \;

travis-build:
ifdef PLATFORMIO_CI_SRC
	-rm -rf ./linux
ifdef PLATFORMIO_CI_EXTRA_ARGS
	platformio ci --lib="." $(PLATFORMIO_CI_EXTRA_ARGS)
else
	platformio ci --lib="." --board=uno
endif
endif
ifdef LINUX_SRC
	make -C $(LINUX_SRC)
endif

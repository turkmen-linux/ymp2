SHELL=/bin/bash -e
all: clean build

build:
	CFLAGS="-g3 -O2" meson setup build && \
	ninja -C build

test:
	@for example in $(shell find build/test/ -type f -iname *.c) ; do \
	    echo Testing: $$example ; \
	    [ ! -f $$example ] || $$example >/dev/null;\
	done ; echo DONE

clean:
	rm -rvf build docs/html docs/latex
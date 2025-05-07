SHELL=/bin/bash -e
all: clean build

build:
	CFLAGS="-g3 -O2" meson setup build && \
	ninja -C build

test:
	@for example in $(wildcard build/examples/*) ; do \
	    if [ -f $$example ] ; then \
	        echo Testing: $$example ; \
	        cd build ; ../$$example >/dev/null; cd .. ;\
	    fi \
	done ; echo DONE

clean:
	rm -rvf build docs/html docs/latex

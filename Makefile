SHELL=/bin/bash -e
all: clean build

build:
	CFLAGS='-g3 -O3' meson setup build $(ARGS) \
	    --buildtype=debug \
	    --prefix="/" \
	    -Dlocaledir=`pwd`/po \
	    -Dplugindir=`pwd`/build \
	    && \
	ninja -C build -v


pot:
	xgettext --language=C++ -v --keyword=_\
	     -o po/ymp.pot --from-code="utf-8" \
	     `find src -type f -iname "*.c"` 2>/dev/null
	for file in `ls po/*.po`; do \
	    msgmerge $$file po/ymp.pot -o $$file.new ; \
	    echo POT: $$file; \
	    rm -f $$file ; \
	    mv $$file.new $$file ; \
	done
	sed -f data/fix-turkish.sed -i po/tr.po


test:
	@mkdir -p build/test ; \
	for example in $(wildcard build/examples/*) ; do \
	    if [ -f $$example ] ; then \
	        echo Testing: $$example ; \
	        cd build/test ; ../../$$example >/dev/null; cd ../.. ;\
	    fi \
	done ; echo DONE

clean:
	rm -rvf build docs/html docs/latex

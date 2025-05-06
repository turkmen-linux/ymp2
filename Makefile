all: clean build

build:
	CFLAGS="-g3 -O2" meson setup build && \
	ninja -C build

clean:
	rm -rvf build docs/html docs/latex
CC := gcc

.PHONY: all clean debug

all: dist/cmpr

CFLAGS := -O2 -Wall
LDFLAGS := -lm

debug: CFLAGS := -g -O0 -Wall -Werror -fsanitize=address
debug: dist/cmpr

dist/cmpr: cmpr.c spanio.c
	mkdir -p dist
	(VER=7; D=$$(date +%Y%m%d-%H%M%S); GIT=$$(git log -1 --pretty="%h %f"); sed 's/\$$VERSION\$$/'"$$VER"' (build: '"$$D"' '"$$GIT"')/' <cmpr.c >cmpr-sed.c; echo "Version: $$VER (build: $$D $$GIT)"; $(CC) -o dist/cmpr-$$D cmpr-sed.c siphash/siphash.c siphash/halfsiphash.c $(CFLAGS) $(LDFLAGS) && rm -f dist/cmpr && ln -s cmpr-$$D dist/cmpr)

clean:
	rm -rf dist

dist/cmpr: cmpr.c spanio.c
	(VER=5; D=$$(date +%Y%m%d-%H%M%S); sed 's/\$$VERSION\$$/'"$$VER"' (build: '"$$D"')/' <cmpr.c >cmpr-sed.c; echo "Version: $$VER (build: $$D)"; gcc -o dist/cmpr-$$D cmpr-sed.c -fsanitize=address -Werror -g -lm && rm -f dist/cmpr && ln -s cmpr-$$D dist/cmpr)

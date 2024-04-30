dist/cmpr: cmpr.c spanio.c
	(VER=7; D=$$(date +%Y%m%d-%H%M%S); GIT=$$(git log -1 --pretty="%h %f"); sed 's/\$$VERSION\$$/'"$$VER"' (build: '"$$D"' '"$$GIT"')/' <cmpr.c >cmpr-sed.c; echo "Version: $$VER (build: $$D $$GIT)"; gcc -o dist/cmpr-$$D cmpr-sed.c siphash/siphash.c siphash/halfsiphash.c -fsanitize=address -Wall -Werror -g -lm -lcurl && rm -f dist/cmpr && ln -s cmpr-$$D dist/cmpr)

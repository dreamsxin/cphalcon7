all: lexer.c

lexer.c: lexer.re Makefile
	re2c -b -o "$@" "$<"
	sed -i 's!^#line.*$$!/* & */!g' "$@"
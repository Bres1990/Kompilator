c: bison.y flex.l
	bison -v -d bison.y
	flex -d flex.l
	g++ -o c lex.yy.c bison.tab.c
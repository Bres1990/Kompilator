c: bison.y flex.l
	bison -v -d bison.y
	flex flex.l
	g++ -o c lex.yy.c bison.tab.c
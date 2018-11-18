execucao: server.c
	-rm -f *.bin out  	
	gcc -pthread server.c -o out
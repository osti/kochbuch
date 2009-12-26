run: waffel Makefile
	@clear
	@./waffel

waffel: waffel.c Makefile
	gcc -O2 -Wall -lm -lpthread waffel.c -o waffel -Werror

Clean:
	echo -n > waffel.input 

waffel-c: waffel-c.c
	gcc -O2 -Wall -lm waffel-c.c -o waffel-c

client: waffel-c
	./waffel-c 127.0.0.1

echo-s: echo-s.c
	gcc -O2 -Wall -lm echo-s.c -o echo-s


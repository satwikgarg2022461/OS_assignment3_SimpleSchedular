all:
	gcc -o simpleShell simpleShell.c
	gcc -o simpleSchedular simpleSchedular.c
	gcc -o fib fib.c
	gcc -o hello hello.c
	./simpleShell

clean:
	@rm -f simpleShell
	@rm -f simpleSchedular
	@rm -f fib
	@rm -f hello
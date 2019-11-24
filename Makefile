myshell: myshell.c
	gcc myshell.c -I/usr/include -lreadline -o myshellH

clean:
	rm myshell
	

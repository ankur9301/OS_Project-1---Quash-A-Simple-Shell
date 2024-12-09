shell: shell.c
	 gcc -o shell shell.c -I.

testfile: testfile.c
	 gcc -o a.out testfile.c -I.

background: background.c
	gcc -o background background.c -I.
all:
	gcc dwmstatus.c -I/usr/X11/include/ -lX11 --std=c99 -o status

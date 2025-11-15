
#Z80 CPU dis assembler hoja ver 2.01 Makefile
#Z80 CPU gyaku asenbura hoja ver2.01   Makefile

#---alphabet message
#ROMAJI = -DROMAJI
ROMAJI =

#---- UNIX ----
#CC = gcc
CC = cc
DEF =
#CFLAG = -O2
OUTNAME = -o hoja
OBJ = o
DOSLINK =
EXE =

#---- MS-DOS real mode Microsoft-C ----
#CC = cl
#DEF = -DMSDOSREAL
#CFLAG = /AC
#OUTNAME =
#OBJ = obj
#DOSLINK = /link /ST:50000
#EXE = .exe

#---- MS-DOS go32 DJGPP(MS-DOS gcc)----
#CC = gcc
#DEF = -DGO32
#OUTNAME =
#CFLAG = -O2
#OBJ = o
#DOSLINK =
#EXE =

#---- WINDOWS 95/NT Borland C++ Builder ----
#CC = bcc32
#DEF = -w- -DWIN32 -DWIN32 -I\PROGRA~1\Borland\CBuilder\include -L\PROGRA~1\Borland\CBuilder\lib
#OUTNAME =
#CFLAG =
#OBJ = obj
#DOSLINK =
#EXE = .exe

hoja$(EXE):	hoja.$(OBJ) ihex.$(OBJ)
	$(CC) $(DEF) $(OUTNAME) hoja.$(OBJ) ihex.$(OBJ)

hoja.$(OBJ):	hoja.c
	$(CC) -c $(DEF) $(ROMAJI) hoja.c $(DOSLINK)

ihex.$(OBJ):	ihex.c
	$(CC) -c $(DEF) ihex.c $(DOSLINK)

#sym.$(OBJ):	sym.c
#	$(CC) -c $(DEF) sym.c $(DOSLINK)

install:
	cp ./hoja /usr/local/bin
	ln -s /usr/local/bin/hoja /usr/local/bin/hojar


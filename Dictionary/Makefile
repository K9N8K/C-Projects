#------------------------------------------------------------------------------
# Makefile for Dictionary ADT and its test client
#------------------------------------------------------------------------------

ADT_NAME  =	Dictionary
SOURCES	  =	$(ADT_NAME).c	DictionaryClient.c
OBJECTS	  =	$(ADT_NAME).o	DictionaryClient.o
EXEBIN	  =	$(ADT_NAME)Client
HEADERS   =	$(ADT_NAME).h
FLAGS	  =	-std=c99 -Wall
VGFLAGS   =     --leak-check=full --show-leak-kinds=all

all : $(EXEBIN)

$(EXEBIN) : $(OBJECTS) $(HEADERS)
	gcc -o $(EXEBIN) $(OBJECTS)

$(OBJECTS) : $(SOURCES) $(HEADERS)
	gcc -c $(FLAGS) $(SOURCES)

check :
	valgrind $(VGFLAGS) ./$(EXEBIN)

clean :
	rm -f $(EXEBIN) $(OBJECTS)

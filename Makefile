# Name of your program:
NAME=VMsimulator

# List of all .cpp source code files included in your program (separated by spaces):
SRC=VMsimulator.c queue.c



SRCPATH=./
OBJ=$(addprefix $(SRCPATH), $(SRC:.c=.o))
CC=gcc
RM=rm -f
INCPATH=includes
CFLAGS+= -I $(INCPATH) -std=c99 -g -lm
LFLAGS=-lm

all: $(OBJ)
	gcc $(OBJ) -o $(NAME) $(LFLAGS)

clean:
	-$(RM) *~
	-$(RM) *#*
	-$(RM) *swp
	-$(RM) *.core
	-$(RM) *.stackdump
	-$(RM) $(SRCPATH)*.o
	-$(RM) $(SRCPATH)*.obj
	-$(RM) $(SRCPATH)*~
	-$(RM) $(SRCPATH)*#*
	-$(RM) $(SRCPATH)*swp
	-$(RM) $(SRCPATH)*.core
	-$(RM) $(SRCPATH)*.stackdump

fclean: clean
	-$(RM) $(NAME)

re: fclean all

run:
	./VMsimulator plist.txt ptrace.txt 2 LRU +


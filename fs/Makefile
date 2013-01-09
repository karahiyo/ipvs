#--------------------------------------------------------------------
#  Makefile for C
#--------------------------------------------------------------------
SHELL		= /bin/sh
CC     		= gcc
#CFLAGS 		= -g
#LDFLAGS		= -g -lm
CFLAGS 		= -O
LDFLAGS		= -O -lm

#--------------- You must wite all linked file name -----------------
SRCS	= fs.c imgio.c main.c
#--------------------------------------------------------------------
OBJS    = ${SRCS:.c=.o} 

#-------- Please write executive file name and put directory --------
DIR     = ./
BINS    = fs
#--------------------------------------------------------------------

all: ${BINS} clean

${BINS}: ${OBJS}
	${CC} -o ${DIR}$@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CFLAGS} -c $<

clean:
	rm -f ${OBJS} core

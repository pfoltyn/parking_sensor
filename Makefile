SRCS=main.c
PROG=main.hex

CC=sdcc-sdcc
CFLAGS=-p16f917 -mpic14 -Wl,-m --use-non-free --fverbose-asm --debug-xtra

#-Wl,-s12f675.lkr
TYPE=PIC16F917

OD=odyssey


OBJS=${SRCS:%.c=%.o}

DIS=${PROG:%.hex=%.dis}

default:${PROG} ${DIS}

${DIS}:${PROG}
	gpdasm -p${TYPE} $< > $@ || /bin/rm -f $@
${PROG}:${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS}

%.o:%.c
	${CC} ${CFLAGS} -c  $<

clean:
	/bin/rm -f ${PROG} ${OBJS} *% *~  *.sym *.lst *.cod *.asm *.dis *.map

prog: ${PROG}
	${OD} ${TYPE} erase
	${OD} ${TYPE} check
	${OD} ${TYPE} write ${PROG}
	${OD} ${TYPE} verify ${PROG}


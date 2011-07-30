
CC	= ${CROSS_COMPILE}arm-linux-gcc

# USE_INLINE   - static inline some functions (recommended but optional)
CFLAGS	+= -O3 -funroll-loops -I../include -DUSE_INLINE

LDFLAGS	+= -L/home/cain/Embedded/ARM/buildroot/output/target/lib/ -ljpeg

INSTALL	= install


.c.o :
	${CC} ${CFLAGS} -c $<


prefix	= /usr/local


LIBS	+= ../src/libembedcv.a


TARGETS = \
	jpg2crop \
	jpg2ppm \
	ppm2jpg \
	ppm2blur \
	ppm2chroma \
	ppm2edge \
	ppm2fbox \
	ppm2morph \
	ppm2seg \
	ppm2tbox


# used to prevent building anything if either JPEG or PPM support is missing



# make
all: ${TARGETS}

# make install
install :	all
	${INSTALL} -d ${prefix}/bin
	${INSTALL} -m 755 ${TARGETS} ${prefix}/bin


ppm2blur : ppm2blur.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2chroma : ppm2chroma.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2edge : ppm2edge.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2fbox : ppm2fbox.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2morph : ppm2morph.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2seg : ppm2seg.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2tbox : ppm2tbox.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

ppm2jpg : ppm2jpg.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

jpg2ppm : jpg2ppm.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}

jpg2crop : jpg2crop.o ${LIBS}
	${CC} -o $@ ${LDFLAGS} $< ${LIBS}


clean :
	rm -f *.o
	rm -f ${TARGETS}

distclean : clean
	rm Makefile


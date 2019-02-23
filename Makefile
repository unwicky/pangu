CC = gcc
CFLAGS = -g

INCLUDE = -I./include -I/usr/local/include/curl -I/usr/local/include/libxml2 -I/root/hrg/antcity-tools/include \
        -I/root/hrg/zlog-1.2.12/src
LIBS =-lpthread -L/root/hrg/antcity-tools -lantcitytools -L/root/hrg/zlog-1.2.12/src -lzlog
    

OBJECTS = pangu.o pg_page_util.o pg_extensions.o pg_download_extension.o pg_urlparser_extension.o
PROG = pangu
VPATH = src:src/extensions


.c.o:
	${CC} ${CFLAGS} ${INCLUDE} -c $<
	
all: ${OBJECTS}
	${CC} ${CFLAGS} -o ${PROG} ${OBJECTS} ${LIBS}
	#cd tests && make

clean:
	rm -f ${PROG} ${OBJECTS}
	#cd tests && make clean

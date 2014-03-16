INC = -I /usr/src/linux-headers-3.11.0-12-generic/include/
CFLAG = -g

.PHONY: clean

expand: try.c
	gcc -E try.c ${INC} ${CFLAG} -o try_expand.c
%: %.c
	gcc ${CFLAG} ${INC} $< -o $@.o

clean:
	rm -rf *.o
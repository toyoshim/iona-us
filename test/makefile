CXXFLAGS	= -std=c++17 -Igoogletest/googletest/include -I../us -I../ -D__timer3_h__
CFLAGS		= -I../ -D__timer3_h__
LFLAGS		= -Lout/lib -lgtest -lgtest_main
LIBGTEST	= out/lib/libgtest.a
OBJS			= test.o hid.o mock.o

test: ${LIBGTEST} ${OBJS}
	clang++ -o test ${OBJS} ${LFLAGS}

clean:
	rm -rf out *.o test

%.o: ../us/%.c *.h
	clang -c ${CFLAGS} -o $@ $<

%.o: %.cc *.h
	clang++ -c ${CXXFLAGS} -o $@ $<

${LIBGTEST}:
	(cd googletest && cmake . -B ../out && cd ../out && make)

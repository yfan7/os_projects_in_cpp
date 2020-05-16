CC=g++ -g -Wall -std=c++17

# List of source files for your file server
FS_SOURCES= server.cpp globals.cpp fs_handlers.cpp

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:.cpp=.o}
tests = test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 test14 test15 test16 test17 test18 test19 test20 test21 test22 test32 test33 test34 test35 test40 test

all: fs $(tests)

# Compile the file server and tag this compilation
fs: ${FS_OBJS} libfs_server.o
	./autotag.sh
	${CC} -o $@ $^ -pthread -ldl

$(tests):
 %: %.cpp libfs_client.o
	${CC} -o $@ $^ -pthread -ldl

# Compile a client program
# app: test1.cpp libfs_client.o
	# ${CC} -o $@ $^
	

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${FS_OBJS} fs app $(tests)
runserver: 
	ulimit -c unlimited
	./createfs
	export FS_CRYPT=AES
	./fs 10001 <users 

runtest:
	ulimit -c unlimited
	export FS_CRYPT=AES
	# time ./test40 localhost 10000
	for i in {1..100} ; do ( \./test40 localhost 10001 & \
	 ./test2 localhost 10001  & ./test3 localhost 10001 \
	& ./test7 localhost 10001 & ./test2 localhost 10001 \
	& ./test1 localhost 10001 & ./test2 localhost 10001 \
	& ./test5 localhost 10001 & ./test6 localhost 10001 \
	& ./test7 localhost 10001 & ./test8 localhost 10001 \
	& ./test9 localhost 10001 & ./test10 localhost 10001 \
	& ./test11 localhost 10001 & ./test12 localhost 10001 \
	& ./test13 localhost 10001 & ./test14 localhost 10001 \
	& ./test15 localhost 10001 & ./test16 localhost 10001 \
	& ./test17 localhost 10001 & ./test18 localhost 10001 \
	& ./test19 localhost 10001 & ./test20 localhost 10001 \
	& ./test21 localhost 10001 & ./test22 localhost 10001 \
	& ./test23 localhost 10001 & ./test24 localhost 10001 \
	& ./test25 localhost 10001 & ./test26 localhost 10001 \
	& ./test27 localhost 10001 & ./test28 localhost 10001 \
	& ./test29 localhost 10001 & ./test30 localhost 10001 \
	& ./test31 localhost 10001 & ./test32 localhost 10001 \
	& ./test33 localhost 10001 & ./test34 localhost 10001 \
	& ./test7 localhost 10001 & ./test1 localhost 10001 \
	 & ./test33 localhost 10001); done

runall:
	ulimit -c unlimited
	export FS_CRYPT=AES
	for i in {1..1000}; do (./test34 localhost 10000); done
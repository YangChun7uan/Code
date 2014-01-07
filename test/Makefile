include rules

objs= utility.o signalhandler.o Socket.o Semaphore.o ShareMemory.o 

ext_inc = $(inc)
sys_lib = -lpthread -lrt
ext_lib = 

all:main main2
        
main: main.o $(objs)
	$(cpp) $(cppflag) -o $@ $^ $(sys_lib) $(ext_lib)

main2: main2.o $(objs)
	$(cpp) $(cppflag) -o $@ $^ $(sys_lib)

libtest.so: test.o
	$(makeso) -o $@ $^ $(sys_lib)
        
clean:
	rm -rf main main2
	rm -rf *.o

        
# DO NOT DELETE
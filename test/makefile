objects = mul_pipe.o

mul_pipe : $(objects)
	g++ -o mul_pipe $(objects)

mul_pipe.o : ../npshell.h

.PHONY: clean
clean :
	rm mul_pipe &(objects)

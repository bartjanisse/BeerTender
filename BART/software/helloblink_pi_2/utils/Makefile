all:	vectors.o

#vectors : vectors.o
#	arm-linux-gcc -o $@ $+

vectors.o : vectors.s
	arm-linux-as  -o $@ $<

clean:
	rm -vf vectors *.o
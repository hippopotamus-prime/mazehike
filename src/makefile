ofiles			= palm.o main.o maze.o
prcname			= mazehike
internalname	= Maze Hiker
grcname			= mazehike
creatorID		= "MZHK"
ccopts			= -Wall -Werror -O2

demo: prc clean
	ls -l *.prc

clean:
	rm -f *.bin *.grc $(grcname)

veryclean:
	rm -f *.bin *.grc $(grcname) *.o

prc: grc bin
	build-prc $(prcname).prc "$(internalname)" $(creatorID) *.bin *.grc $(buildopts)

bin: rc.rcp rc.h
	pilrc $(pilrcopts) rc.rcp
	pilrc $(pilrcopts) cosines.rcp
	pilrc $(pilrcopts) angles.rcp

grc: $(ofiles)
	m68k-palmos-gcc $(ccopts) -o $(grcname) $(ofiles)
	m68k-palmos-obj-res $(grcname)

palm.o: palm.c palm.h main.h
	m68k-palmos-gcc $(ccopts) -c palm.c

main.o: main.c main.h palm.h maze.h
	m68k-palmos-gcc $(ccopts) -c main.c

maze.o: maze.c maze.h main.h
	m68k-palmos-gcc $(ccopts) -c maze.c
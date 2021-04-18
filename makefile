ofiles			= palm.o main.o maze.o
prcname			= mazehike
internalname	= Maze Hiker
grcname			= mazehike
creatorID		= "MZHK"
ccopts			= -Wall -Werror -O2

.PHONY: all clean veryclean bin_files grc_files

all: $(prcname).prc
	ls -l *.prc

clean:
	rm -rf bin grc

veryclean:
	rm -rf bin grc obj $(prcname).prc

$(prcname).prc: grc_files bin_files
	build-prc $(prcname).prc "$(internalname)" $(creatorID) bin/*.bin grc/*.grc $(buildopts)

bin_files: src/rc.rcp src/rc.h src/angles.rcp src/cosines.rcp | bin
	cd src && pilrc $(pilrcopts) rc.rcp ../bin
	cd src && pilrc $(pilrcopts) cosines.rcp ../bin
	cd src && pilrc $(pilrcopts) angles.rcp ../bin

grc_files: $(patsubst %.o,obj/%.o,$(ofiles)) | grc
	m68k-palmos-gcc $(ccopts) -o grc/$(grcname) $^
	cd grc && m68k-palmos-obj-res $(grcname)

obj/%.o: src/%.c src/*.h | obj
	m68k-palmos-gcc $(ccopts) -c -o $@ $<

obj bin grc:
	mkdir -p $@

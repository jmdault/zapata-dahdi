all:		    mkfilter mkshape mkaverage gencode genplot

mkfilter:   mkfilter.o complex.o
		    gcc mkfilter.o complex.o -o mkfilter -lm

mkshape:    mkshape.o complex.o
		    gcc mkshape.o complex.o -o mkshape -lm

mkaverage:  mkaverage.o complex.o
		    gcc mkaverage.o complex.o -o mkaverage -lm

gencode:    gencode.o complex.o readdata.o
		    gcc gencode.o complex.o readdata.o -o gencode -lm

genplot:    genplot.o complex.o readdata.o
		    gcc genplot.o complex.o readdata.o -o genplot -L$libdir -lgd -lm

%.o:		    %.C mkfilter.h complex.h
		    gcc -I$libdir -O -c $stem.C

clean:
		    rm -f mkfilter.o mkshape.o mkaverage.o gencode.o genplot.o complex.o readdata.o
		rm -f mkfilter mkshape mkaverage gencode genplot

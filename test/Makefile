.SILENT:

NUM_PT=50

all: aes

lib:
	make -C ../measure/

aes: lib aes.cpp
	g++ -DNUM_PT=${NUM_PT} -maes aes.cpp -L../measure -I../measure -l:libmeasure.a -o aes

clean:
	make -C ../measure/ clean
	rm aes
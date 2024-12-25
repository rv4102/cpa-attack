.SILENT:

NUM_PT=200

all: aes hamming
	chmod +x generate_key.sh

lib:
	make -C ../measure/

aes: lib aes.cpp utils.cpp
	g++ -DNUM_PT=${NUM_PT} -maes aes.cpp -L../measure -I../measure -l:libmeasure.a -o aes

hamming: hamming.cpp utils.cpp
	g++ -DNUM_PT=${NUM_PT} -mssse3 -maes hamming.cpp -o hamming

run: aes hamming
	./generate_key.sh

clean:
	make -C ../measure/ clean
	rm hamming aes

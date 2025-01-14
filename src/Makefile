.SILENT:

NUM_PT=300

all: aes hamming
	chmod +x generate_key.sh

lib:
	make -C ../measure/

time:
	g++ -mssse3 -maes time.cpp utils.cpp -L../measure -I../measure -l:libmeasure.a -o time

aes: lib aes.cpp utils.cpp
	g++ -DNUM_PT=${NUM_PT} -mssse3 -maes aes.cpp utils.cpp -L../measure -I../measure -l:libmeasure.a -o aes

hamming: hamming.cpp utils.cpp
	g++ -DNUM_PT=${NUM_PT} -mssse3 -maes hamming.cpp utils.cpp -o hamming

run: aes hamming
	./generate_key.sh

clean:
	make -C ../measure/ clean
	rm hamming aes

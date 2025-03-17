.SILENT:

all: aes hamming controller
	chmod +x generate_key.sh

lib:
	make -C ../measure/

aes: lib aes.cpp
	g++ -mssse3 -maes aes.cpp -L../measure -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I../measure -I$(HOME)/intel/oneapi/ippcp/2025.0/include -l:libmeasure.a -lippcp -o aes -pthread -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

hamming: hamming.cpp
	g++ -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I$(HOME)/intel/oneapi/ippcp/2025.0/include hamming.cpp -lippcp -o hamming -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

controller:
	g++ controller.cpp -o controller

clean:
	make -C ../measure/ clean
	rm hamming aes controller

clean_data:
	rm -f results/*texts.txt results/traces.csv results/hamm*.csv results/key_ranks.txt

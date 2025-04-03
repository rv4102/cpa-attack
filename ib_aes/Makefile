.SILENT:

all: aes
	chmod +x generate_key.sh

lib:
	make -C ../measure/

aes: lib aes.cpp
	g++ -mssse3 -maes aes.cpp -L../measure -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I../measure -I$(HOME)/intel/oneapi/ippcp/2025.0/include -l:libmeasure.a -lippcp -o aes -pthread -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

clean:
	make -C ../measure/ clean
	rm aes

clean_data:
	rm -f results/*texts.txt results/traces.csv results/hamm*.csv results/key_ranks.txt results/correlations.csv

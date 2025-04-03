.SILENT:

all: aes

aes: aes.cpp msr_handler.h
	g++ -mssse3 -maes aes.cpp -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I$(HOME)/intel/oneapi/ippcp/2025.0/include -lippcp -o aes -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

clean:
	rm -f aes

clean_data:
	rm -f results/*texts.txt results/traces.csv results/hamm*.csv results/key_ranks.txt results/readings.csv results/correlation.csv
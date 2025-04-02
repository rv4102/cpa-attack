.SILENT:

all: aes hamming controller

aes: aes.cpp msr_handler.h
	g++ -mssse3 -maes aes.cpp -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I$(HOME)/intel/oneapi/ippcp/2025.0/include -lippcp -o aes -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

hamming: hamming.cpp
	g++ -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I$(HOME)/intel/oneapi/ippcp/2025.0/include hamming.cpp -lippcp -o hamming -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib

controller:
	g++ controller.cpp -o controller

clean:
	make -C ../measure/ clean
	rm -f hamming aes controller out_of_band_measurer

clean_data:
	rm -f results/*texts.txt results/traces.csv results/hamm*.csv results/key_ranks.txt results/energy_readings.csv
.SILENT:

all: aes
	mkdir -p data
	sudo taskset -c 0-3 ./aes 0 2> data/zero.csv
	sudo taskset -c 0-3 ./aes 1 2> data/full.csv

lib:
	make -C ../measure/

controller:
	g++ controller.cpp -o controller

aes: lib aes.cpp
	g++ -mssse3 -maes aes.cpp -L../measure -L/home/sdp/intel/oneapi/ippcp/2025.0/lib -I../measure -I/home/sdp/intel/oneapi/ippcp/2025.0/include -l:libmeasure.a -lippcp -o aes -Wl,-R/home/sdp/intel/oneapi/ippcp/2025.0/lib
	
clean:
	make -C ../measure/ clean
	rm -r aes controller zero.csv full.csv
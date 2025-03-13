.SILENT:

all: aes controller

lib:
	make -C ../measure/

controller:
	g++ controller.cpp -o controller

aes: lib aes.cpp
	g++ -mssse3 -maes aes.cpp -L../measure -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I../measure -I$(HOME)/intel/oneapi/ippcp/2025.0/include -l:libmeasure.a -lippcp -o aes -pthread -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib
	
clean:
	make -C ../measure/ clean
	rm -r aes controller

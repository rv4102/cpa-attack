.SILENT:

all: aes controller

controller:
	g++ controller.cpp -o controller

aes: aes.cpp
	g++ aes.cpp -L$(HOME)/intel/oneapi/ippcp/2025.0/lib -I$(HOME)/intel/oneapi/ippcp/2025.0/include -lippcp -o aes -lpthread -Wl,-R$(HOME)/intel/oneapi/ippcp/2025.0/lib
	
clean:
	rm -r aes controller

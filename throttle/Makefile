.SILENT:

all: main

lib:
	make -C ../measure/

controller:
	g++ controller.cpp -o controller

main: lib controller main.cpp
	g++ main.cpp -L../measure -I../measure -l:libmeasure.a -o main

run: main
	sudo taskset -c 1 ./main 0 2> zero.csv
	sudo taskset -c 1 ./main 255 2> full.csv

clean:
	make -C ../measure/ clean
	rm -r main controller zero.csv full.csv
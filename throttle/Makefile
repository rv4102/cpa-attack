.SILENT:

all: main

lib:
	make -C ../measure/

controller:
	g++ controller.cpp -o controller

# main: lib controller main.cpp
# 	g++ main.cpp -L../measure -I../measure -l:libmeasure.a -o main

# run: main
# 	sudo taskset -c 1 ./main 0 2> zero.csv
# 	sudo taskset -c 1 ./main 255 2> full.csv

clean:
	make -C ../measure/ clean
	rm -r main controller zero.csv full.csv

multi_core_main: lib controller multi_core_main.cpp
	g++ multi_core_main.cpp -L../measure -I../measure -l:libmeasure.a -o multi_core_main -pthread

multi_core_run: multi_core_main
	mkdir -p data
	sudo taskset -c 0-3 ./multi_core_main 0 4 2> data/zero.csv
	sudo taskset -c 0-3 ./multi_core_main 255 4 2> data/full.csv
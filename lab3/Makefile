all: filter

task1:
	g++ task1.cpp -fopenmp -o task1.out

task2:
	g++ task2.cpp -fopenmp -o task2.out

task3:
	g++ task3.cpp -fopenmp -o task3.out

task5:
	g++ task5.cpp -fopenmp -o task5.out

task6:
	g++ task6.cpp -fopenmp -o task6.out

filter:
	g++ -o image_filter filter.cpp `pkg-config --cflags --libs opencv4` -fopenmp

clean:
	rm -rf task*.out image_filter
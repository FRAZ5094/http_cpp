default:
	g++ *.cpp -o main && ./main
build:
	g++ *.cpp -o main
run:
	./main
clean:
	rm main
	rm *.o
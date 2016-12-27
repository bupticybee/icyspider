crawler: spider.o
	g++ -o spider spider.o -levent -lpthread

spider.o: spider.cpp 
	g++ -c spider.cpp

clean:
	rm main.o spider

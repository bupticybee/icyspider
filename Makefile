crawler: spider.o
	g++ -o spider spider.o -levent -lpthread

spider.o: spider.cpp crawl.o
	g++ -c spider.cpp -std=c++0x

clean:
	rm main.o spider


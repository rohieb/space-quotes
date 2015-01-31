quotes: quotes.cpp
	g++ -O0 -ggdb --std=c++11 -o $@ $<

clean:
	rm -f quotes

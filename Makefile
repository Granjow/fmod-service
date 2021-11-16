.PHONY : doctest
doctest :
	wget https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h -O lib/doctest.h

.PHONY : all
all : doctest

.PHONY : clean
clean :
	rm lib/doctest.h

.PHONY : doctest
doctest :
	wget https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h -O lib/doctest.h

.PHONY : gitversion
gitversion :
	echo -n "const char *version = \"" > src/version.h
	git describe --tags HEAD | tr -d '\n' >> src/version.h
	echo "\";" >> src/version.h

.PHONY : all
all : doctest

.PHONY : clean
clean :
	rm lib/doctest.h

#default path for installing petty
install_PREFIX=/usr/local
lib_install_PATH=$(install_PREFIX)/lib
include_install_PATH=$(install_PREFIX)/include/petty

libs:
	cd src && make clean && make libs && cd ..
	@if [ ! -d lib ]; then mkdir lib; fi
	cd src && cp *.a ../lib && cp *.so ../lib && cd ..

tests:
	cd test && make clean && make tests && cd ..

install:
	mkdir -p ${include_install_PATH}
	mkdir -p ${lib_install_PATH}
	cp include/* ${include_install_PATH}
	cp src/*.a ${lib_install_PATH}
	cp src/*.so ${lib_install_PATH}

all:libs tests
	
.PHONY:clean libs tests install all

clean:
	cd src && make clean && cd .. && cd test && make clean && cd ..
	rm -f *.o
	rm -rf lib

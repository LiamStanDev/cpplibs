default_target : 
	@cmake -B build -DCMAKE_CXX_COMPILER=$(shell which g++) -DCMAKE_C_COMPILER=$(shell which gcc)
	@cd build && make -j4


clean : 
	@rm -r bin build lib 

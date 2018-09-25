all:
	clang++ -g -O3 main.cpp `llvm-config --cxxflags`
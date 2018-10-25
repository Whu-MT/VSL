LLVM = `llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native`
all:
	mkdir -p bin/Debug
	mkdir -p obj/Debug
	clang++ -g -O3 -c main.cpp -o obj/Debug/main.o $(LLVM)
	clang++ obj/Debug/main.o -o bin/Debug/VSL $(LLVM)
clean:
	rm -r -f bin obj
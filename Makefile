LLVM = `llvm-config --cxxflags --ldflags --system-libs --libs all`
all:
	mkdir -p bin/Debug
	mkdir -p obj/Debug
	clang++ -g -Dlinux -O3 -c main.cpp -o obj/Debug/main.o $(LLVM)
	clang++ obj/Debug/main.o -o bin/Debug/VSL $(LLVM)
clean:
	rm -r -f bin obj
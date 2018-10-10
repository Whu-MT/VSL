LLVM = `llvm-config --cxxflags --ldflags --system-libs --libs core`
all:
	mkdir -p bin/Debug
	mkdir -p obj/Debug
	clang++ -g -O3 -c main.cpp -o obj/Debug/main.o $(LLVM)
	clang++ obj/Debug/main.o -o bin/Debug/VSL $(LLVM)
clean:
	rm -r -f bin obj
	mv VSL.cbp ..
	mv VSL.layout ..
	mv VSL.depend ..
cbp:
	mv ../VSL.cbp .
	mv ../VSL.layout .
	mv ../VSL.depend .
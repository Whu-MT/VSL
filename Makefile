all:
	mkdir bin
	mkdir obj
	clang++ -g -O3 main.cpp -o obj/Debug/main.o `llvm-config --cxxflags`
	clang++  -o bin/Debug/VSL obj/Debug/main.o
clean:
	rm -r bin obj
	mv VSL.cbp ..
	mv VSL.layout ..
cbp:
	mv ../VSL.cbp .
	mv ../VSL.layout .
#Makefile to test the program

INC = -g -I./include -I./headers -std=c++17
INC_C = -g -I ./include -I ./headers
TST = -lglfw3 -lfreetype
FRM = -lbz2 -lz -Iglfw3/include/ -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

default: executable

saphira.o: source/saphira.cpp headers/*.h
	g++ -c $(INC) source/saphira.cpp
Mission.o: source/Mission.cpp headers/*.h
	g++ -c $(INC) source/Mission.cpp
Material.o: source/Material.cpp headers/*.h
	g++ -c $(INC) source/Material.cpp
TexHandler.o: source/TexHandler.cpp headers/*.h
	g++ -c $(INC) source/TexHandler.cpp
Planet.o: source/Planet.cpp headers/*.h
	g++ -c $(INC) source/Planet.cpp
GUI.o: source/GUI.cpp headers/*.h
	g++ -c $(INC) source/GUI.cpp
Camera.o: source/Camera.cpp headers/*.h
	g++ -c $(INC) source/Camera.cpp
Spacecraft.o: source/Spacecraft.cpp headers/*.h
	g++ -c $(INC) source/Spacecraft.cpp
glad.o: source/glad.c
	gcc -c $(INC_C) source/glad.c
OdeSolver.o: source/OdeSolver.cpp headers/*.h
	g++ -c $(INC) source/OdeSolver.cpp

executable: saphira.o Material.o Planet.o TexHandler.o Camera.o GUI.o glad.o Spacecraft.o Mission.o OdeSolver.o
	g++ $(INC) $(FRM) Camera.o GUI.o saphira.o Material.o Planet.o TexHandler.o Spacecraft.o Mission.o OdeSolver.o glad.o libs/libfreetype.a libs/libglfw3.a libs/cspice.a
	mv a.out saphira.app/Contents/MacOs/mainApp

clean:
	rm *.o
	rm saphira.app/Contents/MacOS/mainApp


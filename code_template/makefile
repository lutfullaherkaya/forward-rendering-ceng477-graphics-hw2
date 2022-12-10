OBJS	= Camera.o Color.o Helpers.o Main.o Matrix4.o Mesh.o Rotation.o Scaling.o Scene.o tinyxml2.o Translation.o Triangle.o Vec3.o Vec4.o
SOURCE	= Camera.cpp Color.cpp Helpers.cpp Main.cpp Matrix4.cpp Mesh.cpp Rotation.cpp Scaling.cpp Scene.cpp tinyxml2.cpp Translation.cpp Triangle.cpp Vec3.cpp Vec4.cpp
HEADER	= Camera.h Color.h Helpers.h Matrix4.h Mesh.h Rotation.h Scaling.h Scene.h tinyxml2.h Translation.h Triangle.h Vec3.h Vec4.h
OUT	= rasterizer
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

Camera.o: Camera.cpp
	$(CC) $(FLAGS) Camera.cpp

Color.o: Color.cpp
	$(CC) $(FLAGS) Color.cpp

Helpers.o: Helpers.cpp
	$(CC) $(FLAGS) Helpers.cpp

Main.o: Main.cpp
	$(CC) $(FLAGS) Main.cpp

Matrix4.o: Matrix4.cpp
	$(CC) $(FLAGS) Matrix4.cpp

Mesh.o: Mesh.cpp
	$(CC) $(FLAGS) Mesh.cpp

Rotation.o: Rotation.cpp
	$(CC) $(FLAGS) Rotation.cpp

Scaling.o: Scaling.cpp
	$(CC) $(FLAGS) Scaling.cpp

Scene.o: Scene.cpp
	$(CC) $(FLAGS) Scene.cpp

tinyxml2.o: tinyxml2.cpp
	$(CC) $(FLAGS) tinyxml2.cpp

Translation.o: Translation.cpp
	$(CC) $(FLAGS) Translation.cpp

Triangle.o: Triangle.cpp
	$(CC) $(FLAGS) Triangle.cpp

Vec3.o: Vec3.cpp
	$(CC) $(FLAGS) Vec3.cpp

Vec4.o: Vec4.cpp
	$(CC) $(FLAGS) Vec4.cpp


clean:
	rm -f $(OBJS) $(OUT)

run: $(OUT)
	./$(OUT)
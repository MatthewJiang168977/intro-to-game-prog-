UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

SRCS = main.cpp CS3113/cs3113.cpp CS3113/Entity.cpp CS3113/Map.cpp \
       CS3113/Effects.cpp CS3113/ShaderProgram.cpp \
       CS3113/MenuScene.cpp CS3113/LevelScene.cpp CS3113/BattleScene.cpp

TARGET = stackoverflow
CXX = g++
CXXFLAGS = -std=c++11

ifeq ($(UNAME_S),Darwin)
    RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
    RAYLIB_LIBS   = $(shell pkg-config --libs   raylib)
    CXXFLAGS += -arch arm64 $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = ./$(TARGET)
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = ./$(TARGET)
else ifeq ($(UNAME_S),Windows)
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = $(TARGET)
else
    RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
    RAYLIB_LIBS   = $(shell pkg-config --libs   raylib)
    CXXFLAGS += $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC = ./$(TARGET)
endif

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LIBS)

.PHONY: clean run
clean:
	@rm -f $(TARGET) $(TARGET).exe
run: $(TARGET)
	$(EXEC)

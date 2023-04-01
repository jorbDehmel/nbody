SOURCES := nbody.cpp main.cpp
OBJECTS := $(SOURCES:.cpp=.o)
EXE := main.out

CC := clang++
LFLAGS := `sdl2-config --libs`
CFLAGS := -Wall -Werror -pedantic `sdl2-config --cflags`

all: $(SOURCES) $(EXE)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(OBJECTS) $(EXE)
CC = gcc
CFLAGS = -g -std=c11 -Wall -Wextra -c
LFLAGS = -lSDL2 -lGL -lGLU -lGLEW -lm deps/stb_image/stb_image.a deps/tinycthread/tinycthread.a -lpthread

SOURCEDIR = src
BUILDDIR = build
SHADERDIR = shaders

EXE = voxel
SOURCES = $(wildcard $(SOURCEDIR)/*.c)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

all: $(BUILDDIR)/$(EXE)

run:
	./$(BUILDDIR)/$(EXE)

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/$(EXE)
	rm -f *~ $(SOURCEDIR)/*~
	rm -f $(SHADERDIR)/*~

rebuild: clean all run

$(BUILDDIR)/$(EXE): $(OBJECTS)
	$(CC) $^ -o $@ $(LFLAGS)

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@



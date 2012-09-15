SOURCES = main.o shell.o input.o builtin.o
OBJECTS = $(SOURCES:.c=.o)
CC = clang
CFLAGS = -Werror
LDFLAGS = 
EXE = shell

all: $(SOURCES) $(EXE)

$(EXE): $(SOURCES)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

.c:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXE)
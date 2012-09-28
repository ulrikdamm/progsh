CC = clang
CFLAGS = -Wall -Werror -Wextra
LDFLAGS = 
SOURCES = main.c util.c cmd.c input.c shell.c array.c
OBJECTS = $(SOURCES:.c=.o)
EXE = shell

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

.c:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXE)

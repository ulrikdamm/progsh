CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -lreadline
SOURCES = main.c util.c cmd.c input.c shell.c array.c
OBJECTS = $(SOURCES:.c=.o)
EXE = shell

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.c:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXE)

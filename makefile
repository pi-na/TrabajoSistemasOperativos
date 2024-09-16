CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm -fsanitize=address

# Default target
all: view app slave

# Rule to compile view
view: view.c
	$(CC) $(CFLAGS) view.c -o view

# Rule to compile app
app: app.c list.c
	$(CC) $(CFLAGS) app.c list.c -o app $(LDFLAGS)

# Rule to compile slave
slave: slave.c
	$(CC) $(CFLAGS) slave.c -o slave $(LDFLAGS)

# Clean up the build files
clean:
	rm -f view app slave result.txt

.PHONY: all clean

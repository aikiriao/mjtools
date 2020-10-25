CC 		    = gcc
CFLAGS 	  = -std=c99 -O0 -g3 -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
CPPFLAGS	= -DDEBUG
LDFLAGS		= -O0
LDLIBS    = -lm
SRC				= test_main.c test.c test_mj_tile.c test_mj_shanten.c
INCLUDE   = 
OBJS	 		= $(SRC:%.c=%.o) 
TARGET    = test 

all: $(TARGET) 

rebuild:
	make clean
	make all

run: $(TARGET)
	./test

clean:
	rm -f $(OBJS) $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c $<

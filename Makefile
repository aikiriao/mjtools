CC 		    = gcc
AR			= ar
CFLAGS 	  	= -std=c99 -O0 -g3 -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
CPPFLAGS	= -DDEBUG
LDFLAGS		= -Wall -Wextra -Wpedantic -O0
LDLIBS		= -lm
ARFLAGS		= r

SRCS      	= mj_shanten.c mj_score.c mj_random_xoshiro256pp.c mj_deck.c mj_shanten_use_table.c
SRCS      	+= mj_player_wrapper.c mj_river.c 
SRCS      	+= mj_player_tsumogiri.c mj_player_shantenman.c mj_player_furoman.c
OBJS		= $(SRCS:%.c=%.o)
TARGETS   	= libmjtools.a

all: $(TARGETS) 

rebuild:
	make clean
	make all

clean:
	rm -rf $(TARGETS) $(OBJS)

libmjtools.a : $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

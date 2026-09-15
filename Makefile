NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCS = coders/main.c coders/coder.c coders/dongles.c coders/monitor.c\
		coders/parser.c coders/queue.c coders/state.c coders/utils.c\
		coders/thread.c coders/dongles_utils.c coders/request.c\
		coders/sim_phase.c coders/log.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c coders.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

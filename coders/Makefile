CC = cc
NAME = codexion

CFLAGS = -Wall -Wextra -Werror
RM = rm -f

SRCS = main.c parser.c utils.c coder.c dongles.c state.c monitor.c queue.c

OBJS = $(SRCS:.c=.o)
CHECKER_OBJS = $(CHECKER_SRCS:.c=.o)

all: $(NAME)

bonus: $(CHECKER)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(CHECKER): $(CHECKER_OBJS)
	$(CC) $(CFLAGS) -o $(CHECKER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(CHECKER_OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

bonus_re : fclean bonus

.PHONY: all clean fclean re bonus bonus_re

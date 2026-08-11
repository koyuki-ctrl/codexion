

#include "coders.h"

static int	arg_error_handler(int argc, char **argv, t_arguments *arguments)
{
	if (argc != 9)
	{
		fprintf(stdout, "Invalid Arguments\n");
		return (0);
	}
	if (!arguments_validator(argv, arguments))
	{
		fprintf(stdout, "Invalid Arguments\n");
		return (0);
	}
	gettimeofday(&arguments->start_time, NULL);
	return (1);
}

static void	routing_sim(t_coder *coders, t_arguments *arguments)
{
	int	i;

	i = 0;
	while (i < arguments->coders)
	{
		coders[i].id = i + 1;
		coders[i].args = arguments;
		coders[i].left = &arguments->dongles[i];
		coders[i].right = &arguments->dongles[(i + 1) % arguments->coders];
		coders[i].compiles_done = 0;
		coders[i].last_compile_start = arguments->start_time;
		i++;
	}
	i = 0;
	while (i < arguments->coders)
	{
		pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}
	i = 0;
	while (i < arguments->coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
}

static int	init_allocation(t_arguments *arguments, t_coder **coders)
{
	int	i;

	if (!arguments->dongles)
	{
		fprintf(stdout, "Allocation error\n");
		return (0);
	}
	i = 0;
	while (i < arguments->coders)
	{
		dongle_init(&arguments->dongles[i], i, arguments->coders);
		i++;
	}
	*coders = malloc(sizeof(t_coder) * arguments->coders);
	if (!*coders)
	{
		free(arguments->dongles);
		fprintf(stdout, "Allocation error\n");
		return (0);
	}
	return (1);
}

static int	dongles_manager(t_arguments *arguments, t_coder **coders)
{
	if (!arguments->dongles)
	{
		fprintf(stdout, "Allocation error\n");
		return (0);
	}
	if (!init_allocation(arguments, coders))
	{
		free(arguments->dongles);
		return (0);
	}
	arguments->coder_list = *coders;
	if (
		pthread_create(
			&arguments->monitor, NULL,
			monitor_routine, arguments) != 0)
		return (0);
	return (1);
}

int	main(int argc, char **argv)
{
	t_arguments	arguments;
	t_coder		*coders;
	int			i;

	coders = NULL;
	if (!arg_error_handler(argc, argv, &arguments))
		return (1);
	pthread_init(&arguments);
	arguments.next_ticket = 0;
	arguments.stop = 0;
	arguments.dongles = malloc(sizeof(t_dongle) * arguments.coders);
	if (!dongles_manager(&arguments, &coders))
		return (0);
	routing_sim(coders, &arguments);
	pthread_join(arguments.monitor, NULL);
	i = 0;
	while (i < arguments->coders)
	{
		dongle_destroy(&arguments->dongles[i]);
		i++;
	}
	pthread_destroy(&arguments, coders);
	return (0);
}

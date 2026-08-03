#include "coders.h"

int	main(int argc, char **argv)
{
	t_arguments	arguments;
	t_coder		*coders;
	int			i;

	if (argc != 9)
	{
		fprintf(stdout, "Invalid Arguments\n");
		return (0);
	}
	if (!arguments_validator(argv, &arguments))
	{
		fprintf(stdout, "Invalid Arguments\n");
		return (0);
	}
	gettimeofday(&arguments.start_time, NULL);
	pthread_mutex_init(&arguments.print_lock, NULL);
	pthread_mutex_init(&arguments.stop_lock, NULL);
	pthread_mutex_init(&arguments.count_lock, NULL);
	pthread_mutex_init(&arguments.state_lock, NULL);
	pthread_cond_init(&arguments.state_cond, NULL);
	arguments.stop = 0;

	arguments.dongles = malloc(sizeof(t_dongle) * arguments.coders);
	if (!arguments.dongles)
		return (1);
	i = 0;
	while (i < arguments.coders)
	{
		dongle_init(&arguments.dongles[i], i);
		i++;
	}
	coders = malloc(sizeof(t_coder) * arguments.coders);
	arguments.coder_list = coders;

	if (!coders)
		return (1);
	i = 0;
	while (i < arguments.coders)
	{
		coders[i].id = i + 1;
		coders[i].args = &arguments;
		coders[i].left = &arguments.dongles[i];
		coders[i].right = &arguments.dongles[(i + 1) % arguments.coders];
		coders[i].compiles_done = 0;
		coders[i].last_compile_start = arguments.start_time;
		i++;
	}
	if (pthread_create(&arguments.monitor, NULL, monitor_routine, &arguments) != 0)
		return (1);
	i = 0;
	while (i < arguments.coders)
	{
		pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}
	i = 0;
	while (i < arguments.coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
	pthread_join(arguments.monitor, NULL);
	i = 0;
	while(i < arguments.coders)
	{
		dongle_destroy(&arguments.dongles[i]);
		i++;
	}
	pthread_mutex_destroy(&arguments.print_lock);
	pthread_mutex_destroy(&arguments.stop_lock);
	pthread_mutex_destroy(&arguments.count_lock);
	pthread_mutex_destroy(&arguments.state_lock);
	pthread_cond_destroy(&arguments.state_cond);
	free(arguments.dongles);
	free(coders);
	return (0);
}

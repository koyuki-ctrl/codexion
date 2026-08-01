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
	coders = malloc(sizeof(t_coder) * arguments.coders);
	if (!coders)
		return (1);
	i = 0;
	while (i < arguments.coders)
	{
		coders[i].id = i + 1;
		coders[i].args = &arguments;
		pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}
	i = 0;
	while (i < arguments.coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
	pthread_mutex_destroy(&arguments.print_lock);
	free(coders);
	return (0);
}

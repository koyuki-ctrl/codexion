#include "coders.h"

static int	is_all_done(t_arguments *args, t_coder *coders)
{
	int	i;

	i = 0;
	while (i < args->coders)
	{
		if (coders[i].compiles_done < args->compiles)
			return (0);
		i++;
	}
	return (1);
}

int	is_stopped(t_arguments *args)
{
	int	value;

	pthread_mutex_lock(&args->stop_lock);
	value = args->stop;
	pthread_mutex_unlock(&args->stop_lock);
	return (value);
}

void	request_stop(t_arguments *args)
{
	int	i;

	pthread_mutex_lock(&args->stop_lock);
	args->stop = 1;
	pthread_mutex_unlock(&args->stop_lock);
	pthread_mutex_lock(&args->state_lock);
	pthread_cond_broadcast(&args->state_cond);
	pthread_mutex_unlock(&args->state_lock);
	i = 0;
	while (i < args->coders)
	{
		pthread_mutex_lock(&args->dongles[i].lock);
		pthread_cond_broadcast(&args->dongles[i].cond);
		pthread_mutex_unlock(&args->dongles[i].lock);
		i++;
	}
}

void	register_compile(t_arguments *args, t_coder *coder, t_coder *coders)
{
	int	finished;

	pthread_mutex_lock(&args->count_lock);
	coder->compiles_done++;
	finished = is_all_done(args, coders);
	pthread_mutex_unlock(&args->count_lock);
	if (finished)
		request_stop(args);
}

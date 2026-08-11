#include "coders.h"

void	pthread_init(t_arguments *arguments)
{
	pthread_mutex_init(&arguments->print_lock, NULL);
	pthread_mutex_init(&arguments->stop_lock, NULL);
	pthread_mutex_init(&arguments->count_lock, NULL);
	pthread_mutex_init(&arguments->state_lock, NULL);
	pthread_cond_init(&arguments->state_cond, NULL);
	pthread_mutex_init(&arguments->ticket_lock, NULL);
}

void	pthread_destroy(t_arguments *arguments, t_coder *coders)
{
	pthread_mutex_destroy(&arguments->print_lock);
	pthread_mutex_destroy(&arguments->stop_lock);
	pthread_mutex_destroy(&arguments->count_lock);
	pthread_mutex_destroy(&arguments->state_lock);
	pthread_cond_destroy(&arguments->state_cond);
	pthread_mutex_destroy(&arguments->ticket_lock);
	free(arguments->dongles);
	free(coders);
}

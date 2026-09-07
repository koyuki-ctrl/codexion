/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 16:48:37 by ainradan          #+#    #+#             */
/*   Updated: 2026/09/07 17:07:51 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	broadcast_stop(t_arguments *args)
{
	int	i;

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

void	request_stop(t_arguments *args)
{
	pthread_mutex_lock(&args->print_lock);
	args->stop = 1;
	pthread_mutex_unlock(&args->print_lock);
	broadcast_stop(args);
}

void	stop_with_log(t_arguments *args, int coder_id, const char *msg)
{
	pthread_mutex_lock(&args->print_lock);
	if (!args->stop)
	{
		printf("%ld %d %s\n", ms_since(&args->start_time), coder_id, msg);
		args->stop = 1;
	}
	pthread_mutex_unlock(&args->print_lock);
	broadcast_stop(args);
}

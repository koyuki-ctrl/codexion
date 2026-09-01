/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:21:34 by ainradan          #+#    #+#             */
/*   Updated: 2026/09/01 10:13:05 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	deadline_of(t_coder *c, long burnout, struct timespec *out)
{
	tv_add_ms(&c->last_compile_start, burnout, out);
}

static int	timespec_lt(struct timespec *a, struct timespec *b)
{
	if (a->tv_sec != b->tv_sec)
		return (a->tv_sec < b->tv_sec);
	return (a->tv_nsec < b->tv_nsec);
}

static int	find_earliest(t_arguments *args, struct timespec *out, int *idx)
{
	int				i;
	struct timespec	ts;
	int				found;

	found = 0;
	i = 0;
	pthread_mutex_lock(&args->count_lock);
	while (i < args->coders)
	{
		if (args->coder_list[i].compiles_done < args->compiles)
		{
			deadline_of(&args->coder_list[i], args->burnout, &ts);
			if (!found || timespec_lt(&ts, out))
			{
				*out = ts;
				*idx = i;
				found = 1;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&args->count_lock);
	return (found);
}

static int	routing_step(t_arguments *args, struct timespec *deadline, int idx)
{
	int	rc;

	rc = pthread_cond_timedwait(&args->state_cond, &args->state_lock, deadline);
	if (is_stopped(args))
		return (0);
	pthread_mutex_lock(&args->count_lock);
	if (args->coder_list[idx].compiles_done >= args->compiles)
	{
		pthread_mutex_unlock(&args->count_lock);
		return (1);
	}
	pthread_mutex_unlock(&args->count_lock);
	if (rc == ETIMEDOUT
		&& ms_since(&args->coder_list[idx].last_compile_start) >= args->burnout)
	{
		pthread_mutex_unlock(&args->state_lock);
		log_state(args, args->coder_list[idx].id, "burned out");
		request_stop(args);
		pthread_mutex_lock(&args->state_lock);
		return (0);
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_arguments		*args;
	struct timespec	deadline;
	int				idx;

	args = (t_arguments *)arg;
	pthread_mutex_lock(&args->state_lock);
	while (!is_stopped(args))
	{
		if (!find_earliest(args, &deadline, &idx))
			break ;
		if (!routing_step(args, &deadline, idx))
			break ;
	}
	pthread_mutex_unlock(&args->state_lock);
	return (NULL);
}

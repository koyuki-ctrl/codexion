/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 10:00:10 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/04 11:56:01 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	dongle_init(t_dongle *dongle, int id)
{
	dongle->id = id;
	dongle->available = 1;
	dongle->free_since.tv_sec = 0;
	dongle->free_since.tv_usec = 0;
	dongle->next_seq = 0;
	dongle->queue = NULL;
	pthread_mutex_init(&dongle->lock, NULL);
	pthread_cond_init(&dongle->cond, NULL);
}

void	dongle_destroy(t_dongle *dongles)
{
	t_request	*cur;
	t_request	*next;

	cur = dongles->queue;
	while (cur)
	{
		next = cur->next;
		free(cur);
		cur = next;
	}
	pthread_mutex_destroy(&dongles->lock);
	pthread_cond_destroy(&dongles->cond);
}

static long	compute_priority_key(t_arguments *args, t_coder *coder)
{
	long	key;
	long	key_sum;

	if (args->scheduler == 2)
	{
		pthread_mutex_lock(&args->state_lock);
		key = tv_diff_ms(&coder->last_compile_start, &args->start_time);
		key_sum = key + args->burnout;
		pthread_mutex_unlock(&args->state_lock);
		return (key);
	}
	pthread_mutex_lock(&args->ticket_lock);
	key = args->next_ticket++;
	pthread_mutex_unlock(&args->ticket_lock);
	return (key);
}

int	dongle_acquire(t_dongle *d, t_arguments *args, t_coder *coder)
{
	t_request		*req;

	req = malloc(sizeof(t_request));
	if (!req)
		return (0);
	pthread_mutex_lock(&d->lock);
	req->coder_id = coder->id;
	req->priority_key = compute_priority_key(args, coder);
	req->seq = d->next_seq++;
	req->next = NULL;
	request_enqueue(d, req);
	dongle_acquire_loop(d, args, coder);
	if (is_stopped(args))
	{
		request_remove_by_id(d, coder->id);
		pthread_mutex_unlock(&d->lock);
		return (0);
	}
	d->available = 0;
	d->queue = d->queue->next;
	free(req);
	pthread_mutex_unlock(&d->lock);
	return (1);
}

void	dongle_release(t_dongle *dongles)
{
	pthread_mutex_lock(&dongles->lock);
	dongles->available = 1;
	gettimeofday(&dongles->free_since, NULL);
	pthread_cond_broadcast(&dongles->cond);
	pthread_mutex_unlock(&dongles->lock);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 16:48:12 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/04 11:52:50 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	mark_compile_start(t_coder *c)
{
	pthread_mutex_lock(&c->args->state_lock);
	gettimeofday(&c->last_compile_start, NULL);
	pthread_cond_broadcast(&c->args->state_cond);
	pthread_mutex_unlock(&c->args->state_lock);
}

static int	take_dongle(t_coder *coder, t_dongle *first, t_dongle *second)
{
	log_state(coder->args, coder->id, "has taken a dongle");
	if (!dongle_acquire(second, coder->args, coder))
	{
		dongle_release(first);
		return (0);
	}
	log_state(coder->args, coder->id, "has taken a dongle");
	return (1);
}

void	loop_sim(t_coder *coder, t_dongle *first, t_dongle *second)
{
	int	i;

	i = 0;
	while (i < coder->args->compiles && !is_stopped(coder->args))
	{
		if (!dongle_acquire(first, coder->args, coder))
			break ;
		if (!take_dongle(coder, first, second))
			break ;
		mark_compile_start(coder);
		log_state(coder->args, coder->id, "is compiling");
		usleep(coder->args->compile * 1000);
		dongle_release(coder->left);
		dongle_release(coder->right);
		register_compile(coder->args, coder, coder->args->coder_list);
		if (is_stopped(coder->args))
			break ;
		log_state(coder->args, coder->id, "is debugging");
		usleep(coder->args->debug * 1000);
		if (is_stopped(coder->args))
			break ;
		log_state(coder->args, coder->id, "is refactoring");
		usleep(coder->args->refactor * 1000);
		i++;
	}
}

void	*coder_routine(void *args)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)args;
	if (coder->left->id < coder->right->id)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	loop_sim(coder, first, second);
	return (NULL);
}

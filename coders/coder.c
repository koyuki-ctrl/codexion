/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:20:55 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/20 13:19:14 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	mark_compile_start(t_coder *c)
{
	pthread_mutex_lock(&c->args->state_lock);
	gettimeofday(&c->last_compile_start, NULL);
	pthread_cond_broadcast(&c->args->state_cond);
	pthread_mutex_unlock(&c->args->state_lock);
}

int	take_dongle(t_coder *coder, t_dongle *first, t_dongle *second)
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

static int	is_last_compilation(int i, t_coder	*coder)
{
	if (i == coder->args->compiles - 1)
		return (1);
	else
		return (0);
}

void	loop_sim(t_coder *coder, t_dongle *first, t_dongle *second)
{
	int	i;
	int last_compile;

	i = 0;
	while (i < coder->args->compiles && !is_stopped(coder->args))
	{
		if (!acquire_dongles(coder, first, second))
			break ;
		if (!compile_phase(coder))
			break ;
		if (!debug_phase(coder))
			break ;
		refactor_phase(coder);
		last_compile = is_last_compilation(i, coder);
		register_compile(coder->args, coder, coder->args->coder_list);
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

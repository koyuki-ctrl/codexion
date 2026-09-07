/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:22:07 by ainradan          #+#    #+#             */
/*   Updated: 2026/09/07 16:51:32 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

	pthread_mutex_lock(&args->print_lock);
	value = args->stop;
	pthread_mutex_unlock(&args->print_lock);
	return (value);
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

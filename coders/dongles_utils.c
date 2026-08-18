/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:21:13 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/18 13:58:48 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	dongle_acquire_loop(t_dongle *d, t_arguments *args, t_coder *coder)
{
	struct timespec	wait_until;

	while (!is_stopped(args) && !(d->available
			&& ms_since(&d->free_since) >= args->dongle
			&& request_is_front(d, coder->id)))
	{
		if (d->available && ms_since(&d->free_since) < args->dongle)
		{
			tv_add_ms(&d->free_since, args->dongle, &wait_until);
			pthread_cond_timedwait(&d->cond, &d->lock, &wait_until);
		}
		else
			pthread_cond_wait(&d->cond, &d->lock);
	}
}

int	monitoring_manager(t_arguments *arguments, t_coder **coders)
{
	if (!arguments->dongles)
	{
		fprintf(stdout, "Allocation error\n");
		return (0);
	}
	if (!init_allocation(arguments, coders))
	{
		free(arguments->dongles);
		return (0);
	}
	arguments->coder_list = *coders;
	if (
		pthread_create(
			&arguments->monitor, NULL,
			monitor_routine, arguments) != 0)
		return (0);
	return (1);
}

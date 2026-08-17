/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_phase.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:46:47 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/17 13:27:47 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	acquire_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (!dongle_acquire(first, coder->args, coder))
		return (0);
	if (first != second)
	{
		if (!take_dongle(coder, first, second))
		{
			dongle_release(first);
			return (0);
		}
	}
	else
	{
		log_state(coder->args, coder->id, "has taken a dongle");
		log_state(coder->args, coder->id, "has taken a dongle");
	}
	return (1);
}

void	release_dongles(t_coder *coder)
{
	dongle_release(coder->left);
	if (coder->left != coder->right)
		dongle_release(coder->right);
}

int	compile_phase(t_coder *coder)
{
	mark_compile_start(coder);
	log_state(coder->args, coder->id, "is compiling");
	usleep(coder->args->compile * 1000);
	release_dongles(coder);
	register_compile(coder->args, coder, coder->args->coder_list);
	return (!is_stopped(coder->args));
}

int	debug_phase(t_coder *coder)
{
	log_state(coder->args, coder->id, "is debugging");
	usleep(coder->args->debug * 1000);
	return (!is_stopped(coder->args));
}

void	refactor_phase(t_coder *coder)
{
	log_state(coder->args, coder->id, "is refactoring");
	usleep(coder->args->refactor * 1000);
}

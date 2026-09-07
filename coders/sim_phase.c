/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_phase.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:46:47 by ainradan          #+#    #+#             */
/*   Updated: 2026/09/07 09:37:54 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	lock_compilation_resources(
	t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (!dongle_acquire(first, coder->args, coder))
		return (0);
	if (first != second)
	{
		if (!finalize_second_dongle(coder, first, second))
			return (0);
	}
	else
	{
		log_state(coder->args, coder->id, "has taken a dongle");
		if (!dongle_acquire(first, coder->args, coder))
			return (0);
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
	log_state(coder->args, coder->id, "is compiling");
	mark_compile_start(coder);
	usleep(coder->args->compile * 1000);
	release_dongles(coder);
	return (!is_stopped(coder->args));
}

int	debug_phase(t_coder *coder)
{
	log_state(coder->args, coder->id, "is debugging");
	usleep(coder->args->debug * 1000);
	return (!is_stopped(coder->args));
}

int	refactor_phase(t_coder *coder)
{
	log_state(coder->args, coder->id, "is refactoring");
	usleep(coder->args->refactor * 1000);
	return (!is_stopped(coder->args));
}

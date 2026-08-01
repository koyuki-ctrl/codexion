/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 16:48:12 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/01 17:31:29 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	*coder_routine(void *args)
{
	t_coder	*coder;
	int		i;

	coder = (t_coder *)args;
	i = 0;
	while (i < coder->args->compiles)   // pour l'instant : boucle N fois, N = compiles_required
	{
		log_state(coder->args, coder->id, "is compiling");
		usleep(coder->args->compile * 1000);
		log_state(coder->args, coder->id, "is debugging");
		usleep(coder->args->debug * 1000);
		log_state(coder->args, coder->id, "is refactoring");
		usleep(coder->args->refactor * 1000);
		i++;
	}
	return (NULL);
}

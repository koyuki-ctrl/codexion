/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 11:01:41 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/04 11:19:40 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	pthread_init(t_arguments *arguments)
{
	pthread_mutex_init(&arguments->print_lock, NULL);
	pthread_mutex_init(&arguments->stop_lock, NULL);
	pthread_mutex_init(&arguments->count_lock, NULL);
	pthread_mutex_init(&arguments->state_lock, NULL);
	pthread_cond_init(&arguments->state_cond, NULL);
	pthread_mutex_init(&arguments->ticket_lock, NULL);
}

void	pthread_destroy(t_arguments *arguments, t_coder *coders)
{
	pthread_mutex_destroy(&arguments->print_lock);
	pthread_mutex_destroy(&arguments->stop_lock);
	pthread_mutex_destroy(&arguments->count_lock);
	pthread_mutex_destroy(&arguments->state_lock);
	pthread_cond_destroy(&arguments->state_cond);
	pthread_mutex_destroy(&arguments->ticket_lock);
	free(arguments->dongles);
	free(coders);
}

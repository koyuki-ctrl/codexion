/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 13:47:49 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/01 17:27:21 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include <pthread.h>
#include <unistd.h>
# include <sys/time.h>

typedef struct s_arguments
{
	int				dongle;
	int				burnout;
	int				compile;
	int				debug;
	int				refactor;
	int				coders;
	int				compiles;
	int				scheduler;
	struct timeval	start_time;
	pthread_mutex_t	print_lock;
}	t_arguments;

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	t_arguments	*args;
}	t_coder;

int		arguments_validator(char	**argv, t_arguments	*arguments);
void	log_state(t_arguments *args, int coder_id, const char *msg);
int		ft_strict_atoi(const char	*s, long	*out);
long	ms_since(struct timeval *ref);
void	*coder_routine(void *args);

#endif

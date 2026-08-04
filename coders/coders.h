/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 13:47:49 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/04 11:51:18 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>
# include <errno.h>

typedef struct s_request
{
	int					coder_id;
	long				priority_key;
	long				seq;
	struct s_request	*next;
}	t_request;

typedef struct s_dongle
{
	int				id;
	int				available;
	struct timeval	free_since;
	long			next_seq;
	t_request		*queue;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

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
	t_dongle		*dongles;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	count_lock;
	struct s_coder	*coder_list;
	int				stop;
	pthread_mutex_t	state_lock;
	pthread_cond_t	state_cond;
	pthread_t		monitor;
	pthread_mutex_t	ticket_lock;
	long			next_ticket;
}	t_arguments;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*right;
	t_dongle		*left;
	t_arguments		*args;
	int				compiles_done;
	struct timeval	last_compile_start;
}	t_coder;

int		arguments_validator(char **argv, t_arguments *arguments);
void	log_state(t_arguments *args, int coder_id, const char *msg);
int		ft_strict_atoi(const char *s, long *out);
long	ms_since(struct timeval *ref);
void	*coder_routine(void *args);
void	dongle_init(t_dongle *dongles, int id);
int		dongle_acquire(t_dongle *d, t_arguments *args, t_coder *coder);
void	dongle_destroy(t_dongle *dongles);
void	dongle_release(t_dongle *dongles);
int		is_stopped(t_arguments *args);
void	request_stop(t_arguments *args);
void	register_compile(t_arguments *args, t_coder *coder, t_coder *coders);
void	request_enqueue(t_dongle *dongle, t_request *req);
int		request_is_front(t_dongle *dongle, int coder_id);
void	request_remove_front(t_dongle *dongle);
void	request_remove_by_id(t_dongle *dongle, int coder_id);
long	tv_diff_ms(struct timeval *later, struct timeval *earlier);
void	tv_add_ms(struct timeval *base, long ms, struct timespec *out);
void	*monitor_routine(void *arg);
void	pthread_init(t_arguments *arguments);
void	pthread_destroy(t_arguments *arguments, t_coder *coders);
void	dongle_acquire_loop(t_dongle *d, t_arguments *args, t_coder *coder);

#endif

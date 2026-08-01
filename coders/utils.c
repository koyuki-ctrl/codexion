/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 10:25:44 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/01 17:27:56 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	ft_strict_atoi(const char	*s, long	*out)
{
	long	result;
	int		i;

	if (!s || !s[0])
		return (0);
	result = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		result = result * 10 + (s[i] - '0');
		if (result > 2147483647L)
		{
			fprintf(stdout, "Overflow\n");
			return(0);
		}
		i++;
	}
	*out = result;
	return (1);
}

long	ms_since(struct timeval *ref)
{
	struct timeval	now;
	long			sec_diff;
	long			usec_diff;

	gettimeofday(&now, NULL);
	sec_diff = now.tv_sec - ref->tv_sec;
	usec_diff = now.tv_usec - ref->tv_usec;
	return (sec_diff * 1000 + usec_diff / 1000);
}

void	log_state(t_arguments *args, int coder_id, const char *msg)
{
	long	ts;

	ts = ms_since(&args->start_time);
	pthread_mutex_lock(&args->print_lock);
	printf("%ld %d %s\n", ts, coder_id, msg);
	pthread_mutex_unlock(&args->print_lock);
}

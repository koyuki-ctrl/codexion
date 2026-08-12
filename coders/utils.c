/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:22:24 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/12 09:22:25 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	ft_strict_atoi(const char *s, long *out)
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
			return (0);
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

long	tv_diff_ms(struct timeval *later, struct timeval *earlier)
{
	long	sec_diff;
	long	usec_diff;

	sec_diff = later->tv_sec - earlier->tv_sec;
	usec_diff = later->tv_usec - earlier->tv_usec;
	return (sec_diff * 1000 + usec_diff / 1000);
}

void	tv_add_ms(struct timeval *base, long ms, struct timespec *out)
{
	long	total_usec;

	total_usec = (long)base->tv_usec + (ms % 1000) * 1000;
	out->tv_sec = base->tv_sec + ms / 1000 + total_usec / 1000000;
	out->tv_nsec = (total_usec % 1000000) * 1000;
}

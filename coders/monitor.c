
#include "coders.h"

static void	deadline_of(t_coder *c, long burnout, struct timespec *out)
{
	tv_add_ms(&c->last_compile_start, burnout, out);
}

static int	timespec_lt(struct timespec *a, struct timespec *b)
{
	if (a->tv_sec != b->tv_sec)
		return (a->tv_sec < b->tv_sec);
	return (a->tv_nsec < b->tv_nsec);
}

static int	find_earliest(t_arguments *args, struct timespec *out, int *idx)
{
	int				i;
	struct timespec	ts;
	int				found;

	found = 0;
	i = 0;
	while (i < args->coders)
	{
		deadline_of(&args->coder_list[i], args->burnout, &ts);
		if (!found || timespec_lt(&ts, out))
		{
			*out = ts;
			*idx = i;
			found = 1;
		}
		i++;
	}
	return (found);
}

static void	routing(t_arguments *args)
{
	struct timespec	deadline;
	int				idx;
	int				rc;

	pthread_mutex_lock(&args->state_lock);
	while (!is_stopped(args))
	{
		if (!find_earliest(args, &deadline, &idx))
			break ;
		rc = pthread_cond_timedwait(&args->state_cond, &args->state_lock,
				&deadline);
		if (is_stopped(args))
			break ;
		if (
			rc == ETIMEDOUT
			&& ms_since(&args->coder_list[idx].last_compile_start)
			>= args->burnout)
		{
			pthread_mutex_unlock(&args->state_lock);
			log_state(args, args->coder_list[idx].id, "burned out");
			request_stop(args);
			pthread_mutex_lock(&args->state_lock);
			break ;
		}
	}
}

void	*monitor_routine(void *arg)
{
	t_arguments		*args;

	args = (t_arguments *)arg;
	routing(args);
	pthread_mutex_unlock(&args->state_lock);
	return (NULL);
}

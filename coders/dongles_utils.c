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

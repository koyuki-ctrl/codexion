/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 15:13:04 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/03 22:48:00 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	request_lt(t_request *a, t_request *b)
{
	if (a->priority_key != b->priority_key)
		return (a->priority_key < b->priority_key);
	return (a->seq < b->seq);
}

void	request_enqueue(t_dongle *dongle, t_request *req)
{
	t_request	**cur;

	cur = &dongle->queue;
	while (*cur && request_lt(*cur, req))
		cur = &(*cur)->next;
	req->next = *cur;
	*cur = req;
}

int	request_is_front(t_dongle *dongle, int coder_id)
{
	return (dongle->queue != NULL && dongle->queue->coder_id == coder_id);
}

void	request_remove_front(t_dongle *dongle)
{
	if (dongle->queue)
		dongle->queue = dongle->queue->next;
}

void	request_remove_by_id(t_dongle *dongle, int coder_id)
{
	t_request	**cur;
	t_request	*tmp;

	cur = &dongle->queue;
	while (*cur)
	{
		if ((*cur)->coder_id == coder_id)
		{
			tmp = *cur;
			*cur = (*cur)->next;
			free(tmp);
			return ;
		}
		cur = &(*cur)->next;
	}
}

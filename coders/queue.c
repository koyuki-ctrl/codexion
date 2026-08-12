/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:22:00 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/12 09:32:57 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	heap_swap(t_request **a, t_request **b)
{
	t_request	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	request_lt(t_request *a, t_request *b)
{
	if (a->priority_key != b->priority_key)
		return (a->priority_key < b->priority_key);
	return (a->seq < b->seq);
}

void	heap_sift_up(t_dongle *dongle, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (request_lt(dongle->heap[idx], dongle->heap[parent]))
		{
			heap_swap(&dongle->heap[idx], &dongle->heap[parent]);
			idx = parent;
		}
		else
			break ;
	}
}

int	request_is_front(t_dongle *dongle, int coder_id)
{
	t_request	*front;

	front = heap_peek(dongle);
	return (front != NULL && front->coder_id == coder_id);
}

void	heap_insert(t_dongle *dongle, t_request *req)
{
	if (dongle->heap_size >= dongle->heap_cap)
		return ;
	dongle->heap[dongle->heap_size] = req;
	heap_sift_up(dongle, dongle->heap_size);
	dongle->heap_size++;
}

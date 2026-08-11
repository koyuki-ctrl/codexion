/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 15:13:04 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/11 21:24:00 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	heap_swap(t_request **a, t_request **b)
{
	t_request	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static int	request_lt(t_request *a, t_request *b)
{
	if (a->priority_key != b->priority_key)
		return (a->priority_key < b->priority_key);
	return (a->seq < b->seq);
}

static void	heap_sift_up(t_dongle *dongle, int idx)
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

static void	heap_sift_down(t_dongle *dongle, int idx)
{
	int		size;
	int		left;
	int		right;
	int		smallest;

	size = dongle->heap_size;
	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		smallest = idx;
		if (left < size && request_lt(dongle->heap[left], dongle->heap[smallest]))
			smallest = left;
		if (right < size && request_lt(dongle->heap[right], dongle->heap[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap(&dongle->heap[idx], &dongle->heap[smallest]);
		idx = smallest;
	}
}

void	heap_insert(t_dongle *dongle, t_request *req)
{
	if (dongle->heap_size >= dongle->heap_cap)
		return ;
	dongle->heap[dongle->heap_size] = req;
	heap_sift_up(dongle, dongle->heap_size);
	dongle->heap_size++;
}

t_request	*heap_extract_min(t_dongle *dongle)
{
	t_request	*min;

	if (dongle->heap_size == 0)
		return (NULL);
	min = dongle->heap[0];
	dongle->heap_size--;
	if (dongle->heap_size > 0)
	{
		dongle->heap[0] = dongle->heap[dongle->heap_size];
		heap_sift_down(dongle, 0);
	}
	return (min);
}

t_request	*heap_peek(t_dongle *dongle)
{
	if (dongle->heap_size == 0)
		return (NULL);
	return (dongle->heap[0]);
}

int	request_is_front(t_dongle *dongle, int coder_id)
{
	t_request	*front;

	front = heap_peek(dongle);
	return (front != NULL && front->coder_id == coder_id);
}

void	request_remove_by_id(t_dongle *dongle, int coder_id)
{
	int	i;

	i = 0;
	while (i < dongle->heap_size)
	{
		if (dongle->heap[i]->coder_id == coder_id)
		{
			free(dongle->heap[i]);
			dongle->heap_size--;
			if (i < dongle->heap_size)
			{
				dongle->heap[i] = dongle->heap[dongle->heap_size];
				heap_sift_up(dongle, i);
				heap_sift_down(dongle, i);
			}
			return ;
		}
		i++;
	}
}

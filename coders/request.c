/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:28:13 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/12 09:36:48 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

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
		if (
			left < size
			&& request_lt(dongle->heap[left], dongle->heap[smallest]))
			smallest = left;
		if (
			right < size
			&& request_lt(dongle->heap[right], dongle->heap[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap(&dongle->heap[idx], &dongle->heap[smallest]);
		idx = smallest;
	}
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

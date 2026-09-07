/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 09:21:48 by ainradan          #+#    #+#             */
/*   Updated: 2026/09/07 09:26:58 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	scheduler_converter(char *str)
{
	int		result;

	if (strcmp(str, "fifo") == 0)
		result = 1;
	else if (strcmp(str, "edf") == 0)
		result = 2;
	else
		result = 0;
	return (result);
}

static int	fill_field(const char *str, int *dest)
{
	long	tmp;

	if (!ft_strict_atoi(str, &tmp))
		return (0);
	*dest = (int)tmp;
	return (1);
}

int	arguments_validator(char **argv, t_arguments *arguments)
{
	if (!fill_field(argv[1], &arguments->coders))
		return (0);
	if (!fill_field(argv[2], &arguments->burnout))
		return (0);
	if (!fill_field(argv[3], &arguments->compile))
		return (0);
	if (!fill_field(argv[4], &arguments->debug))
		return (0);
	if (!fill_field(argv[5], &arguments->refactor))
		return (0);
	if (!fill_field(argv[6], &arguments->compiles))
		return (0);
	if (!fill_field(argv[7], &arguments->dongle))
		return (0);
	arguments->scheduler = scheduler_converter(argv[8]);
	if (arguments->coders < 1 || arguments->dongle < 0
		|| (arguments->compile < 0 || arguments->debug < 0)
		|| (arguments->burnout < 0 || arguments->refactor < 0)
		|| (arguments->compiles < 1 || arguments->scheduler == 0)
	)
		return (0);
	return (1);
}

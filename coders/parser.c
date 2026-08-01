/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 14:32:52 by ainradan          #+#    #+#             */
/*   Updated: 2026/08/01 16:43:12 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static char	*str_to_lower(char *str)
{
	int		i;
	char	*new_str;

	i = 0;
	if (!str)
		return (NULL);
	new_str = malloc(sizeof(char) * (strlen(str) + 1));
	if (!new_str)
		return (NULL);
	while (str[i] != '\0')
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			new_str[i] = str[i] + 32;
		else
			new_str[i] = str[i];
		i++;
	}
	return (new_str);
}

static int	scheduler_converter(char *str)
{
	char	*lower;
	int		result;

	lower = str_to_lower(str);
	if (!lower)
		return (0);
	if (strcmp(lower, "fifo") == 0)
		result = 1;
	else if (strcmp(lower, "edf") == 0)
		result = 2;
	else
		result = 0;
	free(lower);
	return (result);
}

static int	fill_field(const char	*str, int	*dest)
{
	long	tmp;

	if (!ft_strict_atoi(str, &tmp))
		return (0);
	*dest = (int)tmp;
	return (1);
}

int	arguments_validator(char	**argv, t_arguments	*arguments)
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
	if (arguments->coders < 1 || arguments->dongle < 1
		|| (arguments->compile < 1 || arguments->debug < 1)
		|| (arguments->burnout < 1 || arguments->refactor < 1)
		|| (arguments->compiles < 1 || arguments->scheduler == 0)
	)
		return (0);
	return (1);
}

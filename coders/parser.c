/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 14:32:52 by ainradan          #+#    #+#             */
/*   Updated: 2026/07/31 17:26:11 by ainradan         ###   ########.fr       */
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
	if (strcmp(str_to_lower(str), "fifo") == 0)
		return (1);
	else if (strcmp(str_to_lower(str), "edf") == 0)
		return (2);
	else
		return (0);
}

int	arguments_validator(char **argv)
{
	t_arguments	arguments;

	arguments.coders = atoi(argv[1]);
	arguments.burnout = atoi(argv[2]);
	arguments.compile = atoi(argv[3]);
	arguments.debug = atoi(argv[4]);
	arguments.refactor = atoi(argv[5]);
	arguments.compiles = atoi(argv[6]);
	arguments.dongle = atoi(argv[7]);
	arguments.scheduler = scheduler_converter(argv[8]);
	if (arguments.coders <= 1 || arguments.dongle < 1
		|| (arguments.compile < 1 || arguments.debug < 1)
		|| (arguments.burnout < 1 || arguments.refactor < 1)
		|| (arguments.compiles < 1 || arguments.scheduler == 0)
	)
		return (0);
	else
		return (1);
}

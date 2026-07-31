/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 13:47:17 by ainradan          #+#    #+#             */
/*   Updated: 2026/07/31 17:20:19 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	main(int argc, char **argv)
{
	if (argc != 9)
	{
		fprintf(stdout, "Invalid Arguments\n");
		return (0);
	}
	else
	{
		if (arguments_validator(argv) == 1)
			printf("%s\n", "OK");
		else
			fprintf(stdout, "Invalid Arguments\n");
	}
	return (0);
}

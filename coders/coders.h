/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ainradan <ainradan@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 13:47:49 by ainradan          #+#    #+#             */
/*   Updated: 2026/07/31 17:03:00 by ainradan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

typedef struct s_arguments
{
	int	dongle;
	int	burnout;
	int	compile;
	int	debug;
	int	refactor;
	int	coders;
	int	compiles;
	int	scheduler;
}	t_arguments;

int	arguments_validator(char **argv);

#endif

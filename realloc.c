/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aleung-c <aleung-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2015/06/11 12:47:52 by aleung-c          #+#    #+#             */
/*   Updated: 2015/08/19 14:13:57 by aleung-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

void	*realloc(void *ptr, size_t size)
{
	t_mem_seg		*tmp;
	void			*new_alloc;
	char			recup[size];

	if (!ptr)
		return (NULL);
	tmp = (t_mem_seg *)((char *)ptr - sizeof(t_mem_seg));
	if (size <= tmp->size)
	{
		ft_memcpy(recup, (char *)ptr, size);
		free(ptr);
		new_alloc = malloc(size);
		ft_memcpy(new_alloc, recup, size);
		return ((char *)new_alloc);
	}
	else if (size > tmp->size)
	{
		new_alloc = malloc(size);
		ft_memcpy(new_alloc, ptr, size);
		free(ptr);
		return ((char *)new_alloc);
	}
	return (NULL);
}

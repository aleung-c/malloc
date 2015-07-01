/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aleung-c <aleung-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2015/06/05 10:38:50 by aleung-c          #+#    #+#             */
/*   Updated: 2015/07/01 14:34:40 by aleung-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <stdio.h> //

t_memzone g_memzone = { NULL, NULL, NULL };

void *ft_malloc(size_t size)
{
	void *ret;

	ret = allocate_mem(size);	
	return (ret);
}

char *allocate_mem(size_t size)
{
	void *ret;

	ret = NULL;
	if ((size + sizeof(t_mem_seg)) < (TINY - sizeof(t_mem_chunk)) && g_memzone.tiny == NULL)
		allocate_tiny();
	ret = search_mem(size);
	return ((char *)ret);
}

void allocate_tiny(void)
{
	t_mem_chunk *ret;
	t_mem_chunk *tmp;

	ret = mmap(0, TINY, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	// cast pour remplir la zone chunk allouée.
	ret->size_occupied = sizeof(t_mem_chunk);
	ret->nb_segs = 0;
	ret->first_memseg = NULL;
	ret->last_memseg = NULL;
	if (ret == MAP_FAILED)
	{
		ft_putendl_fd("ERROR - mmap", 2);
		exit(-1);
	}
	if (g_memzone.tiny == NULL)
	{
		ret->next = NULL;
		g_memzone.tiny = ret;
	}
	else
	{
		tmp = g_memzone.tiny;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = ret;
		ret->next = NULL;
	}
	ft_putendl("tiny chunk created.");
	
}

void *add_seg_to_chunk(t_mem_chunk *chunk, size_t size_asked) // free semble marcher.
{
	t_mem_seg *tmp_segs;
	t_mem_seg *tmp_next;

	if (!chunk->first_memseg) // si no segments dans le chunk.
	{
		tmp_segs = (t_mem_seg *)((char *)chunk + sizeof(t_mem_chunk));
		tmp_segs->next = NULL;
		tmp_segs->free = 0;
		tmp_segs->size = size_asked;
		chunk->size_occupied += sizeof(t_mem_seg) + size_asked;
		chunk->first_memseg = tmp_segs;

		if ((size_t)(TINY - chunk->size_occupied) > (size_t)sizeof(t_mem_seg))
		{
			// add segment freed en bout;
			// tmp_next = (t_mem_seg *)((char *)tmp_segs + sizeof(t_mem_seg) + size_asked);
			tmp_segs->next = (t_mem_seg *)((char *)tmp_segs + sizeof(t_mem_seg) + size_asked);
			tmp_segs->next->free = 1;
			tmp_segs->next->next = NULL;
			chunk->size_occupied += sizeof(t_mem_seg);
			tmp_segs->next->size = (size_t)(TINY - chunk->size_occupied);
		}
		printf("seg added first = %p\n", (char *)((char *)tmp_segs + sizeof(t_mem_seg)));
		return ((char *)tmp_segs + sizeof(t_mem_seg));
	}
	else // il existe des segs. chercher le seg free.
	{
		
		tmp_segs = chunk->first_memseg;
		while (tmp_segs)
		{
			if (tmp_segs->free == 1 && tmp_segs->size > size_asked)
			{

				tmp_segs->free = 0;
				tmp_segs->size = size_asked;
				if (tmp_segs->next)
				{
					if ((char *)tmp_segs->next - ((char *)tmp_segs + sizeof(t_mem_seg) + size_asked) 
							> (long)sizeof(t_mem_seg)) // recup inter segment.
					{
						tmp_next = tmp_segs->next; // stock next;
						tmp_segs->next = (t_mem_seg *)((char *)tmp_segs + sizeof(t_mem_seg) + size_asked);
						
						tmp_segs->next->free = 1;
						tmp_segs->next->size = (char *)tmp_segs->next - ((char *)tmp_segs + sizeof(t_mem_seg) + size_asked);
						chunk->size_occupied -= tmp_segs->next->size;
						
						tmp_segs->next->next = tmp_segs->next;
					}
					printf("seg added inner = %p\n", (char *)((char *)tmp_segs + sizeof(t_mem_seg)));
					return ((char *)tmp_segs + sizeof(t_mem_seg));
				}
				else // le seg free en bout de chaine;
				{
					tmp_segs->free = 0;
					tmp_segs->size = size_asked;
					chunk->size_occupied += size_asked;

					if ((size_t)(TINY - chunk->size_occupied) > (size_t)sizeof(t_mem_seg))
					{
						// add segment freed en bout
						tmp_segs->next = (t_mem_seg *)((char *)tmp_segs + sizeof(t_mem_seg) + size_asked);
						// tmp_segs->next = tmp_next;
						tmp_segs->next->free = 1;
						tmp_segs->next->next = NULL;
						chunk->size_occupied += sizeof(t_mem_seg);
						tmp_segs->next->size = (size_t)(TINY - chunk->size_occupied);
						printf("seg added outer = %p\n", (char *)((char *)tmp_segs + sizeof(t_mem_seg)));
					}
					return ((char *)tmp_segs + sizeof(t_mem_seg));
				}
			}
			tmp_segs = tmp_segs->next;
		}

	}
	printf("NULL\n");
	return (NULL);
}

int check_space(t_mem_chunk *chunk, size_t size)
{
	size_t free_space;
	t_mem_seg *tmp_segs;

	free_space = TINY - chunk->size_occupied;
	if ((size + sizeof(t_mem_seg)) < free_space)
		return (1);
	else
	{
		tmp_segs = chunk->first_memseg;
		while (tmp_segs)
		{
			if (tmp_segs->free == 1 && size <= tmp_segs->size)
				return (1);
			tmp_segs = tmp_segs->next;
		}
	}
	return (0);
}

char *search_mem(size_t size) // chercher liste de zones allouées pour trouver de l'espace.
{
	t_mem_chunk *tmp;
	void *ret;

	ret = NULL;
	if (size + sizeof(t_mem_seg) < (TINY - sizeof(t_mem_chunk)))
	{
		tmp = g_memzone.tiny;
		while (tmp)
		{
				if (check_space(tmp, size) == 1)
				{
					ret = add_seg_to_chunk(tmp, size);
					return((char *)ret);
				}
			tmp = tmp->next;
		}
		if (!tmp) // a parcouru tous les chunks.
		{
			ft_putendl("no space in mem, create new alloc"); //
			allocate_tiny();
			ret = search_mem(size);
		}
	}
	ft_putendl("must create SMALL"); //
	return ((char *)ret);
}
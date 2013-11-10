/* WARNING:  This file was automatically generated!
 * Original: ./src/thread/generic/SDL_sysmutex.c
 */
/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_sysmutex.c,v 1.5 2004/01/04 16:49:19 slouken Exp $";
#endif

/* An implementation of mutexes using semaphores */

#include <stdio.h>
#include <stdlib.h>

#include "SDL_error.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"


struct SDL_mutex {
	int recursive;
	Uint32 owner;
	SDL_sem *sem;
};

/* Create a mutex */
SDL_mutex *SDL_CreateMutex(void)
{
	SDL_mutex *mutex;

	/* Allocate mutex memory */
	mutex = (SDL_mutex *)malloc(sizeof(*mutex));
	if ( mutex ) {
		/* Create the mutex semaphore, with initial value 1 */
		mutex->sem = SDL_CreateSemaphore(1);
		mutex->recursive = 0;
		mutex->owner = 0;
		if ( ! mutex->sem ) {
			free(mutex);
			mutex = NULL;
		}
	} else {
		SDL_OutOfMemory();
	}
	return mutex;
}

/* Free the mutex */
void SDL_DestroyMutex(SDL_mutex *mutex)
{
	if ( mutex ) {
		if ( mutex->sem ) {
			SDL_DestroySemaphore(mutex->sem);
		}
		free(mutex);
	}
}

/* Lock the semaphore */
int SDL_mutexP(SDL_mutex *mutex)
{
#ifdef DISABLE_THREADS
	return 0;
#else
	Uint32 this_thread;

	if ( mutex == NULL ) {
		SDL_SetError("Passed a NULL mutex");
		return -1;
	}

	this_thread = SDL_ThreadID();
	if ( mutex->owner == this_thread ) {
		++mutex->recursive;
	} else {
		/* The order of operations is important.
		   We set the locking thread id after we obtain the lock
		   so unlocks from other threads will fail.
		*/
		SDL_SemWait(mutex->sem);
		mutex->owner = this_thread;
		mutex->recursive = 0;
	}

	return 0;
#endif /* DISABLE_THREADS */
}

/* Unlock the mutex */
int SDL_mutexV(SDL_mutex *mutex)
{
#ifdef DISABLE_THREADS
	return 0;
#else
	if ( mutex == NULL ) {
		SDL_SetError("Passed a NULL mutex");
		return -1;
	}

	/* If we don't own the mutex, we can't unlock it */
	if ( SDL_ThreadID() != mutex->owner ) {
		SDL_SetError("mutex not owned by this thread");
		return -1;
	}

	if ( mutex->recursive ) {
		--mutex->recursive;
	} else {
		/* The order of operations is important.
		   First reset the owner so another thread doesn't lock
		   the mutex and set the ownership before we reset it,
		   then release the lock semaphore.
		 */
		mutex->owner = 0;
		SDL_SemPost(mutex->sem);
	}
	return 0;
#endif /* DISABLE_THREADS */
}

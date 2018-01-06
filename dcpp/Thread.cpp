/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "Exception.h"
#ifdef _WIN32
#include <process.h>
#endif
#include "Thread.h"

namespace dcpp {
#ifndef _WIN32
pthread_mutex_t Thread::mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef _WIN32
void Thread::start(){
	join();
	if((threadHandle = (HANDLE)_beginthreadex(NULL, 0, &starter, this, 0, &threadId)) == NULL) {
		throw Exception("Can not create thread");
	}
}

#else
void Thread::start()  {
	join();
	if(pthread_create(&threadHandle, NULL, &starter, this) != 0) {
		throw Exception("Can not create thread");
	}
}
#endif

}

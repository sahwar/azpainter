/*$
 Copyright (C) 2013-2018 Azel.

 This file is part of AzPainter.

 AzPainter is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 AzPainter is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
$*/

/*****************************************
 * <Linux> スレッド
 *****************************************/

#include <pthread.h>

#include "mDef.h"
#include "mThread.h"



//*******************************
// mThread
//*******************************

/**
@defgroup thread mThread
@brief スレッド

@ingroup group_system
@{

@file mThread.h
@struct _mThread
*/


/** スレッド関数 */

static void *_thread_func(void *arg)
{
	mThread *p = (mThread *)arg;

	//スレッドID をセット

	mMutexLock(p->mutex);
	p->handle = (intptr_t)pthread_self();
	mMutexUnlock(p->mutex);

	//登録された関数へ

	(p->func)(p);

	return 0;
}


/** 削除 */

void mThreadDestroy(mThread *p)
{
	if(p)
	{
		mMutexDestroy(p->mutex);
	
		mFree(p);
	}
}

/** スレッド用データ作成
 *
 * @param size 構造体の全体サイズ */

mThread *mThreadNew(int size,void (*func)(mThread *),intptr_t param)
{
	mThread *p;

	if(size < sizeof(mThread)) size = sizeof(mThread);

	p = (mThread *)mMalloc(size, TRUE);
	if(!p) return NULL;

	p->func = func;
	p->param = param;
	p->mutex = mMutexNew();

	return p;
}

/** スレッド開始 */

mBool mThreadRun(mThread *p)
{
	pthread_t id;
	int loop = 1;

	if(!p || p->handle) return FALSE;

	//開始

	if(pthread_create(&id, NULL, _thread_func, p) != 0)
		return FALSE;

	//スレッド関数が開始するまで待つ

	while(loop)
	{
		mMutexLock(p->mutex);

		if(p->handle) loop = FALSE;

		mMutexUnlock(p->mutex);
	}

	return TRUE;
}

/** スレッドが終了するまで待つ */

mBool mThreadWait(mThread *p)
{
	if(!p || !p->handle) return TRUE;

	if(pthread_join(p->handle, NULL) != 0)
		return FALSE;
	else
	{
		p->handle = 0;
		return TRUE;
	}
}

/** @} */


//*******************************
// mMutex
//*******************************


#define _MUTEX(p)  ((pthread_mutex_t *)(p))


/**
@defgroup mutex mMutex
@brief ミューテクス (スレッド同期)

@ingroup group_system
@{

@file mThread.h
*/


/** 削除 */

void mMutexDestroy(mMutex p)
{
	if(p)
	{
		pthread_mutex_destroy(_MUTEX(p));

		mFree(p);
	}
}

/** 作成 */

mMutex mMutexNew(void)
{
	pthread_mutex_t *p;

	//確保

	p = (pthread_mutex_t *)mMalloc(sizeof(pthread_mutex_t), TRUE);
	if(!p) return NULL;

	//初期化

	pthread_mutex_init(p, NULL);

	return (mMutex)p;
}

/** ロック */

void mMutexLock(mMutex p)
{
	if(p)
		pthread_mutex_lock(_MUTEX(p));
}

/** ロック解除 */

void mMutexUnlock(mMutex p)
{
	if(p)
		pthread_mutex_unlock(_MUTEX(p));
}

/** @} */

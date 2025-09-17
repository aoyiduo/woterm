/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxopensslthreadcryptosafety.h"

#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#endif

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rand.h>


int THREAD_setup(void);

int THREAD_cleanup(void);

#if defined(WIN32)
    #define MUTEX_TYPE HANDLE
    #define MUTEX_SETUP(x) (x= CreateMutex(NULL, FALSE, NULL))
    #define MUTEX_CLEANUP(x) (CloseHandle(x))
    #define MUTEX_LOCK(x) (WaitForSingleObject((x), INFINITE))
    #define MUTEX_UNLOCK(x) (ReleaseMutex(x))
    #define THREAD_ID (GetCurrentThreadId())
#else
    #define MUTEX_TYPE pthread_mutex_t
    #define MUTEX_SETUP(x) (pthread_mutex_init(&(x), NULL))
    #define MUTEX_CLEANUP(x) (pthread_mutex_destroy(&(x)))
    #define MUTEX_LOCK(x) (pthread_mutex_lock(&(x)))
    #define MUTEX_UNLOCK(x) (pthread_mutex_unlock(&(x)))
    #define THREAD_ID (pthread_self())
#endif

static MUTEX_TYPE *mutex_buf = NULL;

struct CRYPTO_dynlock_value
{
    MUTEX_TYPE mutex;
};

static void locking_function(int mode, int n, const char * file, int line)
{
    if (mode & CRYPTO_LOCK){
        MUTEX_LOCK(mutex_buf[n]);
    } else {
        MUTEX_UNLOCK(mutex_buf[n]);
    }
}

static unsigned long id_function(void)
{
    return ((unsigned long)THREAD_ID);
}

static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line)
{
    struct CRYPTO_dynlock_value *value = (struct CRYPTO_dynlock_value *)malloc(sizeof(struct CRYPTO_dynlock_value));
    if (!value){
        return NULL;
    }
    MUTEX_SETUP(value->mutex);
    return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        MUTEX_LOCK(l->mutex);
    }else{
        MUTEX_UNLOCK(l->mutex);
    }
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{
    MUTEX_CLEANUP(l->mutex);
    free(l);
}


int THREAD_setup(void)
{
    int i;
    mutex_buf = (MUTEX_TYPE *)malloc(CRYPTO_num_locks() * sizeof(MUTEX_TYPE));
    if (!mutex_buf){
        return 0;
    }
    for(i = 0; i < CRYPTO_num_locks(); i++){
        MUTEX_SETUP(mutex_buf[i]);
    }
    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);
    CRYPTO_set_dynlock_create_callback(dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
    return 1;
}

int THREAD_cleanup(void)
{
    int i;
    if (!mutex_buf){
        return 0;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);
    for(i = 0; i < CRYPTO_num_locks(); i++){
        MUTEX_CLEANUP(mutex_buf[i]);
    }
    free(mutex_buf);
    mutex_buf = NULL;
    return 1;
}

bool QKxOpenSSLThreadCryptoSafety::init()
{
    qFatal("openssl > 1.1, do not do this safety.\r\n  < 1.1, the qt network had alreay do this and you should not do again");
    static bool hasInit = false;
    if(hasInit) {
        return true;
    }
    int err = THREAD_setup();
    hasInit = true;
    return err > 0;
}

void QKxOpenSSLThreadCryptoSafety::cleanup()
{
    static bool hasRelease = false;
    if(hasRelease) {
        return;
    }
    THREAD_cleanup();
    hasRelease = true;
}


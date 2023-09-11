/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QKXOPENSSLTHREADCRYPTOSAFETY_H
#define QKXOPENSSLTHREADCRYPTOSAFETY_H

#include "qkxutil_share.h"

#include <QObject>


/***
 *
 *
 *
 *
 *       openssl > 1.1, do not do this safety.
 *               < 1.1, the qt network had alreay do this and you should not do again.
 *
 *
 *
 *
 *
 *
 *
 *
 */
// RSA Crypto need to init thread safety.
class KXUTIL_EXPORT QKxOpenSSLThreadCryptoSafety
{
public:
    static bool init();
    static void cleanup();
};

#endif // QKXOPENSSLTHREADCRYPTOSAFETY_H

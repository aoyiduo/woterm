/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QVNC_DEF_H_
#define QVNC_DEF_H_


#ifdef QKXVNC_LIBRARY
    #define QKXVNC_EXPORT Q_DECL_EXPORT
#else
    #define QKXVNC_EXPORT Q_DECL_IMPORT
#endif

#endif

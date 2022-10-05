/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QKXFTP_PRIVATE_H
#define QKXFTP_PRIVATE_H

#include "qkxftp_share.h"

#include <QtGlobal>
#include <QObject>

#define FTP_FILEMODEL_DATA                  (1)
#define FTP_FILEMODEL_LIST_FILES_REQ        (2)
#define FTP_FILEMODEL_LIST_FILES_RESP       (3)
#define FTP_FILEMODEL_ENTRY_INFO_LIST_REQ   (4)  //entryInfoList
#define FTP_FILEMODEL_ENTRY_INFO_LIST_RESP  (5)
#define FTP_FILEMODEL_MKDIR_REQ             (6)
#define FTP_FILEMODEL_MKDIR_RESP            (7)

#define FTP_DOWNLOAD_INIT_REQ          (50)
#define FTP_DOWNLOAD_INIT_RESP         (51)
#define FTP_DOWNLOAD_DATA_START        (52)
#define FTP_DOWNLOAD_DATA_BLOCK        (53)
#define FTP_DOWNLOAD_DATA_NEXT         (54)
#define FTP_DOWNLOAD_DATA_END          (55)
#define FTP_DOWNLOAD_DATA_ABORT        (56)

#define FTP_UPLOAD_INIT_REQ          (100)
#define FTP_UPLOAD_INIT_RESP         (101)
#define FTP_UPLOAD_DATA_START        (102)
#define FTP_UPLOAD_DATA_BLOCK        (103)
#define FTP_UPLOAD_DATA_NEXT         (104)
#define FTP_UPLOAD_DATA_END          (105)
#define FTP_UPLOAD_DATA_ABORT        (106)


#define FTP_REQUEST_PROXY_PREFIX                ("ftp-request-proxy")
#define FTP_SERVER_PREFIX                       ("ftp-server")

#endif // QKXFTP_PRIVATE_H

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

#include "qkxcleandesktop.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <set>

#if 1
class Handle {
public:
    Handle(HANDLE h_=0) : h(h_) {

    }
    ~Handle() {
      if (h) {
          CloseHandle(h);
      }
    }
    operator HANDLE() {
      return h;
    }
    HANDLE h;
};

struct CurrentUserToken : public Handle {
    CurrentUserToken() {
        if (!OpenProcessToken(GetCurrentProcess(), GENERIC_ALL, &h)) {
          DWORD err = GetLastError();
          if (err != ERROR_CALL_NOT_IMPLEMENTED){
              throw QString("OpenProcessToken failed:%1").arg(err);
          }

          h = INVALID_HANDLE_VALUE;
        }
    }
    bool canImpersonate() const {
        return h;
    }
    bool noUserLoggedOn() const {
        return !h;
    }
};

struct ImpersonateCurrentUser {
    ImpersonateCurrentUser(){

    }
    ~ImpersonateCurrentUser(){

    }
    CurrentUserToken token;
};
#endif

struct ActiveDesktop {
  ActiveDesktop() : handle(0) {
      // - Contact Active Desktop
      HRESULT result = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (PVOID*)&handle);
      if (result != S_OK){
          throw QString("failed to contact Active Desktop:%1").arg(result);
      }
  }
  ~ActiveDesktop() {
      if (handle){
          handle->Release();
      }
  }

  // enableItem
  //   enables or disables the Nth Active Desktop item
  bool enableItem(int i, bool enable_) {
      COMPONENT item;
      memset(&item, 0, sizeof(item));
      item.dwSize = sizeof(item);
      HRESULT hr = handle->GetDesktopItem(i, &item, 0);
      if (hr != S_OK) {
          qDebug() << QString("unable to GetDesktopItem %1: %2").arg(i).arg(hr);
          return false;
      }
      item.fChecked = enable_;
      //qDebug() << QString("%sbling %d: \"%s\"", enable_ ? "ena" : "disa", i, (const char*)CStr(item.wszFriendlyName));
      hr = handle->ModifyDesktopItem(&item, COMP_ELEM_CHECKED);
      return hr == S_OK;
  }

  // enable
  //   Attempts to enable/disable Active Desktop, returns true if the setting changed,
  //   false otherwise.
  //   If Active Desktop *can* be enabled/disabled then that is done.
  //   If Active Desktop is always on (XP/2K3) then instead the individual items are
  //   disabled, and true is returned to indicate that they need to be restored later.
  bool enable(bool enable_) {
      bool modifyComponents = false;
      qDebug() << QString("ActiveDesktop::enable");
      // - Firstly, try to disable Active Desktop entirely
      HRESULT hr;
      COMPONENTSOPT adOptions;
      memset(&adOptions, 0, sizeof(adOptions));
      adOptions.dwSize = sizeof(adOptions);
      // Attempt to actually disable/enable AD
      hr = handle->GetDesktopItemOptions(&adOptions, 0);
      if (hr == S_OK) {
          // If Active Desktop is already in the desired state then return false (no change)
          // NB: If AD is enabled AND restoreItems is set then we regard it as disabled...
          if (((adOptions.fActiveDesktop==0) && restoreItems.empty()) == (enable_==false)) {
              return false;
          }
          adOptions.fActiveDesktop = enable_;
          hr = handle->SetDesktopItemOptions(&adOptions, 0);
      }
      // Apply the change, then test whether it actually took effect
      if (hr == S_OK){
          hr = handle->ApplyChanges(AD_APPLY_REFRESH);
      }
      if (hr == S_OK){
          hr = handle->GetDesktopItemOptions(&adOptions, 0);
      }
      if (hr == S_OK){
          modifyComponents = (adOptions.fActiveDesktop==0) != (enable_==false);
      }
      if (hr != S_OK) {
          qDebug() << QString("failed to get/set Active Desktop options: %1").arg(hr);
          return false;
      }
      if (enable_) {
          // - We are re-enabling Active Desktop.  If there are components in restoreItems
          //   then restore them!
          std::set<int>::const_iterator i;
          for (i=restoreItems.begin(); i!=restoreItems.end(); i++) {
              enableItem(*i, true);
          }
          restoreItems.clear();
      } else if (modifyComponents) {
          // - Disable all currently enabled items, and add the disabled ones to restoreItems
          int itemCount = 0;
          hr = handle->GetDesktopItemCount(&itemCount, 0);
          if (hr != S_OK) {
              qDebug() << QString("failed to get desktop item count: %1").arg(hr);
              return false;
          }
          for (int i=0; i<itemCount; i++) {
              if (enableItem(i, false)){
                  restoreItems.insert(i);
              }
          }
      }
      // - Apply whatever changes we have made, but DON'T save them!
      hr = handle->ApplyChanges(AD_APPLY_REFRESH);
      return hr == S_OK;
  }
  IActiveDesktop* handle;
  std::set<int> restoreItems;
};

#endif

DWORD SysParamsInfo(UINT action, UINT param, PVOID ptr, UINT ini) {
    DWORD r = ERROR_SUCCESS;
    if (!SystemParametersInfo(action, param, ptr, ini)) {
        r = GetLastError();
        qInfo() << QString("SPI error: %1").arg(r);
    }
    return r;
}

QKxCleanDesktop::QKxCleanDesktop(QObject *parent)
    : QObject(parent)
    , restoreActiveDesktop(false)
    , restoreWallpaper(false)
    , restoreEffects(false)
{
#ifdef Q_OS_WIN
    CoInitialize(0);
#endif
}

QKxCleanDesktop::~QKxCleanDesktop()
{
#ifdef Q_OS_WIN
    enableEffects();
    enableWallpaper();
    CoUninitialize();
#endif
}

void QKxCleanDesktop::backupDesktopColor()
{
    DWORD clr = GetSysColor(COLOR_BACKGROUND);
    qDebug() << "backupDesktopColor" << clr;
}

void QKxCleanDesktop::setDesktopColor()
{
    int aElements[1] = {COLOR_BACKGROUND};
    DWORD aNewColors[1];
    aNewColors[0] = RGB(0x80, 0x00, 0x80);  // dark purple
    SetSysColors(1, aElements, aNewColors);
    qDebug() << "setDesktopColor" << aNewColors[0];
    backupDesktopColor();
}

void QKxCleanDesktop::backupWallpaper()
{
    wchar_t oldWallPaper[255] = {0};
    int result = SystemParametersInfoW(SPI_GETDESKWALLPAPER, sizeof(oldWallPaper)-1, oldWallPaper, 0);
    qDebug() << "backupWallpaper" << QString::fromWCharArray(oldWallPaper) << result;
}

void QKxCleanDesktop::disableWallpaper()
{
    bool isWallSet=SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, L"", SPIF_UPDATEINIFILE);
    //bool isWallSet=SystemParametersInfoW(SPI_GETDESKWALLPAPER, 0, L"", SPIF_UPDATEINIFILE);
}

void QKxCleanDesktop::enableWallpaper()
{
    const wchar_t *filenm = L"d:\\1.jpeg"; //ADDRESS of first image
    bool isWallSet=SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)filenm, SPIF_UPDATEINIFILE);
}

void QKxCleanDesktop::disableWallpaper2()
{
    try{
        ImpersonateCurrentUser icu;
        // -=- First attempt to remove the wallpaper using Active Desktop
        ActiveDesktop ad;
        if (ad.enable(false)){
            restoreActiveDesktop = true;
        }
        // -=- Switch of normal wallpaper and notify apps
        SysParamsInfo(SPI_SETDESKWALLPAPER, 0, (PVOID) "", SPIF_SENDCHANGE);
        restoreWallpaper = true;
    } catch (...) {
        qDebug() << "exception.";
    }
}

void QKxCleanDesktop::enableWallpaper2()
{
    try {
        if (restoreActiveDesktop) {
            try {
                ActiveDesktop ad;
                ad.enable(true);
                restoreActiveDesktop = false;
            } catch (...) {
                qDebug() << "exception";
            }
        }
        if (restoreWallpaper) {
            qDebug("restore desktop wallpaper");
            // -=- Then restore the standard wallpaper if required
            SysParamsInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE);
            restoreWallpaper = false;
        }
    } catch (...) {
        qDebug() << "exception";
    }
}

void QKxCleanDesktop::disableEffects()
{
    try {
        //ImpersonateCurrentUser icu;
        qDebug("disable desktop effects");
        SysParamsInfo(SPI_SETFONTSMOOTHING, FALSE, 0, SPIF_SENDCHANGE);
        if (SysParamsInfo(SPI_GETUIEFFECTS, 0, &uiEffects, 0) == ERROR_CALL_NOT_IMPLEMENTED) {
            SysParamsInfo(SPI_GETCOMBOBOXANIMATION, 0, &comboBoxAnim, 0);
            SysParamsInfo(SPI_GETGRADIENTCAPTIONS, 0, &gradientCaptions, 0);
            SysParamsInfo(SPI_GETHOTTRACKING, 0, &hotTracking, 0);
            SysParamsInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &listBoxSmoothScroll, 0);
            SysParamsInfo(SPI_GETMENUANIMATION, 0, &menuAnim, 0);
            SysParamsInfo(SPI_SETCOMBOBOXANIMATION, 0, FALSE, SPIF_SENDCHANGE);
            SysParamsInfo(SPI_SETGRADIENTCAPTIONS, 0, FALSE, SPIF_SENDCHANGE);
            SysParamsInfo(SPI_SETHOTTRACKING, 0, FALSE, SPIF_SENDCHANGE);
            SysParamsInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, FALSE, SPIF_SENDCHANGE);
            SysParamsInfo(SPI_SETMENUANIMATION, 0, FALSE, SPIF_SENDCHANGE);
        } else {
            SysParamsInfo(SPI_SETUIEFFECTS, 0, FALSE, SPIF_SENDCHANGE);
            // We *always* restore UI effects overall, since there is no Windows GUI to do it
            uiEffects = TRUE;
        }
        restoreEffects = true;
      } catch (...) {
        qDebug() << "exception";
      }
}

void QKxCleanDesktop::enableEffects()
{
    try {
        if (restoreEffects) {
            //ImpersonateCurrentUser icu;
            qDebug("restore desktop effects");
            if (SysParamsInfo(SPI_SETUIEFFECTS, 0, (void*)(intptr_t)uiEffects, SPIF_SENDCHANGE) == ERROR_CALL_NOT_IMPLEMENTED) {
                SysParamsInfo(SPI_SETCOMBOBOXANIMATION, 0, (void*)(intptr_t)comboBoxAnim, SPIF_SENDCHANGE);
                SysParamsInfo(SPI_SETGRADIENTCAPTIONS, 0, (void*)(intptr_t)gradientCaptions, SPIF_SENDCHANGE);
                SysParamsInfo(SPI_SETHOTTRACKING, 0, (void*)(intptr_t)hotTracking, SPIF_SENDCHANGE);
                SysParamsInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, (void*)(intptr_t)listBoxSmoothScroll, SPIF_SENDCHANGE);
                SysParamsInfo(SPI_SETMENUANIMATION, 0, (void*)(intptr_t)menuAnim, SPIF_SENDCHANGE);
            }
            restoreEffects = false;
        }
      } catch (...) {
        qDebug("exception");
      }
}

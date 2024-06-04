#ifndef FORMATDEBUG_H
#define FORMATDEBUG_H

#include "Windows.h"
#include <iostream>

// Debug Buffer
char buffer[256];
char formatName[256];

// 등록된 클립보드 포맷 이름 가져오는 함수
const char* GetStandardClipboardFormatName(UINT uFormat) {
    switch (uFormat) {
    case CF_TEXT: return "CF_TEXT";
    case CF_BITMAP: return "CF_BITMAP";
    case CF_METAFILEPICT: return "CF_METAFILEPICT";
    case CF_SYLK: return "CF_SYLK";
    case CF_DIF: return "CF_DIF";
    case CF_TIFF: return "CF_TIFF";
    case CF_OEMTEXT: return "CF_OEMTEXT";
    case CF_DIB: return "CF_DIB";
    case CF_PALETTE: return "CF_PALETTE";
    case CF_PENDATA: return "CF_PENDATA";
    case CF_RIFF: return "CF_RIFF";
    case CF_WAVE: return "CF_WAVE";
    case CF_UNICODETEXT: return "CF_UNICODETEXT";
    case CF_ENHMETAFILE: return "CF_ENHMETAFILE";
    case CF_HDROP: return "CF_HDROP";
    case CF_LOCALE: return "CF_LOCALE";
    case CF_DIBV5: return "CF_DIBV5";
    case CF_OWNERDISPLAY: return "CF_OWNERDISPLAY";
    case CF_DSPTEXT: return "CF_DSPTEXT";
    case CF_DSPBITMAP: return "CF_DSPBITMAP";
    case CF_DSPMETAFILEPICT: return "CF_DSPMETAFILEPICT";
    case CF_DSPENHMETAFILE: return "CF_DSPENHMETAFILE";
    default: return NULL;
    }
}

void GetClipboardFormat(UINT uFormat, char* formatName, int size)
{
    const char* standardName = GetStandardClipboardFormatName(uFormat);
    if (standardName) {
        sprintf_s(formatName, size, "%s", standardName);
    }
    else {
        // 등록된 클립보드 형식이 아니라면 애플리케이션에서 정의한 클립보드 형식
        if (GetClipboardFormatNameA(uFormat, formatName, size) == 0) {
            sprintf_s(formatName, size, "Unknown format 0x%x", uFormat);
        }
    }
}

#endif // FORMATDEBUG_H


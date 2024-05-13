#pragma once
#include "Windows.h"
#include <string>
#include <atlimage.h>
class CMyTool
{
public:
	static void Dump(BYTE* pData, size_t nSize) {
        std::string strout;
        for (size_t i = 0; i < nSize; i++) {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strout += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strout += buf;
        }
        strout += "\n";
        OutputDebugStringA(strout.c_str());
    }
    static int Bytes2Image(CImage& image, const std::string& strBuffer)
    {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == nullptr) {
			TRACE("内存不足了！");
			Sleep(1);	//防止cpu一直卡死在这里
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (hRet == S_OK) {
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL)image.Destroy();
			image.Load(pStream);
		}
		return hRet;
    }
};


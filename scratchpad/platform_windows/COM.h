//platform specific file (WINDOWS)
#ifndef _COM_H
#define _COM_H

#include "stdafx.h" //windows precompiled header includes #import of MSXML
#include <windows.h>
#include <iostream>
#include <assert.h>
using namespace std;

#include <comutil.h> //code in comsuppw.lib (dependency). VARIANT types utilities
using namespace _com_util; //VARIANT types utilities

namespace general_server {
	class COM {
		IUnknown *m_pUnknown;

	protected:
		COM() {}
		~COM() {}
		COM(IUnknown *pUnknown): m_pUnknown(pUnknown) {}

		void setUnknown(IUnknown *pUnknown) {m_pUnknown = pUnknown;}
		unsigned long addRef() {return m_pUnknown->AddRef();}
		unsigned long release() {return m_pUnknown->Release();}

		const char *interpretCOMError(_com_error &e, long *lCode = 0) const;

#ifdef _DEBUG
		virtual unsigned long refCount() {
			m_pUnknown->AddRef();
			return m_pUnknown->Release();
		}
#endif
	};
}
#endif

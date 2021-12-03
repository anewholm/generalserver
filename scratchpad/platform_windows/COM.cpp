//platform specific file (WINDOWS)
#include "COM.h"
#include <exception>

namespace general_server {
	const char *COM::interpretCOMError(_com_error &e, long *lCode) const {
		char *sValue = 0;
		BSTR sDescB;
		IErrorInfo *ei = e.ErrorInfo();
		ei->GetDescription(&sDescB);
		ei->Release();
		sValue = ConvertBSTRToString(sDescB);
		SysFreeString(sDescB);
		if (lCode) *lCode = e.WCode();
		return sValue;
	}
}

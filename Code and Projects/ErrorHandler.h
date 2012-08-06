#pragma once

enum vErrorType
{
	kFatal = 0,
	kNonFatal = 1,
	kUserAbort = 2,
	kWithCopy = 256,
};

void HandleError(vErrorType vType, bool fCheckDependentStatus, int nErrorTextId, ...);

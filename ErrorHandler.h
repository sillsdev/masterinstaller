#pragma once

enum vErrorType
{
	kFatal,
	kNonFatal,
	kUserAbort,
};

void HandleError(vErrorType vType, bool fCheckDependentStatus, int nErrorTextId, ...);

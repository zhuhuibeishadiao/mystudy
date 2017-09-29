#pragma once
namespace usr::hijacking
{
	void hijack_knownDlls(std::wstring dllname);
	void hijack_me(PVOID ImageBase);
}
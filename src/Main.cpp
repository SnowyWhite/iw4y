#include "STDInclude.hpp"

namespace Main
{
	static Utils::Hook EntryPointHook;

	void Initialize()
	{
		Main::EntryPointHook.Uninstall();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(NULL), 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect);

		Main::EntryPointHook.Initialize(0x6BAC0F, [] ()
		{
			__asm
			{
				// This has to be called, otherwise the hook is not uninstalled and we're deadlocking
				call Main::Initialize

				mov eax, 6BAC0Fh
				jmp eax
			}

		})->Install();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}

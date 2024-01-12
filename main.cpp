#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>


extern "C" __declspec(dllexport) int fuzz_hwppdfsdk(char* pDocument, wchar_t* hwp_path);

typedef INT(__cdecl* InitHwp2PdfSDK_jayoo_t)(HMODULE lpModuleName, WCHAR* plugins_path, INT a3);
typedef INT(__cdecl* CreateDocument_jayoo_t)(char** pDocument, INT(_stdcall* a2)(int a1, int a2));
typedef INT(__cdecl* CloseDocument_jayoo_t)(char** pDocument);
typedef UINT(__cdecl* OpenHwp_jayoo_t)(char* pDocument, wchar_t* pszname, INT a3);
typedef INT(*ReleaseHwp2PdfSDK_jayoo_t)();

InitHwp2PdfSDK_jayoo_t InitHwp2PdfSDK_jayoo_func;
CreateDocument_jayoo_t CreateDocument_jayoo_func;
CloseDocument_jayoo_t CloseDocument_jayoo_func;
OpenHwp_jayoo_t OpenHwp_jayoo_func;
ReleaseHwp2PdfSDK_jayoo_t ReleaseHwp2PdfSDK_jayoo_func;

bool load_dll_and_functions() {
	HMODULE hHwppdfsdk = LoadLibraryA("hwppdfsdk.dll");

	if (hHwppdfsdk == NULL) {
		printf("Failed to load hwppdfsdk.dll\n");
		return false;
	}

	InitHwp2PdfSDK_jayoo_func = (InitHwp2PdfSDK_jayoo_t)GetProcAddress(hHwppdfsdk, "InitHwp2PdfSDK_jayoo");
	CreateDocument_jayoo_func = (CreateDocument_jayoo_t)GetProcAddress(hHwppdfsdk, "CreateDocument_jayoo");
	OpenHwp_jayoo_func = (OpenHwp_jayoo_t)GetProcAddress(hHwppdfsdk, "OpenHwp_jayoo");
	CloseDocument_jayoo_func = (CloseDocument_jayoo_t)GetProcAddress(hHwppdfsdk, "CloseDocument_jayoo");
	ReleaseHwp2PdfSDK_jayoo_func = (ReleaseHwp2PdfSDK_jayoo_t)GetProcAddress(hHwppdfsdk, "ReleaseHwp2PdfSDK_jayoo");

	return true;
}

int fuzz_hwppdfsdk(char* pDocument, wchar_t* hwp_path) {

	int open_res = OpenHwp_jayoo_func(pDocument, hwp_path, 0);

	if (open_res != 0) {
		printf("Failure on OpenDocument(err=0X%X)\n", open_res);
	}

	return open_res;
}

int __stdcall dummy_func(int a1, int a2)
{
	return 0;
}

int wmain(int argc, wchar_t** argv)
{
	wchar_t plugin_path[100];
	char buffer[520];
	char* pDocument = 0;


	if (argc != 2) {
		printf("USAGE: %ws input_file\n", argv[0]);
		return 1;
	}

	if (load_dll_and_functions() == false) {
		printf("Failure on load_dll_and_functions\n");
		return 0;
	}


	wcscpy(plugin_path, L"C:\Program Files (x86)\JayooPDF\plugins");
	int init_res = InitHwp2PdfSDK_jayoo_func(NULL, plugin_path, 0x11223344);
	if (init_res != 0) {
		printf("Failure on CreateDocument(err=0X%X)\n", init_res);
		return 0;
	}

	int create_res = CreateDocument_jayoo_func(&pDocument, dummy_func);
	if (create_res != 0) {
		printf("Failure on CreateDocument(err=0X%X)\n", init_res);
		ReleaseHwp2PdfSDK_jayoo_func();
		return  0;
	}

	fuzz_hwppdfsdk(pDocument, argv[1]);

	int close_res = CloseDocument_jayoo_func(&pDocument);
	if (create_res != 0) {
		printf("Failure on CloseDocument(err=0X%X)\n", init_res);
	}

	ReleaseHwp2PdfSDK_jayoo_func();
	return  0;
}
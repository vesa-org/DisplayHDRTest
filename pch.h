//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#pragma once

// Target latest OS/SDK
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#include <PathCch.h>

#include <wrl/client.h>
#include <wrl.h>

#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <d2d1effects_2.h>
#include <d2d1effectauthor_1.h>
#include <d2d1effecthelpers.h>
#include <wincodec.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

#include <algorithm>
#include <array>
#include <exception>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#include <stdio.h>

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = { 0 };
            sprintf_s(s_str, "Failure with HRESULT of %08X", result);
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }

    // Formats numbers with fixed precision.
    template <typename T>
    std::wstring to_string_with_precision(const T a_value, const int n = 4)
    {
        std::wstringstream out;
        out << std::setprecision(n) << a_value;
        return out.str();
    }

    // In some environments (e.g. MSIX/desktop bridge), we have to force the absolute file path
    // for APIs such as WIC - even though the working directory appears to be correct.
    // Note this breaks file loading for cases where the working directory is not where the executable is,
    // for example during Visual Studio debugging.
    inline std::wstring GetAbsolutePath(_In_ std::wstring filename)
    {
        WCHAR wd[MAX_PATH];
        DWORD length = GetModuleFileName(NULL, wd, ARRAYSIZE(wd));
        PathCchRemoveFileSpec(wd, ARRAYSIZE(wd));

        std::wstringstream abspath;
        abspath << wd << L"\\" << filename;

        //MessageBox(nullptr, abspath.str().c_str(), L"Full path", MB_OK);
        return abspath.str();
    }

    inline HRESULT ReadDataFromFile(LPCWSTR filename, byte** data, UINT* size)
    {
        using namespace Microsoft::WRL;

        CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
        extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
        extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
        extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
        extendedParams.lpSecurityAttributes = nullptr;
        extendedParams.hTemplateFile = nullptr;

        Wrappers::FileHandle file(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
        if (file.Get() == INVALID_HANDLE_VALUE)
        {
            throw std::exception();
        }

        FILE_STANDARD_INFO fileInfo = {};
        if (!GetFileInformationByHandleEx(file.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
        {
            throw std::exception();
        }

        if (fileInfo.EndOfFile.HighPart != 0)
        {
            throw std::exception();
        }

        *data = reinterpret_cast<byte*>(malloc(fileInfo.EndOfFile.LowPart));
        *size = fileInfo.EndOfFile.LowPart;

        if (!ReadFile(file.Get(), *data, fileInfo.EndOfFile.LowPart, nullptr, nullptr))
        {
            throw std::exception();
        }

        return S_OK;
    }
}
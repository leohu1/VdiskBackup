//
// Created by 26071 on 2024/9/16.
//

#include "VolumeSystem.h"
#include <windows.h>


static std::string WChar2String (WCHAR wchars[]){
    DWORD dwNum = WideCharToMultiByte(
            CP_UTF8,
            NULL,
            wchars,-1,nullptr,
            0,nullptr,FALSE);
    char *psText;
    psText = new char[dwNum];
    WideCharToMultiByte (CP_UTF8,NULL,wchars,-1,
                         psText,dwNum,NULL,FALSE);
    std::string string1 = psText;
    return string1;
}


vector<VolumeInfo> VolumeSystem::GetAllVolumeInfo() {
    DWORD  CharCount            = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";
    vector<VolumeInfo> vector_out;

    //
    //  Enumerate all volumes in the system.
    FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();
        wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
        return {};
    }

    for (;;)
    {
        //
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0]     != L'\\' ||
            VolumeName[1]     != L'\\' ||
            VolumeName[2]     != L'?'  ||
            VolumeName[3]     != L'\\' ||
            VolumeName[Index] != L'\\')
        {
            Error = ERROR_BAD_PATHNAME;
            wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
            break;
        }

        //
        //  QueryDosDeviceW does not allow a trailing backslash,
        //  so temporarily remove it.
        VolumeName[Index] = L'\0';

        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName));

        VolumeName[Index] = L'\\';

        if ( CharCount == 0 )
        {
            Error = GetLastError();
            wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
            break;
        }
        VolumeInfo volumeInfo;
        volumeInfo.device_path = WChar2String(DeviceName);
        volumeInfo.GUID_path = WChar2String(VolumeName);
        volumeInfo.drive_letter = GetVolumePaths(VolumeName);
        vector_out.push_back(volumeInfo);


        //
        //  Move on to the next volume.
        Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if ( !Success )
        {
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES)
            {
                wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
                break;
            }

            //
            //  Finished iterating
            //  through all the volumes.
            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;
    return vector_out;
}

std::string VolumeSystem::GetVolumePaths(PWCHAR VolumeName)
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names     = nullptr;
    PWCHAR NameIdx   = nullptr;
    BOOL   Success   = FALSE;
    std::string out_string;

    for (;;)
    {
        //
        //  Allocate a buffer to hold the paths.
        Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

        if ( !Names )
        {
            //
            //  If memory can't be allocated, return.
            return out_string;
        }

        //
        //  Obtain all of the paths
        //  for this volume.
        Success = GetVolumePathNamesForVolumeNameW(
                VolumeName, Names, CharCount, &CharCount
        );

        if ( Success )
        {
            break;
        }

        if ( GetLastError() != ERROR_MORE_DATA )
        {
            break;
        }

        //
        //  Try again with the
        //  new suggested size.
        delete [] Names;
        Names = nullptr;
    }

    if ( Success )
    {
        //
        //  Display the various paths.
        for ( NameIdx = Names;
              NameIdx[0] != L'\0';
              NameIdx += wcslen(NameIdx) + 1 )
        {
            out_string += ' ';
            out_string += WChar2String(NameIdx);
        }
    }

    if ( Names != nullptr )
    {
        delete [] Names;
        Names = nullptr;
    }
    return out_string;
}

size_t VolumeSystem::GetVolumeFreeSpace(std::string PathInVolume)
{
    /// 得到盘符， 例如： "C:\\"
std::string str_disk_name	= PathInVolume.substr(0, 3);
DWORD64 qwFreeBytesToCaller = 0;
DWORD64 qwTotalBytes		= 0;
DWORD64 qwFreeBytes			= 0;

///使用GetDiskFreeSpaceEx获取磁盘信息并打印结果
	BOOL bResult				= GetDiskFreeSpaceExA(	str_disk_name.c_str(),
													(PULARGE_INTEGER)&qwFreeBytesToCaller,
													(PULARGE_INTEGER)&qwTotalBytes,
													(PULARGE_INTEGER)&qwFreeBytes) ;

	/// 读取成功
	if (bResult)
	{
        return qwFreeBytesToCaller;		
		//printf("使用GetDiskFreeSpaceEx获取磁盘空间信息\n");
		//printf("可获得的空闲空间（字节）: \t%I64d\n", qwFreeBytesToCaller);
		//printf("空闲空间（字节）: \t\t%I64d\n", qwFreeBytes);
		//printf("磁盘总容量（字节）: \t\t%I64d\n", qwTotalBytes);
	}
	/// 读取失败
	else
		return 0;

}

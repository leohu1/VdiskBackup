#include "VirtDiskSystem.h"
#include "utils.h"
#include <indicators/progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include <initguid.h>
#include <virtdisk.h>
#include <spdlog/spdlog.h>
#pragma comment(lib,"VirtDisk.lib")
using namespace indicators;

bool VirtDiskSystem::CompactVdiskFileSystemAware(std::string path){
    std::wstring w_path(path.begin(), path.end());
    const wchar_t *p_w = w_path.c_str();
    VIRTUAL_STORAGE_TYPE vst;
    vst.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    vst.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
    void *handle;
    OPEN_VIRTUAL_DISK_PARAMETERS OpenParameters;
    OpenParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
    DWORD result_open = OpenVirtualDisk(&vst, p_w,
                                        VIRTUAL_DISK_ACCESS_ALL,
                                        OPEN_VIRTUAL_DISK_FLAG_NONE,&OpenParameters, &handle);
    if (result_open != 0){
        SPDLOG_ERROR("Have error opening vhdx file, result ({}): {}", result_open, utils::ErrorMessage(result_open));
        return false;
    }
    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
    DWORD result_attach = AttachVirtualDisk(handle, nullptr,
                                            ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY,
                                            0, &attachParameters, nullptr);
    if (result_attach != 0){
        SPDLOG_ERROR("Have error attaching vhdx file, result ({}): {}", result_attach, utils::ErrorMessage(result_attach));
        return false;
    }
    COMPACT_VIRTUAL_DISK_PARAMETERS compactParameters;
    compactParameters.Version = COMPACT_VIRTUAL_DISK_VERSION_UNSPECIFIED;
    compactParameters.Version1.Reserved = 0;
    OVERLAPPED overlap;
    memset(&overlap, 0, sizeof(overlap));

    ProgressBar bar{
            option::BarWidth{50},
            option::Start{"["},
            option::Fill{"="},
            option::Lead{"="},
            option::Remainder{"-"},
            option::End{" ]"},
            option::ShowElapsedTime{true},
            option::ShowRemainingTime{true},
            option::Stream{std::cout},
    };

    indicators::show_console_cursor(false);

    DWORD result_compact = CompactVirtualDisk(handle, COMPACT_VIRTUAL_DISK_FLAG_NONE, &compactParameters, &overlap);
    DWORD dw_error = GetLastError();
    VIRTUAL_DISK_PROGRESS virtualDiskProgress = {0};
    LPCTSTR errMsg = nullptr;

    if (!result_compact)
    {
        if (dw_error == ERROR_IO_PENDING){
            BOOL bPending=TRUE;
            DWORD get_progress_result;

            // Loop until the I/O is complete, that is: the overlapped
            // event is signaled.

            while( bPending )
            {
                get_progress_result = GetVirtualDiskOperationProgress(handle, &overlap, &virtualDiskProgress);
                if (get_progress_result != ERROR_SUCCESS){
                    errMsg = utils::ErrorMessage(dw_error);
                    SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
                    LocalFree((LPVOID)errMsg);
                }
                if(virtualDiskProgress.OperationStatus != ERROR_IO_PENDING){
                    bPending = FALSE;
                }
                bar.set_option(option::MaxProgress{virtualDiskProgress.CompletionValue});
                bar.set_progress(virtualDiskProgress.CurrentValue);
                Sleep(100);
            }
        }else{
            errMsg = utils::ErrorMessage(dw_error);
            SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
            LocalFree((LPVOID)errMsg);
        }
    }
    DWORD result_detach = DetachVirtualDisk(handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
    if (result_detach != 0){
        SPDLOG_ERROR("Have error detaching vhdx file, result %d", result_open);
    }
    CloseHandle(handle);
    indicators::show_console_cursor(true);
    return true;
}

bool VirtDiskSystem::CompactVdiskFileSystemAgnostic(std::string path){
    std::wstring w_path(path.begin(), path.end());
    const wchar_t *p_w = w_path.c_str();
    VIRTUAL_STORAGE_TYPE vst;
    vst.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    vst.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
    void *handle;
    OPEN_VIRTUAL_DISK_PARAMETERS OpenParameters;
    OpenParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
    DWORD result_open = OpenVirtualDisk(&vst, p_w,
                                        VIRTUAL_DISK_ACCESS_METAOPS,
                                        OPEN_VIRTUAL_DISK_FLAG_NONE,&OpenParameters, &handle);
    if (result_open != 0){
        SPDLOG_ERROR("Have error opening vhdx file, result %d", result_open);
    }
    COMPACT_VIRTUAL_DISK_PARAMETERS compactParameters;
    compactParameters.Version = COMPACT_VIRTUAL_DISK_VERSION_UNSPECIFIED;
    compactParameters.Version1.Reserved = 0;
    OVERLAPPED overlap;
    memset(&overlap, 0, sizeof(overlap));

    ProgressBar bar{
            option::BarWidth{50},
            option::Start{"["},
            option::Fill{"="},
            option::Lead{"="},
            option::Remainder{"-"},
            option::End{" ]"},
            option::ShowElapsedTime{true},
            option::ShowRemainingTime{true},
            option::Stream{std::cout},
    };

    indicators::show_console_cursor(false);

    DWORD result_compact = CompactVirtualDisk(handle, COMPACT_VIRTUAL_DISK_FLAG_NONE, &compactParameters, &overlap);
    DWORD dw_error = GetLastError();
    VIRTUAL_DISK_PROGRESS virtualDiskProgress = {0};
    LPCTSTR errMsg = nullptr;

    if (!result_compact)
    {
        if (dw_error == ERROR_IO_PENDING){
            BOOL bPending=TRUE;
            DWORD get_progress_result;

            // Loop until the I/O is complete, that is: the overlapped
            // event is signaled.

            while( bPending )
            {
                get_progress_result = GetVirtualDiskOperationProgress(handle, &overlap, &virtualDiskProgress);
                if (get_progress_result != ERROR_SUCCESS){
                    errMsg = utils::ErrorMessage(dw_error);
                    SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
                    LocalFree((LPVOID)errMsg);
                }
                if(virtualDiskProgress.OperationStatus != ERROR_IO_PENDING){
                    bPending = FALSE;
                }
                bar.set_option(option::MaxProgress{virtualDiskProgress.CompletionValue});
                bar.set_progress(virtualDiskProgress.CurrentValue);
                Sleep(100);
            }
        }else{
            errMsg = utils::ErrorMessage(dw_error);
            SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
            LocalFree((LPVOID)errMsg);
        }
    }
    CloseHandle(handle);

    indicators::show_console_cursor(true);
    return true;
}

std::string VirtDiskSystem::GetVdiskParent(std::string path){
    std::wstring w_path(path.begin(), path.end());
    const wchar_t *p_w = w_path.c_str();
    VIRTUAL_STORAGE_TYPE vst;
    vst.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    vst.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
    void *handle;
    OPEN_VIRTUAL_DISK_PARAMETERS OpenParameters;
    OpenParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    OpenParameters.Version2.GetInfoOnly = TRUE;
    DWORD result_open = OpenVirtualDisk(&vst, p_w,
                                        VIRTUAL_DISK_ACCESS_NONE,
                                        OPEN_VIRTUAL_DISK_FLAG_NONE,&OpenParameters, &handle);
    if (result_open != 0){
        SPDLOG_ERROR("Have error opening vhdx file, result %d", result_open);
        return "";
    }
    GET_VIRTUAL_DISK_INFO* disk_info;
    DWORD OutBufferSize = sizeof(GET_VIRTUAL_DISK_INFO);
    disk_info  = (GET_VIRTUAL_DISK_INFO*) malloc(OutBufferSize);
    if (disk_info == nullptr){
        SPDLOG_ERROR("error when malloc memory");
        return "";
    }
    // get disk type
    disk_info -> Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;
    GetVirtualDiskInformation(handle, &OutBufferSize, disk_info,
                              nullptr);
    if (disk_info -> ProviderSubtype != 4){
        return {};
    }
    free(disk_info);

    OutBufferSize = sizeof(GET_VIRTUAL_DISK_INFO);
    disk_info  = (GET_VIRTUAL_DISK_INFO*) malloc(OutBufferSize);
    if (disk_info == nullptr){
        SPDLOG_ERROR("error when malloc memory");
        return "";
    }
    // get parent disk
    disk_info -> Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;

    DWORD result = GetVirtualDiskInformation(handle, &OutBufferSize, disk_info,
                                             nullptr);
    if (result == ERROR_INSUFFICIENT_BUFFER){
        free(disk_info);
        disk_info  = (GET_VIRTUAL_DISK_INFO*) malloc(OutBufferSize);
        if (disk_info == nullptr){
            SPDLOG_ERROR("error when malloc memory");
            return "";
        }
        disk_info -> Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;
        GetVirtualDiskInformation(handle, &OutBufferSize, disk_info, nullptr);
    }
    if (disk_info -> ParentLocation.ParentResolved){
        return std::string{utils::LPWSTRTochar(disk_info -> ParentLocation.ParentLocationBuffer)};
    }
    return "";
}

std::vector<std::string> VirtDiskSystem::GetVdiskParents(const std::string& path){
    std::deque<std::string> out;
    out.push_back(path);
    for (std::deque<std::string>::size_type i = 0;i<out.size();++i){
        std::string p = GetVdiskParent(out[i]);
        if (!p.empty()){
            out.push_back(p);
        }
    }
    out.pop_front();
    return std::vector<std::string>{out.begin(), out.end()};
}

bool VirtDiskSystem::MergeVdiskToParent(std::string path){
    std::wstring w_path(path.begin(), path.end());
    const wchar_t *p_w = w_path.c_str();
    VIRTUAL_STORAGE_TYPE vst;
    vst.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    vst.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
    void *handle;
    OPEN_VIRTUAL_DISK_PARAMETERS OpenParameters;
    OpenParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
    OpenParameters.Version1.RWDepth = 1;
    DWORD result_open = OpenVirtualDisk(&vst, p_w,
                                        VIRTUAL_DISK_ACCESS_ALL,
                                        OPEN_VIRTUAL_DISK_FLAG_NONE,&OpenParameters, &handle);
    if (result_open != 0){
        SPDLOG_ERROR("Have error opening vhdx file, result %d", result_open);
        return FALSE;
    }
    GET_VIRTUAL_DISK_INFO* disk_info;
    DWORD OutBufferSize = sizeof(GET_VIRTUAL_DISK_INFO);
    disk_info  = (GET_VIRTUAL_DISK_INFO*) malloc(OutBufferSize);
    if (disk_info == nullptr){
        SPDLOG_ERROR("error when malloc memory");
        return FALSE;
    }
    // get disk type
    disk_info -> Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;
    GetVirtualDiskInformation(handle, &OutBufferSize, disk_info,
                              nullptr);
    if (disk_info -> ProviderSubtype != 4){
        return FALSE;
    }
    free(disk_info);
    MERGE_VIRTUAL_DISK_PARAMETERS mergeParameters;
    mergeParameters.Version = MERGE_VIRTUAL_DISK_VERSION_1;
    mergeParameters.Version1.MergeDepth = 1;

    OVERLAPPED overlap;
    memset(&overlap, 0, sizeof(overlap));
    DWORD result_merge = MergeVirtualDisk(handle, MERGE_VIRTUAL_DISK_FLAG_NONE, &mergeParameters, &overlap);

    DWORD dw_error = GetLastError();
    VIRTUAL_DISK_PROGRESS virtualDiskProgress = {0};
    LPCTSTR errMsg = nullptr;

    ProgressBar bar{
            option::BarWidth{50},
            option::Start{"["},
            option::Fill{"="},
            option::Lead{"="},
            option::Remainder{"-"},
            option::End{" ]"},
            option::ShowElapsedTime{true},
            option::ShowRemainingTime{true},
            option::Stream{std::cout},
    };

    if (!result_merge)
    {
        if (dw_error == ERROR_IO_PENDING){
            BOOL bPending=TRUE;
            DWORD get_progress_result;

            // Loop until the I/O is complete, that is: the overlapped
            // event is signaled.

            while( bPending )
            {
                get_progress_result = GetVirtualDiskOperationProgress(handle, &overlap, &virtualDiskProgress);
                if (get_progress_result != ERROR_SUCCESS){
                    errMsg = utils::ErrorMessage(dw_error);
                    SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
                    LocalFree((LPVOID)errMsg);
                }
                if(virtualDiskProgress.OperationStatus != ERROR_IO_PENDING){
                    bPending = FALSE;
                }
                bar.set_option(option::MaxProgress{virtualDiskProgress.CompletionValue});
                bar.set_progress(virtualDiskProgress.CurrentValue);
                Sleep(100);
            }
        }else{
            errMsg = utils::ErrorMessage(dw_error);
            SPDLOG_ERROR("Get Error (%d): %s\n", dw_error, errMsg);
            LocalFree((LPVOID)errMsg);
        }
    }

    CloseHandle(handle);

}

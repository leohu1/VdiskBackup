#include "VirtDiskSystem.h"
#include "utils.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <initguid.h>
#include <virtdisk.h>
#include <sddl.h>
#include <spdlog/spdlog.h>

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
                                        VIRTUAL_DISK_ACCESS_ATTACH_RO | VIRTUAL_DISK_ACCESS_METAOPS,
                                        OPEN_VIRTUAL_DISK_FLAG_NONE,&OpenParameters, &handle);
    if (result_open != 0){
        SPDLOG_ERROR("Have error opening vhdx file, result %d", result_open);
    }
    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
    DWORD result_attach = AttachVirtualDisk(handle, nullptr,
                                            ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME,
                                            0, &attachParameters, nullptr);
    if (result_attach != 0){
        SPDLOG_ERROR("Have error attaching vhdx file, result %d", result_open);
    }
    COMPACT_VIRTUAL_DISK_PARAMETERS compactParameters;
    compactParameters.Version = COMPACT_VIRTUAL_DISK_VERSION_UNSPECIFIED;
    compactParameters.Version1.Reserved = 0;
    OVERLAPPED overlap;
    memset(&overlap, 0, sizeof(overlap));
    DWORD result_compact = CompactVirtualDisk(handle, COMPACT_VIRTUAL_DISK_FLAG_NONE, &compactParameters, &overlap);
    DWORD dw_error = GetLastError();
    DWORD dwBytesRead;
    LPCTSTR errMsg = NULL;

    if (!result_compact)
    {
        switch (dw_error)
        {

            case ERROR_HANDLE_EOF:
            {
                break;
            }
            case ERROR_IO_PENDING:
            {
                BOOL bPending=TRUE;

                // Loop until the I/O is complete, that is: the overlapped
                // event is signaled.

                while( bPending )
                {
                    bPending = FALSE;

                    // Do something else then come back to check.
                    Sleep(1000);

                    // Check the result of the asynchronous read
                    // without waiting (forth parameter FALSE).
                    result_compact = GetOverlappedResult(handle,
                                                  &overlap,
                                                  &dwBytesRead,
                                                  FALSE) ;

                    if (!result_compact)
                    {
                        switch (dw_error = GetLastError())
                        {
                            case ERROR_IO_INCOMPLETE:
                            {
                                bPending = TRUE;
                                break;
                            }

                            default:
                            {
                                // Decode any other errors codes.
                                errMsg = ErrorMessage(dw_error);
                                SPDLOG_ERROR(TEXT("GetOverlappedResult failed (%d): %s\n"),
                                         dw_error, errMsg);
                                LocalFree((LPVOID)errMsg);
                            }
                        }
                    }
                    else
                    {
                        printf("ReadFile operation completed\n");
                    }
                }
                break;
            }

            default:
            {
                // Decode any other errors codes.
                errMsg = ErrorMessage(dw_error);
                SPDLOG_ERROR("ReadFile GLE unhandled (%d): %s\n", dw_error, errMsg);
                LocalFree((LPVOID)errMsg);
                break;
            }
        }
    }
    DWORD result_detach = DetachVirtualDisk(handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
    if (result_detach != 0){
        SPDLOG_ERROR("Have error detaching vhdx file, result %d", result_open);
    }
    CloseHandle(handle);
}

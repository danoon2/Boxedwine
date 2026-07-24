/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "fsfileopennode.h"
#include UNISTD
#include <fcntl.h>
#ifdef BOXEDWINE_MSVC
#include <Windows.h>
#include <io.h>
#endif
#include "fsfilenode.h"

FsFileOpenNode::FsFileOpenNode(const std::shared_ptr<FsFileNode>& node, U32 flags, U32 handle) : FsOpenNode(node, flags), fileNode(node), handle(handle) {
    openPositionedWriteHandle();
}

FsFileOpenNode::~FsFileOpenNode() {
    FsFileOpenNode::close();
}

S64 FsFileOpenNode::length() {
    S64 currentPos = lseek64(this->handle, 0, SEEK_CUR);
    S64 size = lseek64(this->handle, 0, SEEK_END);
    lseek64(this->handle, currentPos, SEEK_SET);
    return size;
}

bool FsFileOpenNode::setLength(S64 len) {
    if (len < 0) {
        return false;
    }
#ifdef BOXEDWINE_MSVC
    intptr_t nativeHandle = _get_osfhandle(this->handle);
    if (nativeHandle == -1) {
        errno = EBADF;
        return false;
    }
    FILE_END_OF_FILE_INFO endOfFile;
    endOfFile.EndOfFile.QuadPart = len;
    if (!SetFileInformationByHandle((HANDLE)nativeHandle, FileEndOfFileInfo, &endOfFile, sizeof(endOfFile))) {
        switch (GetLastError()) {
        case ERROR_ACCESS_DENIED:
            errno = EACCES;
            break;
        case ERROR_DISK_FULL:
            errno = ENOSPC;
            break;
        case ERROR_INVALID_HANDLE:
            errno = EBADF;
            break;
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        default:
            errno = EIO;
            break;
        }
        return false;
    }
    return true;
#else
    return ftruncate(this->handle, (off_t)len) == 0;
#endif
}

S64 FsFileOpenNode::getFilePointer() {
    return lseek64(this->handle, 0, SEEK_CUR);
}

S64 FsFileOpenNode::seek(S64 pos) {
    return lseek64(this->handle, pos, SEEK_SET);
}

void FsFileOpenNode::close() {
    if (this->positionedWriteHandle != 0xFFFFFFFF) {
        ::close(this->positionedWriteHandle);
        this->positionedWriteHandle = 0xFFFFFFFF;
    }
    if (this->handle!=0xFFFFFFFF)
        ::close(this->handle);
    this->handle = 0xFFFFFFFF;
}

bool FsFileOpenNode::isOpen() {
    return this->handle!=0xFFFFFFFF;
}

void FsFileOpenNode::reopen() {
    int openFlags = O_BINARY;
    int flags = this->flags;
                        
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        openFlags|=O_RDONLY;
    } else if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
        openFlags|=O_WRONLY;
    } else {
        openFlags|=O_RDWR;
    }
    if (flags & K_O_APPEND) {
        openFlags|=O_APPEND;
    }

    this->handle = ::open(this->fileNode->getNativePathForData().c_str(), openFlags, 0666);
    openPositionedWriteHandle();
}

void FsFileOpenNode::openPositionedWriteHandle() {
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
    if (this->handle == 0xFFFFFFFF || this->positionedWriteHandle != 0xFFFFFFFF ||
        !(this->flags & K_O_APPEND) || (this->flags & K_O_ACCMODE) == K_O_RDONLY) {
        return;
    }
    BString fdPath = B("/proc/self/fd/") + BString::valueOf(this->handle);
    int access = (this->flags & K_O_ACCMODE) == K_O_WRONLY ? O_WRONLY : O_RDWR;
    this->positionedWriteHandle = ::open(fdPath.c_str(), access | O_CLOEXEC | O_BINARY, 0666);
#endif
}

U32 FsFileOpenNode::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void FsFileOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kdebug("FsFileOpenNode::setAsync not implemented");
}

bool FsFileOpenNode::isAsync() {
    return false;
}

void FsFileOpenNode::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kdebug("FsFileOpenNode::::waitForEvents not implemented");
}

bool FsFileOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsFileOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsFileOpenNode::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsFileOpenNode::canMap() {
    return true;
}

U32 FsFileOpenNode::readNative(U8* buffer, U32 len) {
    return (U32)::read(this->handle, buffer, len);
}

U32 FsFileOpenNode::writeNative(U8* buffer, U32 len) {
    return (U32)::write(this->handle, buffer, len);
}

bool FsFileOpenNode::canWriteNativeAt() {
    if (!FsOpenNode::canWriteNativeAt()) {
        return false;
    }
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
    if ((this->flags & K_O_APPEND) && (this->flags & K_O_ACCMODE) != K_O_RDONLY) {
        return this->positionedWriteHandle != 0xFFFFFFFF;
    }
#endif
    return true;
}

FsWriteResult FsFileOpenNode::writeNativeAt(U8* buffer, U64 offset, U32 len) {
    FsWriteResult result;
#ifdef BOXEDWINE_MSVC
    if (offset > (U64)std::numeric_limits<S64>::max()) {
        result.error = -K_EINVAL;
        return result;
    }
    intptr_t rawHandle = _get_osfhandle(this->handle);
    if (rawHandle == -1) {
        result.error = -K_EBADF;
        return result;
    }
    HANDLE nativeHandle = (HANDLE)rawHandle;
    LARGE_INTEGER zero = {};
    LARGE_INTEGER previousOffset;
    LARGE_INTEGER targetOffset;
    targetOffset.QuadPart = (S64)offset;
    if (!SetFilePointerEx(nativeHandle, zero, &previousOffset, FILE_CURRENT) ||
        !SetFilePointerEx(nativeHandle, targetOffset, nullptr, FILE_BEGIN)) {
        result.error = -K_EIO;
        return result;
    }

    DWORD written = 0;
    BOOL writeSucceeded = WriteFile(nativeHandle, buffer, len, &written, nullptr);
    DWORD writeError = writeSucceeded ? ERROR_SUCCESS : GetLastError();
    BOOL restoreSucceeded = SetFilePointerEx(nativeHandle, previousOffset, nullptr, FILE_BEGIN);
    result.bytesWritten = written;
    if (!writeSucceeded) {
        switch (writeError) {
        case ERROR_ACCESS_DENIED: result.error = -K_EACCES; break;
        case ERROR_DISK_FULL: result.error = -K_ENOSPC; break;
        case ERROR_INVALID_HANDLE: result.error = -K_EBADF; break;
        case ERROR_INVALID_PARAMETER: result.error = -K_EINVAL; break;
        default: result.error = -K_EIO; break;
        }
    } else if (!restoreSucceeded) {
        result.error = -K_EIO;
    }
#else
    if (offset > (U64)std::numeric_limits<off_t>::max() ||
        len > (U64)std::numeric_limits<off_t>::max() - offset) {
        result.error = -K_EFBIG;
        return result;
    }
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
    U32 targetHandle = this->handle;
    if (this->flags & K_O_APPEND) {
        if (this->positionedWriteHandle == 0xFFFFFFFF) {
            result.error = -K_EIO;
            return result;
        }
        targetHandle = this->positionedWriteHandle;
    }
    ssize_t written = pwrite(targetHandle, buffer, len, (off_t)offset);
    if (written < 0) {
        result.error = -translateErr(errno);
    } else {
        result.bytesWritten = (U64)written;
    }
#else
    int originalFlags;
    do {
        originalFlags = fcntl(this->handle, F_GETFL);
    } while (originalFlags < 0 && errno == EINTR);
    if (originalFlags < 0) {
        result.error = -translateErr(errno);
        return result;
    }
    off_t previousOffset = lseek(this->handle, 0, SEEK_CUR);
    if (previousOffset == (off_t)-1) {
        result.error = -translateErr(errno);
        return result;
    }
    bool restoreAppend = (originalFlags & O_APPEND) != 0;
    if (restoreAppend) {
        int clearResult;
        do {
            clearResult = fcntl(this->handle, F_SETFL, originalFlags & ~O_APPEND);
        } while (clearResult < 0 && errno == EINTR);
        if (clearResult < 0) {
            result.error = -translateErr(errno);
            return result;
        }
    }

    int operationError = 0;
    ssize_t written = -1;
    if (lseek(this->handle, (off_t)offset, SEEK_SET) == (off_t)-1) {
        operationError = errno;
    } else {
        written = ::write(this->handle, buffer, len);
        if (written < 0) {
            operationError = errno;
        }
    }
    if (written > 0) {
        result.bytesWritten = (U64)written;
    }

    int offsetRestoreError = 0;
    if (lseek(this->handle, previousOffset, SEEK_SET) == (off_t)-1) {
        offsetRestoreError = errno;
    }
    int flagRestoreError = 0;
    if (restoreAppend) {
        int restoreResult;
        do {
            restoreResult = fcntl(this->handle, F_SETFL, originalFlags);
        } while (restoreResult < 0 && errno == EINTR);
        if (restoreResult < 0) {
            flagRestoreError = errno;
        }
    }
    if (operationError) {
        result.error = -translateErr(operationError);
    } else if (offsetRestoreError) {
        result.error = -translateErr(offsetRestoreError);
    } else if (flagRestoreError) {
        result.error = -translateErr(flagRestoreError);
    }
#endif
#endif
    return result;
}

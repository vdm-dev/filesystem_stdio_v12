#include <cstring>

#include "CBspInternalFile.h"

#include "ExLib.h"


CBspInternalFile::CBspInternalFile()
    : _hash(0)
    , _offset(0)
    , _size(0)
{
    memset(_name, 0, sizeof(_name));
}

CBspInternalFile::~CBspInternalFile()
{
    Close();
}

IFile::FileType CBspInternalFile::Type()
{
    return IFile::BspInternal;
}

bool CBspInternalFile::Open(const char* fileName, unsigned int mode)
{
    return false;
}

bool CBspInternalFile::Open(const char* fileName, unsigned int hash, unsigned int offset, unsigned int size, const char* bspName, bool text)
{
    if (_bsp.IsOpen() || _name[0] != '\0' || !fileName || offset == 0 || size == 0)
        return false;

    unsigned int mode = IFile::ReadOnly | IFile::ExistingOnly;
    if (text)
        mode |= IFile::Text;

    if (!_bsp.Open(bspName, mode))
        return false;

    exs_strcpy(_name, sizeof(_name), fileName);

    _hash = hash;
    _offset = offset;
    _size = size;

    _bsp.Seek(_offset, FILESYSTEM_SEEK_HEAD);

    return true;
}

void CBspInternalFile::Close()
{
    _bsp.Close();
    _hash = 0;
    _offset = 0;
    _size = 0;
    memset(_name, 0, sizeof(_name));
}

const char* CBspInternalFile::FileName() const
{
    return _name;
}

bool CBspInternalFile::IsOpen() const
{
    return _bsp.IsOpen() && (_size > 0) && (_offset > 0) && (_name[0] != '\0');
}

unsigned long CBspInternalFile::Seek(long position, FileSystemSeek_t origin)
{
    if (!_bsp.IsOpen())
        return 0;

    if (origin == FILESYSTEM_SEEK_HEAD)
    {
        position += static_cast<long>(_offset);
    }
    else if (origin == FILESYSTEM_SEEK_TAIL)
    {
        position += static_cast<long>(_offset + _size);
    }
    else
    {
        position += static_cast<long>(_bsp.Seek(0, FILESYSTEM_SEEK_CURRENT));
    }

    if (position < static_cast<long>(_offset))
        position = static_cast<long>(_offset);
    if (position > static_cast<long>(_offset + _size))
        position = static_cast<long>(_offset + _size);

    return _bsp.Seek(position, FILESYSTEM_SEEK_HEAD) - _offset;
}

unsigned long CBspInternalFile::Size()
{
    return _bsp.IsOpen() ? _size : 0;
}

void CBspInternalFile::Flush()
{
}

unsigned long CBspInternalFile::Read(void* data, unsigned long size)
{
    unsigned long position = Seek(0, FILESYSTEM_SEEK_CURRENT);
    if (!_bsp.IsOpen() || position >= _size)
        return 0;

    if (position + size > _size)
        size = _size - position;

    return _bsp.Read(data, size);
}

unsigned long CBspInternalFile::Write(const void* data, unsigned long size)
{
    return 0;
}

bool CBspInternalFile::GetFileTime(unsigned __int64* creationTime, unsigned __int64* lastAccessTime, unsigned __int64* lastWriteTime)
{
    return _bsp.GetFileTime(creationTime, lastAccessTime, lastWriteTime);
}

#include <zip_uncompressed.h>

#include "ExLib.h"

#include "CBspArchive.h"
#include "CPath.h"
#include "CBspInternalFile.h"


#define IDBSPHEADER (('P' << 24) + ('S' << 16) + ('B' << 8) + 'V') // little-endian "VBSP"
#define BSPVERSION 19
#define	HEADER_LUMPS 64


enum
{
	// A pak file can be embedded in a .bsp now, and the file system will search the pak
	//  file first for any referenced names, before deferring to the game directory 
	//  file system/pak files and finally the base directory file system/pak files.
	LUMP_PAKFILE = 40
};

struct BspLump
{
	int		fileofs, filelen;
	int		version;		// default to zero
	char	fourCC[4];		// default to ( char )0, ( char )0, ( char )0, ( char )0
};


struct BspHeader
{
	int     ident;
	int     version;
	BspLump lumps[HEADER_LUMPS];
	int     mapRevision;				// the map's revision (iteration, version) number (added BSPVERSION 6)
};


CBspArchive::CBspArchive()
	: _file(NULL)
{
}

CBspArchive::~CBspArchive()
{
	Clear();
}

bool CBspArchive::IsOpen() const
{
	return _file && _file->IsOpen() && _table.size() > 0;
}

bool CBspArchive::SetFile(CFile* file)
{
	if (!file)
		return false;

	file->Seek(0, FILESYSTEM_SEEK_HEAD);

	BspHeader header;
	if (file->Read(&header, sizeof(header)) != sizeof(header))
		return false;

	if (header.ident != IDBSPHEADER || header.version != BSPVERSION)
		return false;

	int zipLength = header.lumps[LUMP_PAKFILE].filelen;
	int zipOffset = header.lumps[LUMP_PAKFILE].fileofs;

	if (zipLength <= sizeof(BspLump))
		return false;

	bool found = false;
	ZIP_EndOfCentralDirRecord cdr;
	memset(&cdr, 0, sizeof(cdr));

	for (int offset = zipLength - sizeof(ZIP_EndOfCentralDirRecord); offset >= 0; offset--)
	{
		file->Seek(zipOffset + offset, FILESYSTEM_SEEK_HEAD);
		file->Read(&cdr, sizeof(cdr));

		if (cdr.signature == 0x06054b50)
		{
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	if (_file)
	{
		if (_stricmp(_file->FileName(), file->FileName()) == 0)
			return true;

		delete _file;
	}

	_file = file;
	_table.clear();

	file->Seek(zipOffset + cdr.startOfCentralDirOffset, FILESYSTEM_SEEK_HEAD);

	ZIP_FileHeader fileHeader;
	ZIP_LocalFileHeader localHeader;
	InternalFile fileRecord;

	for (int i = 0; i < cdr.nCentralDirectoryEntries_Total; i++)
	{
		memset(&fileHeader, 0, sizeof(fileHeader));
		memset(&localHeader, 0, sizeof(localHeader));
		memset(&fileRecord, 0, sizeof(fileRecord));

		file->Read(&fileHeader, sizeof(fileHeader));
		if (fileHeader.signature != 0x02014b50 || fileHeader.compressionMethod != 0)
			return true;

		unsigned long nextOffset = file->Seek(0, FILESYSTEM_SEEK_CURRENT) +
			fileHeader.fileNameLength + fileHeader.extraFieldLength + fileHeader.fileCommentLength;

		unsigned long toRead = fileHeader.fileNameLength;
		if (toRead > sizeof(fileRecord.name) - 1)
			toRead = sizeof(fileRecord.name) - 1;

		file->Read(fileRecord.name, toRead);
		CPath::Normalize(fileRecord.name, toRead);

		fileRecord.hash = ex_crc32(fileRecord.name, toRead);
		fileRecord.size = fileHeader.compressedSize;

		file->Seek(zipOffset + fileHeader.relativeOffsetOfLocalHeader, FILESYSTEM_SEEK_HEAD);
		file->Read(&localHeader, sizeof(localHeader));
		if (localHeader.signature != 0x04034b50)
			return true;

		fileRecord.offset = file->Seek(0, FILESYSTEM_SEEK_CURRENT) +
			localHeader.fileNameLength + localHeader.extraFieldLength;

		_table.insert(std::make_pair(fileRecord.hash, fileRecord));

		file->Seek(nextOffset, FILESYSTEM_SEEK_HEAD);
	}

    return true;
}

void CBspArchive::Clear()
{
	if (_file)
	{
		delete _file;
		_file = NULL;
	}
	_table.clear();
}

const char* CBspArchive::FileName() const
{
	return IsOpen() ? _file->FileName() : NULL;
}

bool CBspArchive::FileExists(const char* fileName, const char* group) const
{
	if (!IsOpen() || !fileName)
		return false;

	char normalized[260];
	exs_strcpy(normalized, sizeof(normalized), fileName);
	size_t length = strlen(normalized);
	CPath::Normalize(normalized, length);

	unsigned int hash = ex_crc32(normalized, length);

	auto range = _table.equal_range(hash);
	for (auto it = range.first; it != range.second; ++it)
	{
		if (strcmp(it->second.name, normalized) == 0)
			return true;
	}

	return false;
}

IFile* CBspArchive::OpenInternal(const char* fileName, const char* group, bool text) const
{
	if (!IsOpen() || !fileName)
		return NULL;

	char normalized[260];
	exs_strcpy(normalized, sizeof(normalized), fileName);
	size_t length = strlen(normalized);
	CPath::Normalize(normalized, length);

	unsigned int hash = ex_crc32(normalized, length);

	auto range = _table.equal_range(hash);
	for (auto it = range.first; it != range.second; ++it)
	{
		if (strcmp(it->second.name, normalized) == 0)
		{
			CBspInternalFile* file = new CBspInternalFile();
			if (file)
			{
				if (file->Open(it->second.name, it->second.hash, it->second.offset, it->second.size, _file->FileName(), text))
					return file;

				delete file;
			}
		}
	}

	return NULL;
}

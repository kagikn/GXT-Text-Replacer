#ifndef __GXTBUILD_H
#define __GXTBUILD_H

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <cstdint>
#include <cctype>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <forward_list>
#include <shlwapi.h>
#include <ctime>
#include "utf8.h"
#include <memory>

using namespace std;

#define CHARACTER_MAP_SIZE		224

enum eGXTVersion
{
	GXT_III,	// Unsupported
	GXT_VC,
	GXT_SA
};

class GXTTableBase
{
public:
	string						szPath;
	string						Content;

	GXTTableBase(const string& szFilePath)
		: szPath(szFilePath)
	{}

	virtual ~GXTTableBase()
	{}

public:
	virtual bool	InsertEntry( const std::string& entryName, size_t offset ) = 0;
	virtual size_t	GetNumEntries() = 0;
	virtual size_t	GetFormattedContentSize() = 0;
	virtual size_t	GetEntrySize() = 0;
	virtual void	WriteOutEntries( std::ostream& stream ) = 0;
	virtual void	WriteOutContent( std::ostream& stream ) = 0;
	virtual void	PushFormattedChar( int character ) = 0;
};

struct EntryName
{
	char						cName[8];

	EntryName(const char* pName)
	{
		memset(cName, 0, sizeof(cName));
		strncpy(cName, pName, 8);
	}
};

struct VersionControlMap
{
	uint32_t					TextHash;
	bool						bLinked;

	VersionControlMap(uint32_t hash = 0)
		: TextHash(hash), bLinked(true)
	{}
};

namespace VC
{
	class GXTTable : public GXTTableBase
	{
	public:
		GXTTable( const string& szFilePath )
			: GXTTableBase( szFilePath )
		{}

		virtual size_t	GetNumEntries() override
		{
			return Entries.size();
		}

		virtual size_t GetFormattedContentSize() override
		{
			return FormattedContent.size() * sizeof(character_t);
		}

		virtual size_t GetEntrySize() override
		{
			return GXT_ENTRY_NAME_LEN + sizeof(size_t);
		}
	
		virtual bool	InsertEntry( const std::string& entryName, size_t offset ) override;
		virtual void	WriteOutEntries( std::ostream& stream ) override;
		virtual void	WriteOutContent( std::ostream& stream ) override;
		virtual void	PushFormattedChar( int character ) override;

	private:
		typedef uint16_t character_t;
		static const size_t		GXT_ENTRY_NAME_LEN = 8;

		map<std::string, size_t>	Entries;
		basic_string<character_t>		FormattedContent;
	};
};

namespace SA
{
	class GXTTable : public GXTTableBase
	{
	public:
		GXTTable( const string& szFilePath )
			: GXTTableBase( szFilePath )
		{}

		virtual size_t	GetNumEntries() override
		{
			return Entries.size();
		}

		virtual size_t GetFormattedContentSize() override
		{
			return FormattedContent.size() * sizeof(character_t);
		}

		virtual size_t GetEntrySize() override
		{
			return sizeof(uint32_t) + sizeof(size_t);
		}

		virtual bool	InsertEntry( const std::string& entryName, size_t offset ) override;
		virtual void	WriteOutEntries( std::ostream& stream ) override;
		virtual void	WriteOutContent( std::ostream& stream ) override;
		virtual void	PushFormattedChar( int character ) override;

	private:
		typedef uint8_t character_t;

		map<uint32_t, size_t>		Entries;
		basic_string<character_t>		FormattedContent;
	};
};

typedef std::map<EntryName, std::unique_ptr<GXTTableBase>, bool(*)(const EntryName&,const EntryName&)>	tableMap_t;

#endif
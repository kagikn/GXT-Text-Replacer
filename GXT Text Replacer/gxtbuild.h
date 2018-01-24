#ifndef __GXTBUILD_H
#define __GXTBUILD_H

#include <string>
#include <map>
#include <memory>
#include <strsafe.h>
#include <intrin.h>

enum eGXTVersion
{
	GXT_III,	// Unsupported
	GXT_VC,
	GXT_SA,
	GXT_SA_MOBILE
};

class GXTTableBase
{
public:
	std::wstring			szPath;
	std::string				Content;

	GXTTableBase(std::wstring szFilePath)
		: szPath(std::move(szFilePath))
	{}
    GXTTableBase() 
    {}
	virtual ~GXTTableBase()
	{}

public:
	virtual bool	InsertEntry( const std::string& entryName, uint32_t offset ) = 0;
    virtual bool	InsertEntry(const uint32_t crc32EntryHash, uint32_t offset) = 0;
    virtual bool	UsesHashForEntryName() = 0;
	virtual size_t	GetNumEntries() = 0;
	virtual size_t	GetFormattedContentSize() = 0;
	virtual size_t	GetEntrySize() = 0;
	virtual void	WriteOutEntries( std::ostream& stream ) = 0;
	virtual void	WriteOutContent( std::ostream& stream ) = 0;
    virtual void	ReadEntireContent(std::ifstream& inputStream, const uint32_t offset, const size_t size) = 0;
	virtual void	PushFormattedChar( int character ) = 0;

    static std::unique_ptr<GXTTableBase> InstantiateGXTTable(eGXTVersion version);

private:

};


class GXTTableBlockInfo
{
public:
    uint32_t			_absoluteOffset = 0;
    std::unique_ptr<GXTTableBase>				_GXTTable;

    GXTTableBlockInfo(eGXTVersion fileVersion)
    {
        _GXTTable = std::move(GXTTableBase::InstantiateGXTTable(fileVersion));
    }

    GXTTableBlockInfo(uint32_t absoluteOffset, const eGXTVersion fileVersion)
    {
        _absoluteOffset = absoluteOffset;
        _GXTTable = std::move(GXTTableBase::InstantiateGXTTable(fileVersion));
    }

    GXTTableBlockInfo(GXTTableBlockInfo&& r)
    {
        _GXTTable = std::move(r._GXTTable);
    }
    GXTTableBlockInfo& operator=(GXTTableBlockInfo&& r)
    {
        _GXTTable = std::move(r._GXTTable);
        return *this;
    }
};

class GXTTableCollection
{
public:
    std::pair<std::string, GXTTableBlockInfo> _mainTable;
    std::map<std::string, GXTTableBlockInfo> _missionTable;

    GXTTableCollection(std::pair<std::string, GXTTableBlockInfo> mainTable);
    GXTTableCollection(std::pair<std::string, GXTTableBlockInfo> mainTable, std::map<std::string, GXTTableBlockInfo> missionTable);

    std::pair<std::string, GXTTableBlockInfo>& GetMainTablePair()
    {
        return _mainTable;
    }
    std::map<std::string, GXTTableBlockInfo>& GetMissionTableMap()
    {
        return _missionTable;
    }

    /*GXTTableCollection(GXTTableCollection&& r)
    {
        _mainTable = std::move(r._mainTable);
        _missionTable = std::move(r._missionTable);
    }
    GXTTableCollection& operator=(GXTTableCollection&& r)
    {
        _mainTable = std::move(r._mainTable);
        _missionTable = std::move(r._missionTable);
        return *this;
    }*/
};

struct EntryName
{
	static const size_t		GXT_TABLE_NAME_LEN = 8;

	char						cName[GXT_TABLE_NAME_LEN];

	EntryName(const char* pName)
	{
		StringCchCopyNExA( cName, _countof(cName), pName, GXT_TABLE_NAME_LEN, nullptr, nullptr, STRSAFE_FILL_BEHIND_NULL );
	}

	EntryName(const wchar_t* pName)
	{
		size_t numConverted = 0;
		wcstombs_s( &numConverted, cName, pName, _countof(cName) );
		std::fill( std::begin(cName)+numConverted, std::end(cName), 0 ); 
	}
};

typedef std::map<EntryName, std::unique_ptr<GXTTableBase>, bool(*)(const EntryName&,const EntryName&)>	tableMap_t;

class GXTFileBase
{
public:
	static std::unique_ptr<GXTFileBase> InstantiateBuilder( eGXTVersion version );

	void ProduceGXTFile( const std::wstring& szLangName, const tableMap_t& TablesMap );

private:
	virtual uint32_t WriteOutHeader( std::ostream& stream ) const = 0;
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
		typedef uint16_t character_t;

		GXTTable( std::wstring szFilePath )
			: GXTTableBase( std::move(szFilePath) )
		{}
        GXTTable() : GXTTableBase()
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
			return GXT_ENTRY_NAME_LEN + sizeof(uint32_t);
		}

        virtual bool UsesHashForEntryName() override
        {
            return false;
        }
        virtual bool InsertEntry(const uint32_t, uint32_t) override
        {
            return false;
        }
	
		virtual bool	InsertEntry( const std::string& entryName, uint32_t offset ) override;
		virtual void	WriteOutEntries( std::ostream& stream ) override;
		virtual void	WriteOutContent( std::ostream& stream ) override;
        virtual void	ReadEntireContent(std::ifstream& inputStream, const uint32_t offset, const size_t size) override;
		virtual void	PushFormattedChar( int character ) override;

	private:
		static const size_t		GXT_ENTRY_NAME_LEN = 8;

		std::map<std::string, uint32_t>	Entries;
		std::basic_string<character_t>	FormattedContent;
	};

	class GXTFile : public GXTFileBase
	{
	private:
		virtual uint32_t WriteOutHeader( std::ostream& ) const override
		{
			// No header
			return 0;
		}
	};
};

namespace SA
{
	template<typename Character>
	class GXTTable : public GXTTableBase
	{
	public:
		typedef Character character_t;

		GXTTable( std:: wstring szFilePath )
			: GXTTableBase( std::move(szFilePath) )
		{}
        GXTTable() : GXTTableBase()
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
			return sizeof(uint32_t) + sizeof(uint32_t);
		}

        virtual bool UsesHashForEntryName() override
        {
            return true;
        }

		virtual bool	InsertEntry( const std::string& entryName, uint32_t offset ) override;
        virtual bool	InsertEntry(const uint32_t crc32EntryHash, uint32_t offset) override;
		virtual void	WriteOutEntries( std::ostream& stream ) override;
		virtual void	WriteOutContent( std::ostream& stream ) override;
        virtual void	ReadEntireContent(std::ifstream& inputStream, const uint32_t offset, const size_t size) override;
		virtual void	PushFormattedChar( int character ) override;

	private:
		std::map<uint32_t, uint32_t>	Entries;
		std::basic_string<character_t>	FormattedContent;
	};

	template<typename Character>
	class GXTFile : public GXTFileBase
	{


	private:
		virtual uint32_t WriteOutHeader( std::ostream& stream ) const override
		{
			const char		header[] = { 0x04, 0x00, sizeof(GXTTable<typename Character>::character_t) * 8, 0x00 };	// 0x080004
			stream.write(header, sizeof(header));
			return sizeof(header);
		}
	};
};

#endif
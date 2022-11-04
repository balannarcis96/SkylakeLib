//!
//! \file Structure.h
//! 
//! \brief Skylake Datacenter abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <vector>
#include <unordered_map>
#include <cstring>

namespace DatacenterTests
{
    class DatacenterTestsFixture;
}

namespace SKL::DC
{
    using TVersion       = uint32_t;
    using TFormatVersion = uint32_t;
    using TBlockIndex    = uint16_t;
    using TStringIndex   = uint32_t;
    using TNameIndex     = TStringIndex;
    using TBlockIndices  = std::pair<TBlockIndex, TBlockIndex>;
    using TStringIndices = TBlockIndices;
    using TStringRef     = const wchar_t*;
    using Stream         = StreamBase;
    using AttributeValue = TBlockIndices;
    using TLanguage      = int16_t;

    constexpr TLanguage      CInvalidLanguage       = -1;
    constexpr TLanguage      CNoSpecificLanguage    = 0;
    constexpr TLanguage      CInternationalLanguage = 1;
    constexpr TVersion       CInvalidVersion        = 0;
    constexpr TFormatVersion CInvalidFormatVersion  = 0;
    constexpr size_t         CElementsBlockSize     = std::numeric_limits<TBlockIndex>::max();
    constexpr size_t         CAttributesBlockSize   = std::numeric_limits<TBlockIndex>::max();
    constexpr size_t         CStringsBlockSize      = std::numeric_limits<TBlockIndex>::max();
    constexpr TStringIndex   CInvalidStringIndex    = std::numeric_limits<TStringIndex>::max();
    constexpr TBlockIndex    CInvalidBlockIndex     = std::numeric_limits<TBlockIndex>::max();

    constexpr TFormatVersion CCurrentFormatVersion  = 0x01000000;

	inline bool CWStringStartsWith( const wchar_t* InPre, size_t InPreLength, const wchar_t* InString ) noexcept
	{
		const size_t StrLength{ wcslen( InString ) };
		return StrLength < StrLength ? false : 0 == memcmp( InPre, InString, InPreLength );
	}

    template<bool bEnableBuildCapabilities>
    struct Datacenter
    {
        using MyType = Datacenter<bEnableBuildCapabilities>;
        
        static constexpr size_t GetAttributeSerialSize() noexcept
        {
            return 
                  sizeof( TNameIndex ) 
                + sizeof( TBlockIndices );
        }
        static constexpr size_t GetElementSerialSize() noexcept
        {
            return 
                  sizeof( TNameIndex ) 
                + sizeof( TBlockIndices )
                + sizeof( uint16_t )
                + sizeof( TBlockIndices )
                + sizeof( uint16_t )
                + sizeof( TBlockIndices );
        }

        struct EmptyType{};

        struct AttributeEditData
        {
            uint64_t      Hash          { 0 };
            TBlockIndices CachedLocation{ CInvalidBlockIndex, CInvalidBlockIndex };
        };
        struct Attribute : std::conditional_t<bEnableBuildCapabilities, AttributeEditData, EmptyType>
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if ( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    
                    NameIndex = Reader->ReadT<decltype( NameIndex )>();
                    Value     = Reader->ReadT<decltype( Value )>();
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    
                    Writer->WriteT( NameIndex );
                    Writer->WriteT( Value );
                }

                return true;
            }
            
			template<size_t NameSize>
			SKL_FORCEINLINE bool IsNamed( const wchar_t( &InName ) [ NameSize ] )const noexcept
			{
                SKL_ASSERT( nullptr != CachedNameRef );
				return 0 == SKL_WSTRICMP( InName, CachedNameRef, NameSize );
			}
			template<size_t StringSize>
			SKL_FORCEINLINE bool IsValue( const wchar_t( &InString ) [ StringSize ] )const noexcept
			{
                SKL_ASSERT( nullptr != CachedValueRef );
				return 0 == SKL_WSTRCMP( InString, CachedValueRef, StringSize );
			}

            SKL_FORCEINLINE float GetFloat() const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstof( CachedValueRef, nullptr );
            }
            SKL_FORCEINLINE double GetDouble() const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstod( CachedValueRef, nullptr );
            }
            SKL_FORCEINLINE int32_t GetInt( int32_t Radix = 10 ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstol( CachedValueRef, nullptr, Radix );
            }
            SKL_FORCEINLINE uint32_t GetUInt( int32_t Radix = 10 ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstoul( CachedValueRef, nullptr, Radix );
            }
            SKL_FORCEINLINE int64_t GetInt64( int32_t Radix = 10 ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstoll( CachedValueRef, nullptr, Radix );
            }
            SKL_FORCEINLINE uint64_t GetUInt64( int32_t Radix = 10 ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return wcstoull( CachedValueRef, nullptr, Radix );
            }
            SKL_FORCEINLINE bool GetBool() const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                //@ TODO Port to other than WIN32
                return 0 == _wcsnicmp( CachedValueRef, L"true", 4 ) || 0 == _wcsnicmp( CachedValueRef, L"1", 1 );
            }
            SKL_FORCEINLINE std::wstring GetWString() const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
                return { CachedValueRef };
            }
            
            SKL_FORCEINLINE bool Get2DPoint( float* Out2DPoint, const wchar_t* Delimiter = L"," ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
				std::vector<std::wstring> Parts = SplitString( GetWString( ), std::wstring{ Delimiter }, true, false );
				if ( Parts.size( ) != 2 )
				{
					return false;
				}

				Out2DPoint[0] = wcstof( Parts [0].c_str( ), nullptr );
				Out2DPoint[1] = wcstof( Parts [1].c_str( ), nullptr );

                return true;
            }
            SKL_FORCEINLINE bool Get3DPoint( float* Out3DPoint, const wchar_t* Delimiter = L"," ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
				std::vector<std::wstring> Parts = SplitString( GetWString( ), std::wstring{ Delimiter }, true, false );
				if ( Parts.size( ) != 3 )
				{
					return false;
				}

				Out3DPoint[0] = wcstof( Parts [0].c_str( ), nullptr );
				Out3DPoint[1] = wcstof( Parts [1].c_str( ), nullptr );
				Out3DPoint[2] = wcstof( Parts [2].c_str( ), nullptr );

                return true;
            }
            SKL_FORCEINLINE bool Get2DPointD( double* Out2DPoint, const wchar_t* Delimiter = L"," ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
				std::vector<std::wstring> Parts = SplitString( GetWString( ), std::wstring{ Delimiter }, true, false );
				if ( Parts.size( ) != 2 )
				{
					return false;
				}

				Out2DPoint[0] = wcstod( Parts [0].c_str( ), nullptr );
				Out2DPoint[1] = wcstod( Parts [1].c_str( ), nullptr );

                return true;
            }
            SKL_FORCEINLINE bool Get3DPointD( double* Out3DPoint, const wchar_t* Delimiter = L"," ) const noexcept
            {
                SKL_ASSERT( nullptr != CachedValueRef );
				std::vector<std::wstring> Parts = SplitString( GetWString( ), std::wstring{ Delimiter }, true, false );
				if ( Parts.size( ) != 3 )
				{
					return false;
				}

				Out3DPoint[0] = wcstod( Parts [0].c_str( ), nullptr );
				Out3DPoint[1] = wcstod( Parts [1].c_str( ), nullptr );
				Out3DPoint[2] = wcstod( Parts [2].c_str( ), nullptr );

                return true;
            }

            SKL_FORCEINLINE TStringRef GetName() const noexcept 
            {
                return CachedNameRef;
            }
            SKL_FORCEINLINE TStringRef GetValue() const noexcept
            {
                return CachedValueRef;
            }
            
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, AttributeEditData>>
            SKL_FORCEINLINE TRet& GetEditData() noexcept { return *this; };
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, AttributeEditData>>
            SKL_FORCEINLINE const TRet& GetEditData() const noexcept { return *this; };

            SKL_FORCEINLINE void SetNameIndex( TNameIndex InNameIndex ) noexcept { NameIndex = InNameIndex; }
            SKL_FORCEINLINE void SetValueIndices( TBlockIndices InValueIndices ) noexcept { Value = InValueIndices; }

        private:
            TNameIndex     NameIndex{ CInvalidStringIndex };
            TBlockIndices  Value    { CInvalidBlockIndex, CInvalidBlockIndex };

            TStringRef             CachedNameRef { nullptr };
            mutable const wchar_t* CachedValueRef{ nullptr };

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        struct ElementEditData
        {
            TBlockIndices CachedLocation{ CInvalidBlockIndex,CInvalidBlockIndex  };
            TBlockIndices ParentLocation{ CInvalidBlockIndex,CInvalidBlockIndex  };
            uint32_t      RefCount        { 1 };
            
            SKL_FORCEINLINE void AddRef() noexcept { ++RefCount; }
            SKL_FORCEINLINE bool RemoveRef() noexcept 
            { 
                SKL_ASSERT( 0 < RefCount );
                0 == --RefCount; 
            }
        };
        struct Element : std::conditional_t<bEnableBuildCapabilities, ElementEditData, EmptyType>
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if ( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    
                    NameIndex        = Reader->ReadT<decltype( NameIndex )>();
                    ValueIndices     = Reader->ReadT<decltype( ValueIndices )>();
                    AttributesCount  = Reader->ReadT<decltype( AttributesCount )>();
                    AttributeIndices = Reader->ReadT<decltype( AttributeIndices )>();
                    ChildrenCount    = Reader->ReadT<decltype( ChildrenCount )>();
                    ChildrenIndices  = Reader->ReadT<decltype( ChildrenIndices )>();
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    
                    Writer->WriteT( NameIndex );
                    Writer->WriteT( ValueIndices );
                    Writer->WriteT( AttributesCount );
                    Writer->WriteT( AttributeIndices );
                    Writer->WriteT( ChildrenCount );
                    Writer->WriteT( ChildrenIndices );
                }

                return true;
            }
            
			template<size_t NameSize>
			SKL_FORCEINLINE bool IsNamed( const wchar_t( &InName ) [ NameSize ] )const noexcept
			{
                SKL_ASSERT( nullptr != CachedNameRef );
				return 0 == SKL_WSTRICMP( InName, CachedNameRef, NameSize );
			}
            
			template<size_t StrSize>
			SKL_FORCEINLINE bool IsValue( const wchar_t( &InString ) [ StrSize ] )const noexcept
			{
				if( false == HasValueString() ){ return false; }
                SKL_ASSERT( nullptr != CachedValueRef );
				return 0 == SKL_WSTRICMP( InString, CachedValueRef, StrSize );
			}

            SKL_FORCEINLINE TStringRef GetName() const noexcept 
            {
                return CachedNameRef;
            }
            SKL_FORCEINLINE TStringRef GetValue() const noexcept 
            {
                return CachedValueRef;
            }
            SKL_FORCEINLINE const std::vector<Attribute*>& GetAttributes() const noexcept
            {
                return CachedAttributes;
            }
            SKL_FORCEINLINE const std::vector<Element*>& GetChildren() const noexcept
            {
                return CachedChildren;
            }
            SKL_FORCEINLINE Element* GetParent() const noexcept{ return Parent; }

            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, ElementEditData>>
            SKL_FORCEINLINE TRet& GetEditData() noexcept { return *this; };
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, ElementEditData>>
            SKL_FORCEINLINE const TRet& GetEditData() const noexcept { return *this; };
            SKL_FORCEINLINE bool HasValueString() const noexcept { return CInvalidBlockIndex != ValueIndices.first && CInvalidBlockIndex != ValueIndices.second; }

            SKL_FORCEINLINE void SetChildrenCount( uint32_t InCount ) noexcept
            {
                SKL_ASSERT( true == CachedChildren.empty() );
                ChildrenCount = static_cast<uint16_t>( InCount );
            }
            SKL_FORCEINLINE void SetAttributesCount( uint32_t InCount ) noexcept
            {
                SKL_ASSERT( true == CachedAttributes.empty() );
                AttributesCount = static_cast<uint16_t>( InCount );
            }
            SKL_FORCEINLINE void SetNameIndex( TNameIndex InIndex ) noexcept { NameIndex = InIndex; }
            SKL_FORCEINLINE void SetValueIndices( TBlockIndices InValueIndices ) noexcept { ValueIndices = InValueIndices; }
            SKL_FORCEINLINE void SetChildrenIndices( TBlockIndices InIndices ) noexcept { ChildrenIndices = InIndices; }
            SKL_FORCEINLINE void SetAttributesIndices( TBlockIndices InIndices ) noexcept { AttributeIndices = InIndices; }
        private:
            TNameIndex     NameIndex         { CInvalidStringIndex };
            uint16_t       AttributesCount   { 0 };
            TBlockIndices  AttributeIndices  { CInvalidBlockIndex, CInvalidBlockIndex };
            uint16_t       ChildrenCount     { 0 };
            TBlockIndices  ChildrenIndices   { CInvalidBlockIndex, CInvalidBlockIndex };
            TBlockIndices  ValueIndices      { CInvalidBlockIndex, CInvalidBlockIndex };

            TStringRef              CachedNameRef   { nullptr };
            TStringRef              CachedValueRef  { nullptr };
            std::vector<Attribute*> CachedAttributes{};
            std::vector<Element*>   CachedChildren  {};
            Element*                Parent          { nullptr };

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        template<typename T>
        struct Array
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if ( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    
                    Count = Reader->ReadT<decltype( Count )>();

                    for( uint32_t i = 0; i < Count; ++i )
                    {
                        T Object;

                        if( false == Object.Serialize( InStream, true ) )
                        {
                            SKLL_TRACE_MSG_FMT( "Failed to deserialize item %u", i );                            
                            return false;
                        }

                        Data.push_back( std::move( Object ) );
                    }
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    
                    Writer->WriteT( Count );

                    SKL_ASSERT( Data.size() == Count );

                    for( uint32_t i = 0; i < Count; ++i )
                    {
                        if( false == Data[i].Serialize( InStream, false ) )
                        {
                            SKLL_TRACE_MSG_FMT( "Failed to serialize item %u", i );                            
                            return false;
                        }
                    }
                }

                return true;
            }
            void Clear() noexcept
            {
                Data.clear( );
                Count = 0;
            }

            SKL_FORCEINLINE T& operator[]( size_t InIndex ) noexcept 
            {
                SKL_ASSERT( InIndex < Data.size() );
                SKL_ASSERT( InIndex < Count );

                return Data[ InIndex ];
            }
            SKL_FORCEINLINE const T& operator[]( size_t InIndex ) const noexcept 
            {
                SKL_ASSERT( InIndex < Data.size() );
                SKL_ASSERT( InIndex < Count );

                return Data[ InIndex ];
            }
            SKL_FORCEINLINE T& Last() noexcept 
            { 
                SKL_ASSERT( false == Data.empty() );
                return Data[ Data.size() - 1 ];
            }
            SKL_FORCEINLINE const T& Last() const noexcept 
            { 
                SKL_ASSERT( false == Data.empty() );
                return Data[ Data.size() - 1 ];
            }
            SKL_FORCEINLINE uint32_t Size() const noexcept
            {
                SKL_ASSERT( Data.size() == Count );
                return Count;
            }
            SKL_FORCEINLINE void AddItem( T&& InItem ) noexcept
            {
                SKL_ASSERT( Data.size() == Count );
                Data.push_back( std::forward<T>( InItem ) );
                ++Count;
            }
        private:
            uint32_t       Count{ 0 };
            std::vector<T> Data;

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };
    
        template<typename T, uint32_t ElementSerialSize>
        struct BlockArray
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    
                    TotalBlockCount     = Reader->ReadT<decltype( TotalBlockCount )>();
                    TotalUsedBlockCount = Reader->ReadT<decltype( TotalUsedBlockCount )>();

                    for( uint32_t i = 0; i < TotalUsedBlockCount; ++i )
                    {
                        T Object;

                        if( false == Object.Serialize( InStream, true ) )
                        {
                            SKLL_TRACE_MSG_FMT( "Failed to deserialize item %u", i );                            
                            return false;
                        }

                        Data.push_back( std::move( Object ) );
                    }
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    
                    Writer->WriteT( TotalBlockCount );
                    Writer->WriteT( TotalUsedBlockCount );

                    SKL_ASSERT( Data.size() == TotalUsedBlockCount );

                    for( uint32_t i = 0; i < TotalUsedBlockCount; ++i )
                    {
                        if( false == Data[i].Serialize( InStream, false ) )
                        {
                            SKLL_TRACE_MSG_FMT( "Failed to serialize item %u", i );                            
                            return false;
                        }
                    }
                }

                uint32_t Remaining{ TotalBlockCount - TotalUsedBlockCount };

                if( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    if( false == Reader->TryForward( Remaining * ElementSerialSize ) )
                    {
                        SKLL_TRACE_MSG( "No space left in the stream" );                            
                        return false;  
                    }
                }
                else
                {
                    T DefaultObject{};

                    for( uint32_t i = 0; i < Remaining; ++i )
                    {
                        if( false == DefaultObject.Serialize( InStream, false ) )
                        {
                            SKLL_TRACE_MSG_FMT( "Failed to serialize padding item %u", i );                            
                            return false;
                        }
                    }
                }

                return true;
            }
            void Clear() noexcept
            {
                Data.clear( );
                TotalBlockCount = 0;
                TotalUsedBlockCount = 0;
            }
            
            SKL_FORCEINLINE T& operator[]( size_t InIndex ) noexcept 
            {
                SKL_ASSERT( InIndex < Data.size() );
                SKL_ASSERT( InIndex < TotalUsedBlockCount );

                return Data[ InIndex ];
            }
            SKL_FORCEINLINE const T& operator[]( size_t InIndex ) const noexcept 
            {
                SKL_ASSERT( InIndex < Data.size() );
                SKL_ASSERT( InIndex < TotalUsedBlockCount );

                return Data[ InIndex ];
            }
            SKL_FORCEINLINE T& Last() noexcept 
            { 
                SKL_ASSERT( false == Data.empty() );
                return Data[ Data.size() - 1 ];
            }
            SKL_FORCEINLINE const T& Last() const noexcept 
            { 
                SKL_ASSERT( false == Data.empty() );
                return Data[ Data.size() - 1 ];
            }
            SKL_FORCEINLINE uint32_t Size() const noexcept
            {
                SKL_ASSERT( Data.size() == TotalUsedBlockCount );
                SKL_ASSERT( TotalUsedBlockCount <= TotalBlockCount );
                return TotalUsedBlockCount;
            }
            SKL_FORCEINLINE bool AddItem( T&& InItem ) noexcept
            {
                if( TotalUsedBlockCount + 1 > TotalBlockCount )
                {
                    return false;
                }

                SKL_ASSERT( Data.size() == TotalUsedBlockCount );
                Data.push_back( std::forward<T>( InItem ) );
                ++TotalUsedBlockCount;

                return true;
            }
            SKL_FORCEINLINE bool CanFit( uint32_t InCount ) const noexcept { return ( TotalBlockCount - TotalUsedBlockCount ) >= InCount; }
            SKL_FORCEINLINE void SetMaxSize( uint32_t InCount ) noexcept 
            { 
                SKL_ASSERT( true == Data.empty() );
                SKL_ASSERT( 0 == TotalUsedBlockCount );
                TotalBlockCount = InCount;
            }
            SKL_FORCEINLINE void Reserve( uint32_t InCount ) noexcept { Data.reserve( InCount ); }
        private:
            uint32_t       TotalBlockCount = 0;
            uint32_t       TotalUsedBlockCount = 0;
            std::vector<T> Data;

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        struct StringBlock
        {
            StringBlock() noexcept = default;

            // cant copy
            StringBlock( const StringBlock& ) = delete;
            StringBlock& operator=( const StringBlock& ) = delete;

            // can move
            StringBlock( StringBlock&& ) noexcept = default;
            StringBlock& operator=( StringBlock&& ) noexcept = default;

            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    
                    BlockTotalSize = Reader->ReadT<decltype( BlockTotalSize )>();
                    BlockUsedSize  = Reader->ReadT<decltype( BlockUsedSize )>();

                    Block.reset( new wchar_t[ BlockTotalSize ] );
                    if( nullptr == Block )
                    {
                        SKLL_TRACE_MSG_FMT( "Failed to Allocate block: wchar_t-size %u", static_cast<uint32_t>( BlockTotalSize ) );
                        return false;
                    }

                    memset( Block.get(), 0, sizeof( wchar_t ) * BlockTotalSize );

                    if( false == Reader->Read( 
                          reinterpret_cast<uint8_t*>( Block.get() )
                        , static_cast<uint32_t>( sizeof( wchar_t ) * BlockTotalSize )
                        , false
                    ) )
                    {
                        SKLL_TRACE_MSG_FMT( "Failed to Read() String block: wchar_t-size %u", static_cast<uint32_t>( BlockTotalSize ) );
                        return false;
                    }
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    
                    Writer->WriteT( BlockTotalSize );
                    Writer->WriteT( BlockUsedSize );

                    SKL_ASSERT( nullptr != Block.get() );
                    
                    if( false == Writer->Write( 
                          reinterpret_cast<const uint8_t*>( Block.get() )
                        , static_cast<uint32_t>( sizeof( wchar_t ) * BlockTotalSize )
                        , false
                    ) )
                    {
                        SKLL_TRACE_MSG_FMT( "Failed to Write() String block: wchar_t-size %u", static_cast<uint32_t>( BlockTotalSize ) );
                        return false;
                    }
                }

                return true;
            }

            bool AllocateBlock( size_t BlockSize = CStringsBlockSize ) noexcept
            {
                if( BlockSize > std::numeric_limits<TBlockIndex>::max() )
                {
                    SKLL_TRACE_MSG_FMT( "Cannot allocate block bigger than max:%llu! wanted:%llu", static_cast<size_t>( std::numeric_limits<TBlockIndex>::max() ), BlockSize );
                    return false;
                }

                Block.reset( new wchar_t[ BlockSize ] );
                if( nullptr == Block )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to allocate block wchar_t-size:%llu", BlockSize );
                    return false;
                }

                memset( Block.get(), 0, sizeof( wchar_t ) * BlockSize );

                BlockTotalSize = static_cast<TBlockIndex>( BlockSize );
                BlockUsedSize  = 0;

                return true;
            }

            const wchar_t* TryAddString( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TBlockIndex& OutIndex ) noexcept
            {
                const auto Remaing{ GetUnusedSize() };

                if( Remaing < InStringSizeInWideCharsNoNullTerm + 1 )
                {
                    return nullptr;
                }

                wchar_t* ToReturn{ Block.get() + BlockUsedSize };
                OutIndex = BlockUsedSize;

                if( 0 < InStringSizeInWideCharsNoNullTerm )
                {
                    SKL_MEMCPY( 
                          ToReturn
                        , sizeof( wchar_t ) * Remaing 
                        , InString
                        , sizeof( wchar_t ) * InStringSizeInWideCharsNoNullTerm
                    );

                    BlockUsedSize += static_cast<TBlockIndex>( InStringSizeInWideCharsNoNullTerm );
                }

                Block.get()[ BlockUsedSize ] = L'\0';
                BlockUsedSize++;

                return ToReturn;
            }

            uint32_t GetUnusedSize() const noexcept { return BlockTotalSize - BlockUsedSize; }

            SKL_FORCEINLINE const wchar_t* GetString( TBlockIndex InIndex ) const noexcept
            { 
                SKL_ASSERT( InIndex < BlockTotalSize );
                return &Block.get()[ InIndex ]; 
            }
            
            void Clear( ) noexcept
            {
                Block.reset( );
                BlockTotalSize = 0;
                BlockUsedSize = 0;
            }
        private:
            TBlockIndex                BlockTotalSize{ 0 };       //!< Block total size in wchar_t(s)
            TBlockIndex                BlockUsedSize { 0 };          //!< Block total used size in wchar_t(s)
            std::unique_ptr<wchar_t[]> Block         { nullptr }; //!< String block

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        struct StringEntry
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if( true == bIsLoadingOrSaving )
                {
                    auto* Reader{ IStreamReader<true>::FromStreamBase( InStream ) };
                    Indices = Reader->ReadT<decltype( Indices )>();
                }
                else
                {
                    auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
                    Writer->WriteT( Indices );
                }

                return true;
            }
            SKL_FORCEINLINE TStringRef GetString() const noexcept { return CachedStringRef; }

        private:
            TBlockIndices Indices{ static_cast<TBlockIndex>( 0 ) ,static_cast<TBlockIndex>( 0 ) };

            TStringRef CachedStringRef{ nullptr };

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        struct StringMapEditData
        {
            void Clear() noexcept
            {
                PresentStringsByIndex.clear();
                PresentStringsByIndices.clear();
            }
             
            std::unordered_map<std::wstring, TStringIndex>   PresentStringsByIndex;
            std::unordered_map<std::wstring, TStringIndices> PresentStringsByIndices;
        };
        struct StringMap: std::conditional_t<bEnableBuildCapabilities, StringMapEditData, EmptyType>
        {
            bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) noexcept
            {
                if( false == StringBlocks.Serialize( InStream, bIsLoadingOrSaving ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to serialize StringBlocks" );
                    return false;
                }
                
                if( false == AllStrings.Serialize( InStream, bIsLoadingOrSaving ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to serialize AllStrings" );
                    return false;
                }

                return true;
            }

            SKL_FORCEINLINE const wchar_t* GetString( TBlockIndex InBlockIndex, TBlockIndex InStringIndex ) const noexcept
            {
                SKL_ASSERT( InBlockIndex < StringBlocks.Count );
                const auto& Block = StringBlocks[ InBlockIndex ];
                return Block.GetString( InStringIndex );
            }
            SKL_FORCEINLINE const wchar_t* GetString( TBlockIndices InIndices ) const noexcept
            {
                SKL_ASSERT( InIndices.first < StringBlocks.Count );
                const auto& Block = StringBlocks[ InIndices.first ];
                return Block.GetString( InIndices.second );
            }
            
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, TStringIndex>>
            TRet SearchIndex( const wchar_t* InString ) noexcept
            {
                for( uint32_t i = 0; i < AllStrings.Size(); ++i )
                {
                    if( 0 == wcscmp( InString, AllStrings[i].CachedStringRef ) )
                    {
                        return static_cast<TStringIndex>( i );
                    }
                }

                return CInvalidStringIndex;
            }
            
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, TStringIndex>>
            TRet QueryIndex( const wchar_t* InString ) noexcept
            {
                const auto Item{ this->PresentStringsByIndex.find( InString ) };
                if( this->PresentStringsByIndex.end() != Item )
                {
                    return Item->second;
                }

                return CInvalidStringIndex;
            }
            
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, TStringIndices>>
            TRet QueryIndices( const wchar_t* InString ) noexcept
            {
                const auto Item{ this->PresentStringsByIndices.find( InString ) };
                if( this->PresentStringsByIndices.end() != Item )
                {
                    return Item->second;
                }

                return { CInvalidBlockIndex, CInvalidBlockIndex };
            }

            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, bool>>
            TRet InsertString( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TStringIndex& OutIndex ) noexcept
            {
                auto ExistingItem{ this->PresentStringsByIndex.find( InString ) };
                if( ExistingItem != this->PresentStringsByIndex.end() )
                {
                    OutIndex = ExistingItem->second;
                    return true;
                }

                TBlockIndices OutIndices;
                if( false == InsertStringInBlock( InString, InStringSizeInWideCharsNoNullTerm, OutIndices ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to insert new string into blocks!" );
                    return false;
                }

                StringEntry NewStringEntry{};
                NewStringEntry.Indices         = OutIndices;
                NewStringEntry.CachedStringRef = GetString( OutIndices );
                SKL_ASSERT( nullptr != NewStringEntry.CachedStringRef );
                
                // cache string by indices
                this->PresentStringsByIndices.insert( { std::wstring{ NewStringEntry.CachedStringRef }, OutIndices } );
                
                // cache string by index
                this->PresentStringsByIndex.insert( { std::wstring{ NewStringEntry.CachedStringRef }, static_cast<TStringIndex>( AllStrings.Size() ) } );
                
                // set index where the new string entry will be added
                OutIndex = static_cast<TStringIndex>( AllStrings.Size() );

                // insert into all strings array
                AllStrings.AddItem( std::move( NewStringEntry ) );

                return true;
            }

            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, bool>>
            TRet InsertString( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TBlockIndices& OutIndices ) noexcept
            {
                auto ExistingItem{ this->PresentStringsByIndices.find( InString ) };
                if( ExistingItem != this->PresentStringsByIndices.end() )
                {
                    OutIndices = ExistingItem->second;
                    return true;
                }

                if( false == InsertStringInBlock( InString, InStringSizeInWideCharsNoNullTerm, OutIndices ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to insert new string into blocks!" );
                    return false;
                }

                StringEntry NewStringEntry{};
                NewStringEntry.Indices         = OutIndices;
                NewStringEntry.CachedStringRef = GetString( OutIndices );
                SKL_ASSERT( nullptr != NewStringEntry.CachedStringRef );
                
                // cache string by indices
                this->PresentStringsByIndices.insert( { std::wstring{ NewStringEntry.CachedStringRef }, OutIndices } );
                
                // cache string by index
                this->PresentStringsByIndex.insert( { std::wstring{ NewStringEntry.CachedStringRef }, static_cast<TStringIndex>( AllStrings.Size() ) } );

                // insert into all strings array
                AllStrings.AddItem( std::move( NewStringEntry ) );

                return true;
            }

            void Clear() noexcept
            {
                StringBlocks.Clear();
                AllStrings.Clear();

                if constexpr( true == bEnableBuildCapabilities )
                {
                    StringMapEditData::Clear();
                }
            }

            bool RefreshCaches() noexcept
            {
                for( uint32_t i = 0; i < AllStrings.Size(); ++i )
                {
                    auto& Entry = AllStrings[i];

                    if( CInvalidBlockIndex == Entry.Indices.first || CInvalidBlockIndex == Entry.Indices.second )
                    {
                        Entry.CachedStringRef = nullptr;
                    }
                    else
                    {
                        Entry.CachedStringRef = GetString( Entry.Indices );
                    }
                }

                return true;
            }

            SKL_FORCEINLINE const wchar_t* GetStringByIndex( TStringIndex InIndex ) const noexcept
            {
                SKL_ASSERT( CInvalidStringIndex != InIndex );
                SKL_ASSERT( AllStrings.Size() > InIndex );
                return AllStrings[InIndex].CachedStringRef;
            }

        private:
            template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, bool>>
            TRet InsertStringInBlock( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TBlockIndices& OutIndices ) noexcept
            {
                for( uint32_t i = 0; i < StringBlocks.Size(); ++i )
                {
                    if( nullptr != StringBlocks[i].TryAddString( InString, InStringSizeInWideCharsNoNullTerm, OutIndices.second ) )
                    {
                        OutIndices.first = static_cast<TBlockIndex>( i );
                        return true;
                    }
                }

                StringBlock NewBlock{};
                if( false == NewBlock.AllocateBlock() )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to allocate new block!" );
                    return false;
                }

                if( nullptr == NewBlock.TryAddString( InString, InStringSizeInWideCharsNoNullTerm, OutIndices.second ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to add new string to the new block!" );
                    return false;
                }

                OutIndices.first = static_cast<TBlockIndex>( StringBlocks.Size() );
                StringBlocks.AddItem( std::move( NewBlock ) );

                return true;
            }

            Array<StringBlock> StringBlocks;
            Array<StringEntry> AllStrings;

            friend ::DatacenterTests::DatacenterTestsFixture;
            friend MyType;
        };

        using ElementsBlock   = BlockArray<Element, static_cast<uint32_t>( GetElementSerialSize() )>;
        using AttributesBlock = BlockArray<Attribute, static_cast<uint32_t>( GetAttributeSerialSize() )>;
        
        using DatacenterElements   = Array<ElementsBlock>;
        using DatacenterAttributes = Array<AttributesBlock>;

        Datacenter() noexcept = default;

        SKL_FORCEINLINE void SetStream( StreamBase* InStream )
        {
            SKL_ASSERT( false == IsLoaded() );
            SourceStream = InStream;
        }

        bool Serialize( bool bIsLoadingOrSaving, bool bDoPostLoadProcessing = true ) noexcept
        {
            if constexpr( false == bEnableBuildCapabilities )
            {
                if( false == bIsLoadingOrSaving ) 
                {
                    SKLL_TRACE_MSG_FMT( "Cannot performa save without bEnableBuildCapabilities==true!" );
                    return false;
                }
            }

            if( nullptr == SourceStream )
            {
                SKLL_TRACE_MSG_FMT( "No source stream provided!" );
                return false;
            }

            auto& TargetStream{ *SourceStream };

            if ( true == bIsLoadingOrSaving )
            {
                auto* Reader{ IStreamReader<true>::FromStreamBase( TargetStream ) };
                
                Version       = Reader->ReadT<decltype( Version )>();
                FormatVersion = Reader->ReadT<decltype( FormatVersion )>();
                Language      = Reader->ReadT<decltype( Language )>();
            }
            else
            {
                auto* Writer{ IStreamWriter<true>::FromStreamBase( TargetStream ) };

                Writer->WriteT( Version );
                Writer->WriteT( FormatVersion );
                Writer->WriteT( Language );
            }

            if( false == Attributes.Serialize( TargetStream, bIsLoadingOrSaving ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to de/serialize Attributes!" );
                return false;
            }

            if( false == Elements.Serialize( TargetStream, bIsLoadingOrSaving ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to de/serialize Elements!" );
                return false;
            }
            
            if( false == ValuesMap.Serialize( TargetStream, bIsLoadingOrSaving ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to de/serialize Values!" );
                return false;
            }
            
            if( false == NamesMap.Serialize( TargetStream, bIsLoadingOrSaving ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to de/serialize Names!" );
                return false;
            }

            if( true == bIsLoadingOrSaving && true == bDoPostLoadProcessing )
            {
                if( false == PostLoadProcessing() )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to do the post load processing!" );
                    return false;
                }
            }

            bIsLoaded = true;

            return true;
        }

        bool PostLoadProcessing() noexcept
        {
            if( false == ValuesMap.RefreshCaches() )
            {
                SKLL_TRACE_MSG_FMT( "Failed to refresh ValuesMap caches!" );
                return false;
            }

            if( false == NamesMap.RefreshCaches() )
            {
                SKLL_TRACE_MSG_FMT( "Failed to refresh NamesMap caches!" );
                return false;
            }

            if( false == PostLoadProcessing_Attributes() )
            {
                SKLL_TRACE_MSG_FMT( "Failed to PostLoadProcessing_Attributes()!" );
                return false;
            }

            if( false == PostLoadProcessing_Elements() )
            {
                SKLL_TRACE_MSG_FMT( "Failed to PostLoadProcessing_Elements()!" );
                return false;
            }

            return true;
        }

        void Clear() noexcept
        {
            bIsLoaded      = false;
            Version       = 0;
            FormatVersion = 0;

            Attributes.Clear();
            Elements.Clear();
            ValuesMap.Clear();
            NamesMap.Clear();

            SourceStream = nullptr;
        }

        bool IsLoaded() const noexcept{ return bIsLoaded; }
        
        bool SaveToFile( const char* InFileName ) const noexcept
        {
            if( false == IsLoaded() )
            {
                return false;
            }

            SKL_ASSERT( nullptr != SourceStream );
            auto* Reader{ IStreamReader<true>::FromStreamBase( *SourceStream ) };
            return Reader->SaveToFile( InFileName, false, true, false );
        }

        TVersion GetVersion() const noexcept{ return Version; }
        TFormatVersion GetFormatVersion() const noexcept{ return FormatVersion; }
        TLanguage GetLanguage() const noexcept{ return Language; }
        
        template<typename TParam = std::enable_if_t<bEnableBuildCapabilities, TVersion>> 
        void SetVersion( TParam InVersion ) noexcept{ Version = InVersion; }
        template<typename TParam = std::enable_if_t<bEnableBuildCapabilities, TFormatVersion>> 
        void SetFormatVersion( TParam InFormatVersion ) noexcept{ FormatVersion = InFormatVersion; }
        template<typename TParam = std::enable_if_t<bEnableBuildCapabilities, TLanguage>> 
        void SetLanguage( TParam InLanguage ) noexcept{ Language = InLanguage; }

        Element* GetRootElement() noexcept
        {
            if( 0 == Elements.Size() || 0 == Elements[0].Size() )
            {
                return nullptr;
            }

            return &Elements[0][0];
        }
        const Element* GetRootElement() const noexcept
        {
            if( 0 == Elements.Size() || 0 == Elements[0].Size() )
            {
                return nullptr;
            }

            return &Elements[0][0];
        }
        Element* GetElement( TBlockIndices InIndices ) noexcept 
        {
            SKL_ASSERT( CInvalidBlockIndex != InIndices.first && CInvalidBlockIndex != InIndices.second );
            return &Elements[InIndices.first][InIndices.second];
        }
        Attribute* GetAttribute( TBlockIndices InIndices ) noexcept 
        {
            SKL_ASSERT( CInvalidBlockIndex != InIndices.first && CInvalidBlockIndex != InIndices.second );
            return &Attributes[InIndices.first][InIndices.second];
        }

        template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, StringMap>> 
        TRet& GetValuesMap() noexcept { return ValuesMap; }
        template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, StringMap>> 
        TRet& GetNamesMap() noexcept { return NamesMap; }
        template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, DatacenterElements>> 
        TRet& GetElementsBlock() noexcept { return Elements; }
        template<typename TRet = std::enable_if_t<bEnableBuildCapabilities, DatacenterAttributes>> 
        TRet& GetAttributesBlock() noexcept { return Attributes; }
        
        std::vector<const Element*> GetAllByNameStartsWith( const wchar_t* InStartsWithString ) const noexcept
        {
            SKL_ASSERT( nullptr != InStartsWithString );

            const size_t InStartsWithStringLength{ wcslen( InStartsWithString ) };
            SKL_ASSERT( 0 < InStartsWithStringLength );
            
            std::vector<const Element*> Result;
            Result.reserve( 128 );

            const auto* Root{ GetRootElement() };
            for( const auto* Item : Root->GetChildren() )
            {
                if( true == CWStringStartsWith( InStartsWithString, InStartsWithStringLength, Item->GetName() ) )
                {
                    Result.push_back( Item );
                }
            }

            return Result;
        }
        std::vector<const Element*> GetAllByName( const wchar_t* InName ) const noexcept
        {
            SKL_ASSERT( nullptr != InName );
            SKL_ASSERT( 0 < wcslen( InName ) );

             std::vector<const Element*> Result;
             Result.reserve( 128 );

            const auto* Root{ GetRootElement() };
            const auto NameLength{ SKL_WSTRLEN( InName, 1024 ) };
            for( const auto* Item : Root->GetChildren() )
            {
                if( 0 == SKL_WSTRICMP( InName, Item->GetName(), NameLength ) )
                {
                    Result.push_back( Item );
                }
            }

            return Result;
        }

    protected:
        bool PostLoadProcessing_Attributes() noexcept
        {
            for( uint32_t i = 0; i < Attributes.Size(); ++i )
            {
                for( uint32_t j = 0; j < Attributes[i].Size(); ++j )
                {
                    Attribute& AttributeItem = Attributes[i][j];

                    AttributeItem.CachedValueRef = ValuesMap.GetString( AttributeItem.Value );
                    SKL_ASSERT( nullptr != AttributeItem.CachedValueRef );

                    if( CInvalidStringIndex == AttributeItem.NameIndex )
                    {    
                        SKLL_TRACE_MSG_FMT( "Loaded Attribute with inavlid NameId!" );
                        return false;
                    }

                    AttributeItem.CachedNameRef = NamesMap.GetStringByIndex( AttributeItem.NameIndex );
                    SKL_ASSERT( nullptr != AttributeItem.CachedNameRef );
                }
            }

            return true;
        }
        bool PostLoadProcessing_Elements() noexcept
        {
            for( uint32_t i = 0; i < Elements.Size(); ++i )
            {
                for( uint32_t j = 0; j < Elements[i].Size(); ++j )
                {
                    Element& ElementItem = Elements[i][j];

                    if( CInvalidStringIndex == ElementItem.NameIndex )
                    {
                        SKLL_TRACE_MSG_FMT( "Loaded element with invalid name Index!" );
                        return false;
                    }

                    ElementItem.CachedNameRef = NamesMap.GetStringByIndex( ElementItem.NameIndex );
                    SKL_ASSERT( nullptr != ElementItem.CachedNameRef );

                    if( CInvalidBlockIndex != ElementItem.ValueIndices.first && CInvalidBlockIndex != ElementItem.ValueIndices.second )
                    {
                        ElementItem.CachedValueRef = NamesMap.GetString( ElementItem.ValueIndices );
                        SKL_ASSERT( nullptr != ElementItem.CachedValueRef );
                    }

                    if constexpr( true == bEnableBuildCapabilities )
                    {
                        auto& EditData{ ElementItem.GetEditData() };
                        EditData.CachedLocation = {
                              static_cast<TBlockIndex> ( i )
                            , static_cast<TBlockIndex> ( j )
                        };
                    }
                }
            }

            if( false == PostLoadProcessing_Elements_Recursive( GetRootElement(), nullptr ) )
            {
                return false;
            }

            return true;
        }
        bool PostLoadProcessing_Elements_Recursive( Element* InElement, Element* InParentElement  ) noexcept
        {
            InElement->Parent = InParentElement;
            if ( nullptr != InParentElement )
            {
                InElement->Parent->CachedChildren.push_back( InElement );
            }    

            SKL_ASSERT( CInvalidStringIndex != InElement->NameIndex && nullptr != InElement->CachedNameRef );
            if constexpr( true == bEnableBuildCapabilities )
            {
                InElement->GetEditData().ParentLocation = InParentElement->GetEditData().CachedLocation;
            }

            // cache all attributes
            for( uint16_t i = 0; i < InElement->AttributesCount; ++i )
            {
                auto* AttributeItem{ GetAttribute( { InElement->AttributeIndices.first, InElement->AttributeIndices.second + i } ) };
                if( nullptr == AttributeItem )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to get Attribute {%hu %hu} for element{%ws}"
                        , InElement->AttributeIndices.first
                        , InElement->AttributeIndices.second
                        , InElement->GetName() );
                    return false;
                }

                InElement->CachedAttributes.push_back( AttributeItem );
            }

            for( uint16_t i = 0; i < InElement->ChildrenCount; ++i )
            {
                auto* Child{ GetElement( {
                    InElement->ChildrenIndices.first,
                    InElement->ChildrenIndices.second + i
                } ) };

                if( nullptr == Child )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to get Child {%hu %hu} for element{%ws}"
                        , InElement->ChildrenIndices.first
                        , InElement->ChildrenIndices.second + i
                        , InElement->GetName() );
                    return false;
                }

                if constexpr( true == bEnableBuildCapabilities )
                {
                    InElement->GetEditData().CachedLocation = {
                          InElement->ChildrenIndices.first
                        , InElement->ChildrenIndices.second + i
                    };
                }

                if( false == PostLoadProcessing_Elements_Recursive( Child, InElement ) )
                {
                    return false;
                }
            }

            return true;
        }

        bool                           bIsLoaded    { false };
        TVersion                       Version      { 0 };
        TFormatVersion                 FormatVersion{ 0 };
        TLanguage                      Language     { CNoSpecificLanguage };
        DatacenterAttributes           Attributes   {};
        DatacenterElements             Elements     {};
        StringMap                      ValuesMap    {};
        StringMap                      NamesMap     {};
        StreamBase*                    SourceStream { nullptr };

        friend ::DatacenterTests::DatacenterTestsFixture;
        friend struct Builder;
    };
}

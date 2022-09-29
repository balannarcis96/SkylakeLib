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

namespace SKL::Datacenter
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

	constexpr size_t CElementsBlockSize        = std::numeric_limits<TBlockIndex>::max();
	constexpr size_t CAttributesBlockSize      = std::numeric_limits<TBlockIndex>::max();
	constexpr size_t CStringsBlockSize         = std::numeric_limits<TBlockIndex>::max();
	constexpr TStringIndex CInvalidStringIndex = std::numeric_limits<TStringIndex>::max();
	constexpr TBlockIndex CInvalidBlockIndex   = std::numeric_limits<TBlockIndex>::max();

	template<bool bEnableBuildCapabilities>
	struct Datacenter
	{
		using MyType = Datacenter<bEnableBuildCapabilities>;
		
		static constexpr size_t GetAttributeSerialSize() noexcept
		{
			return 0;
		}
		static constexpr size_t GetElementSerialSize() noexcept
		{
			return 0;
		}

		struct Attribute
		{
			SKL_FORCEINLINE TStringRef GetName() const noexcept 
			{
				return CachedNameRef;
			}
			SKL_FORCEINLINE TStringRef GetValue() const noexcept
			{
				return CachedValueRef;
			}

		private:
			TNameIndex     NameIndex{ CInvalidBlockIndex };
			TBlockIndex    Value    { CInvalidBlockIndex, CInvalidBlockIndex };

			TStringRef     CachedNameRef { nullptr };
			TStringRef     CachedValueRef{ nullptr };
		};

		struct Element
		{
			struct MyEditData
			{
				TBlockIndices CachedLocation{ CInvalidBlockIndex,CInvalidBlockIndex  };
				TBlockIndices ParentLocation{ CInvalidBlockIndex,CInvalidBlockIndex  };
				uint32_t	  RefCount		{ 1 };

				SKL_FORCEINLINE void AddRef() noexcept { ++RefCount; }
				SKL_FORCEINLINE bool RemoveRef() noexcept 
				{ 
					SKL_ASSERT( 0 < RefCount );
					0 == --RefCount; 
				}
			};
			using TMyEditData = std::conditional_t<bEnableBuildCapabilities, MyEditData, void>;

			SKL_FORCEINLINE TStringRef GetName() const noexcept 
			{
				return CachedNameRef;
			}
			SKL_FORCEINLINE const std::vector<std::shared_ptr<Attribute>>& GetAttributes() const noexcept
			{
				return CachedAttributes;
			}
			SKL_FORCEINLINE const std::vector<std::shared_ptr<Element>>& GetChildren() const noexcept
			{
				return CachedChildren;
			}
			SKL_FORCEINLINE Element* GetParent() const noexcept{ return Parent; }

			SKL_FORCEINLINE TMyEditData& GetEditData() noexcept { return EditData; };
			SKL_FORCEINLINE const TMyEditData& GetEditData() const noexcept { return EditData; };

		private:
			TNameIndex     NameIndex         { CInvalidStringIndex };
			uint16_t       AttributesCount   { 0 };
			TBlockIndices  AttributeIndices  { CInvalidBlockIndex, CInvalidBlockIndex };
			uint16_t       ChildrenCount     { 0 };
			TBlockIndices  ChildrenIndices   { CInvalidBlockIndex, CInvalidBlockIndex };

			TStringRef				CachedNameRef   { nullptr };
			std::vector<Attribute*> CachedAttributes{};
			std::vector<Element*>   CachedChildren  {};
			Element*				Parent			{ nullptr };
			TMyEditData				EditData;
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

						if( false == Object.Serialize( InStream, bIsLoadingOrSaving ) )
						{
							SKLL_TRACE_MSG_FMT( "Failed to deserialize item %u", i );							
							return false;
						}

						Data.emplace_back( std::move( Object ) );
					}
				}
				else
				{
					auto* Writer{ IStreamWriter<true>::FromStreamBase( InStream ) };
					
					Writer->WriteT( Count );

					SKL_ASSERT( Data.size() == Count );

					for( uint32_t i = 0; i < Count; ++i )
					{
						if( false == Data[i].Serialize( InStream, bIsLoadingOrSaving ) )
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
			SKL_FORCEINLINE uint32_t Size() const noexcept
			{
				SKL_ASSERT( Data.size() == Count );
				return Count;
			}
			SKL_FORCEINLINE void AddItem( T&& InItem ) noexcept
			{
				SKL_ASSERT( Data.size() == Count );
				Data.emplace_back( std::forward<T>( InItem ) );
				++Count;
			}
		private:
			uint32_t       Count{ 0 };
			std::vector<T> Data;
		};
	
		template<typename T, uint32_t ElementSize = static_cast<uint32_t>( sizeof( T ) )>
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

						if( false == Object.Serialize( InStream, bIsLoadingOrSaving ) )
						{
							SKLL_TRACE_MSG_FMT( "Failed to deserialize item %u", i );							
							return false;
						}

						Data.emplace_back( std::move( Object ) );
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
						if( false == Data[i].Serialize( InStream, bIsLoadingOrSaving ) )
						{
							SKLL_TRACE_MSG_FMT( "Failed to serialize item %u", i );							
							return false;
						}
					}
				}

				uint32_t Remaining{ TotalBlockCount - TotalUsedBlockCount };

				if( true == bIsLoadingOrSaving )
				{
					InStream.Position += Remaining * ElementSize;
				}
				else
				{
					T DefaultObject{};

					for( uint32_t i = 0; i < Remaining; ++i )
					{
						if( false == DefaultObject.Serialize( InStream, bIsLoadingOrSaving ) )
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
			SKL_FORCEINLINE uint32_t Size() const noexcept
			{
				SKL_ASSERT( Data.size() == TotalBlockCount );
				SKL_ASSERT( TotalUsedBlockCount <= TotalBlockCount );
				return TotalUsedBlockCount;
			}
			SKL_FORCEINLINE bool AddItem( T&& InItem ) noexcept
			{
				if( TotalBlockCount + 1 > TotalBlockCount )
				{
					return false;
				}

				SKL_ASSERT( Data.size() == TotalBlockCount );
				Data.emplace_back( std::forward<T>( InItem ) );
				++TotalBlockCount;
			}
		private:
			uint32_t	   TotalBlockCount = 0;
			uint32_t	   TotalUsedBlockCount = 0;
			std::vector<T> Data;
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
				Block.reset( new wchar_t[ BlockSize ] );
				if( nullptr == Block )
				{
					SKLL_TRACE_MSG_FMT( "Failed to allocate block wchar_t-size:%llu", BlockSize );
					return false;
				}

				memset( Block.get(), 0, sizeof( wchar_t ) * BlockSize );

				BlockTotalSize = static_cast<uint32_t>( BlockSize );
				BlockUsedSize  = 0;
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

					BlockUsedSize += InStringSizeInWideCharsNoNullTerm;
				}

				Block.get()[ BlockUsedSize ] = L'\0';
				BlockUsedSize++;

				return ToReturn;
			}

			uint32_t GetUnusedSize() const noexcept { return BlockTotalSize - BlockUsedSize; }

			SKL_FORCEINLINE const wchar_t GetString( TBlockIndex InIndex ) const noexcept
			{ 
				SKL_ASSERT( InIndex < BlockTotalSize );
				return Block.get()[ InIndex ]; 
			}
			
			void Clear( ) noexcept
			{
				Block.reset( );
				BlockTotalSize = 0;
				BlockUsedSize = 0;
			}
		private:
			TBlockIndex                BlockTotalSize{ 0 };       //!< Block total size in wchar_t(s)
			TBlockIndex                BlockUsedSize { 0 };		  //!< Block total used size in wchar_t(s)
			std::unique_ptr<wchar_t[]> Block         { nullptr }; //!< String block

			friend MyType;
		};

		struct StringEntry
		{
			bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) const noexcept
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
			}
			SKL_FORCEINLINE TStringRef GetString() const noexcept { return CachedStringRef; }

		private:
			TBlockIndices Indices{ 0 ,0 };

			TStringRef CachedStringRef{ nullptr };

			friend MyType;
		};

		struct StringMap
		{
			struct MapEditData
			{
				void Clear() noexcept
				{
					PresentStringsByIndex.clear();
					PresentStringsByIndices.clear();
				}
				 
				std::unordered_map<std::wstring, TStringIndex>   PresentStringsByIndex;
				std::unordered_map<std::wstring, TStringIndices> PresentStringsByIndices;
			};
			using TMapEditData = std::conditional_t<bEnableBuildCapabilities, MapEditData, void>;

			bool Serialize( Stream& InStream, bool bIsLoadingOrSaving ) const noexcept
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
			
			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			TStringIndex SearchIndex( const wchar_t* InString ) noexcept
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
			
			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			TStringIndex QueryIndex( const wchar_t* InString ) noexcept
			{
				const auto Item{ EditData.PresentStringsByIndex.find( InString ) };
				if( EditData.PresentStringsByIndex.end() != Item )
				{
					return Item->second;
				}

				return CInvalidStringIndex;
			}
			
			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			TStringIndices QueryIndices( const wchar_t* InString ) noexcept
			{
				const auto Item{ EditData.PresentStringsByIndices.find( InString ) };
				if( EditData.PresentStringsByIndices.end() != Item )
				{
					return Item->second;
				}

				return { CInvalidBlockIndex, CInvalidBlockIndex };
			}

			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			bool InsertString( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TStringIndex& OutIndex ) noexcept
			{
				auto ExistingItem{ EditData.PresentStringsByIndex.find( InString ) };
				if( ExistingItem != EditData.PresentStringsByIndex.end() )
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
				EditData.PresentStringsByIndices.insert( { NewStringEntry.CachedStringRef, OutIndices } );
				
				// cache string by index
				EditData.PresentStringsByIndices.insert( { NewStringEntry.CachedStringRef, AllStrings.Size() } );
				
				// set index where the new string entry will be added
				OutIndex = static_cast<TStringIndex>( AllStrings.Size() );

				// insert into all strings array
				AllStrings.AddItem( std::move( NewStringEntry ) );

				return true;
			}

			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			bool InsertString( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TBlockIndices& OutIndices ) noexcept
			{
				auto ExistingItem{ EditData.PresentStringsByIndices.find( InString ) };
				if( ExistingItem != EditData.PresentStringsByIndices.end() )
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
				EditData.PresentStringsByIndices.insert( { NewStringEntry.CachedStringRef, OutIndices } );
				
				// cache string by index
				EditData.PresentStringsByIndices.insert( { NewStringEntry.CachedStringRef, AllStrings.Count() } );

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
					EditData.Clear();
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
			template<typename = std::enable_if_t<bEnableBuildCapabilities>>
			bool InsertStringInBlock( const wchar_t* InString, uint32_t InStringSizeInWideCharsNoNullTerm, TBlockIndices& OutIndices ) noexcept
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
				StringBlocks.AddItem( std::move( NewBlock ) );
				OutIndices.first = static_cast<TBlockIndex>( StringBlocks.Size() - 1 );

				if( false == NewBlock.TryAddString( InString, InStringSizeInWideCharsNoNullTerm, OutIndices.second ) )
				{
					SKLL_TRACE_MSG_FMT( "Failed to add new string to the new block!" );
					return false;
				}

				return true;
			}

			Array<StringBlock> StringBlocks;
			Array<StringEntry> AllStrings;
			TMapEditData	   EditData;

			friend MyType;
		};

		using ElementsBlock   = Array<BlockArray<Element>>;
		using AttributesBlock = Array<BlockArray<Attribute>>;

		void SetStream( BufferStream&& InStream )
		{
			SKL_ASSERT( false == IsLoaded() );
			SourceStream = std::forward<BufferStream>( InStream );
		}

		bool Serialize( bool bIsLoadingOrSaving, bool bDoPostLoadProcessing = true ) noexcept
		{
			if constexpr( false == bEnableBuildCapabilities )
			{
				if( true == bIsLoadingOrSaving ) 
				{
					SKLL_TRACE_MSG_FMT( "Cannot performa save without bEnableBuildCapabilities==true!" );
					return false;
				}
			}

			auto& TargetStream{ SourceStream.GetStreamBase() };

			if ( true == bIsLoadingOrSaving )
			{
				auto* Reader{ IStreamReader<true>::FromStreamBase( TargetStream ) };
				
				Version       = Reader->ReadT<decltype( Version )>();
				FormatVersion = Reader->ReadT<decltype( FormatVersion )>();
			}
			else
			{
				auto* Writer{ IStreamWriter<true>::FromStreamBase( TargetStream ) };

				Writer->WriteT( Version );
				FormatVersion->WriteT( Version );
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

			if( PostLoadProcessing_Attributes() )
			{
				SKLL_TRACE_MSG_FMT( "Failed to PostLoadProcessing_Attributes()!" );
				return false;
			}

			if( PostLoadProcessing_Elements() )
			{
				SKLL_TRACE_MSG_FMT( "Failed to PostLoadProcessing_Elements()!" );
				return false;
			}

			return true;
		}

		void Clear() noexcept
		{
			bIsLoaded	  = false;
			Version       = 0;
			FormatVersion = 0;

			Attributes.Clear();
			Elements.Clear();
			ValuesMap.Clear();
			NamesMap.Clear();
		}

		bool IsLoaded() const noexcept{ return bIsLoaded; }
		
		Element* GetRootElement() noexcept
		{
			if( 0 == Elements.Size() || 0 == Elements[0].Size() )
			{
				return nullptr;
			}

			return &Elements[0][0];
		}

		const Element* GetElement( TBlockIndices InIndices ) const noexcept 
		{
			SKL_ASSERT( CInvalidBlockIndex != InIndices.first && CInvalidBlockIndex != InIndices.second );
			return &Elements[InIndices.first][InIndices.second];
		}

		const Attribute* GetAttribute( TBlockIndices InIndices ) const noexcept 
		{
			SKL_ASSERT( CInvalidBlockIndex != InIndices.first && CInvalidBlockIndex != InIndices.second );
			return &Attributes[InIndices.first][InIndices.second];
		}

	private:
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
			if ( nullptr != InParentElement )
			{
				if( nullptr != InElement->Parent )
				{
					if constexpr( true == bEnableBuildCapabilities )
					{
						InElement->GetEditData().AddRef();
					}

					InElement->Parent->CachedChildren.push_back( InElement );
					return true;
				}
			}	

			InElement->Parent = InParentElement;
			SKL_ASSERT( CInvalidStringIndex != InElement->NameIndex && nullptr != InElement->CachedNameRef );
			if constexpr( true == bEnableBuildCapabilities )
			{
				InElement->GetEditData().ParentLocation = InParentElement->GetEditData().CachedLocation;
			}

			// cache all attributes
			for( uint16_t i = 0; i < InElement->AttributesCount; ++i )
			{
				const auto* AttributeItem{ GetAttribute( InElement->AttributeIndices ) };
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

		bool            bIsLoaded{ false };

		TVersion        Version{ 0 };
		TFormatVersion  FormatVersion{ 0 };
		AttributesBlock Attributes;
		ElementsBlock   Elements;
		StringMap       ValuesMap;
		StringMap       NamesMap;

		BufferStream	SourceStream;
	};
}
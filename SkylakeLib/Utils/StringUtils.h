//!
//! \file StringUtils.h
//! 
//! \brief Skylake String utils abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct StringUtils final : protected ITLSSingleton<StringUtils>
    {
        StringUtils() noexcept :
            WorkBenchBuffer{ CStringUtilsWorkBenchBufferSize } {}

        //! Initialize
        RStatus Initialize( ) noexcept override 
        { 
            return RSuccess; 
        }

        //! Get the instance name
        const char* GetName() const noexcept override
        {
            return "[StringUtils]";
        }

        //! Convert ipv4 address to string 
        static const char* IpV4AddressToString( TIPv4Address InAddress ) noexcept;

        //! Convert ipv4 address to wide string 
        static const wchar_t* IpV4AddressToWString( TIPv4Address InAddress ) noexcept;

    private:
        BufferStream WorkBenchBuffer;

        friend class ServerInstance;
        friend RStatus Skylake_InitializeLibrary_Thread() noexcept;
        friend RStatus Skylake_TerminateLibrary_Thread() noexcept;
    };

	inline bool IsEmptyString( const std::string& str ) noexcept
	{
		for( auto c : str )
		{
			if( c != ' ' )
			{
				return false;
			}
		}
	
		return true;
	}
	inline bool IsEmptyString( const std::wstring& str ) noexcept
	{
		for( auto c : str )
		{
			if( c != L' ' )
			{
				return false;
			}
		}
	
		return true;
	}
	inline bool IsEmptyString( const std::string_view& str ) noexcept
	{
		for( auto c : str )
		{
			if( c != ' ' )
			{
				return false;
			}
		}
	
		return true;
	}
	inline bool IsEmptyString( const std::wstring_view& str ) noexcept
	{
		for( auto c : str )
		{
			if( c != L' ' )
			{
				return false;
			}
		}
	
		return true;
	}
	
	template<typename Str>
	std::vector<Str> SplitString( const Str& target, const Str& token, bool ommitEmptySpaces, bool keepTokens )noexcept
	{
		static_assert( std::is_same_v<Str, std::wstring> || std::is_same_v<Str, std::string> || std::is_same_v<Str, std::wstring_view> || std::is_same_v<Str, std::string_view>, "Utils::SplitString<Str>() Requires Str to be std::wstring or std::string>" );
	
		std::vector<Str> result;
		result.reserve( 16 );
	
		size_t start = 0;
		auto index = target.find_first_of( token );
		while( index != Str::npos )
		{
			auto found_substr = target.substr( start, index - start );
	
			const bool bIsEmpty = IsEmptyString( found_substr );
	
			if( !ommitEmptySpaces || !bIsEmpty )
			{
				result.push_back( found_substr );
				if( keepTokens )
				{
					result.push_back( token );
				}
			}
			else if( bIsEmpty && keepTokens )
			{
				result.push_back( token );
			}
	
			start = index + 1;
			index = target.find_first_of( token, index + token.size( ) );
		}
	
		if( start < target.size( ) )
		{
			auto found_substr = target.substr( start, target.size( ) - start );
			const bool bIsEmpty = IsEmptyString( found_substr );
			if( !ommitEmptySpaces || !bIsEmpty )
			{
				result.push_back( found_substr );
			}
			else if( bIsEmpty && keepTokens )
			{
				result.push_back( token );
			}
		}
		else if( result.size( ) >= 2 && keepTokens )
		{
			result.erase( result.begin( ) + result.size( ) - 1 );
		}
	
		return result;
	}
	
	// trim from start (in place)
	template<typename Str>
	void LeftTrim( Str& s )
	{
		static_assert( std::is_same_v<Str, std::wstring> || std::is_same_v<Str, std::string> || std::is_same_v<Str, std::wstring_view> || std::is_same_v<Str, std::string_view>, "Utils::LeftTrim<Str>() Requires Str to be std::wstring or std::string>" );
	
		s.erase( s.begin( ), std::find_if( s.begin( ), s.end( ), []( typename Str::value_type ch )
		{
			return !std::isspace( ch );
		} ) );
	}
	
	// trim from end (in place)
	template<typename Str>
	void RightTrim( Str& s )
	{
		static_assert( std::is_same_v<Str, std::wstring> || std::is_same_v<Str, std::string> || std::is_same_v<Str, std::wstring_view> || std::is_same_v<Str, std::string_view>, "Utils::RightTrim<Str>() Requires Str to be std::wstring or std::string>" );
	
		s.erase( std::find_if( s.rbegin( ), s.rend( ), []( typename Str::value_type ch )
		{
			return !std::isspace( ch );
		} ).base( ), s.end( ) );
	}
	
	// trim from both ends (in place)
	template<typename Str>
	void Trim( Str& s ) 
	{
		static_assert( std::is_same_v<Str, std::wstring> || std::is_same_v<Str, std::string> || std::is_same_v<Str, std::wstring_view> || std::is_same_v<Str, std::string_view>, "Utils::Trim<Str>() Requires Str to be std::wstring or std::string>" );
	
		LeftTrim( s );
		RightTrim( s );
	}
}

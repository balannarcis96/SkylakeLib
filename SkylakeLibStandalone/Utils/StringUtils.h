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
	inline bool IsEmptyString( const std::string& InString ) noexcept
	{
		return true == InString.empty() || std::string::npos == InString.find_first_not_of( ' ' );
	}
	inline bool IsEmptyString( const std::wstring& InString ) noexcept
	{
		return true == InString.empty() || std::wstring::npos == InString.find_first_not_of( L' ' );
	}
	inline bool IsEmptyString( const std::string_view& InString ) noexcept
	{
		return true == InString.empty() || std::string::npos == InString.find_first_not_of( ' ' );
	}
	inline bool IsEmptyString( const std::wstring_view& InString ) noexcept
	{
		return true == InString.empty() || std::wstring::npos == InString.find_first_not_of( L' ' );
	}
	inline bool IsEmptyOrWhitespacesOrNull( const char* InString ) noexcept
	{
		if( nullptr == InString )
		{ 
			return true; 
		}

		return IsEmptyString( std::string_view{ InString } );
	}
	inline bool IsEmptyOrWhitespacesOrNull( const wchar_t* InString ) noexcept
	{
		if( nullptr == InString )
		{ 
			return true; 
		}

		return IsEmptyString( std::wstring_view{ InString } );
	}

	template<typename Str>
	std::vector<Str> SplitString( const Str& target, const Str& token, bool ommitEmptySpaces, bool keepTokens ) noexcept
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

	// return true is both strings are equal
	SKL_FORCEINLINE bool StringEqual( const char* InFirstString, const char* InLastString, size_t InMaxmimCharactersToCompare ) noexcept
	{
		return 0 == SKL_STRCMP( InFirstString, InLastString, InMaxmimCharactersToCompare );
	}
	
	// return true is both strings are equal
	SKL_FORCEINLINE bool StringEqual( const wchar_t* InFirstString, const wchar_t* InLastString, size_t InMaxmimCharactersToCompare ) noexcept
	{
		return 0 == SKL_WSTRCMP( InFirstString, InLastString, InMaxmimCharactersToCompare );
	}

	// return true is both strings are equal
	template<typename TChar, size_t InLastStringSize>
	SKL_FORCEINLINE bool StringEqual( const TChar* InFirstString, const TChar(&InLastString)[InLastStringSize] ) noexcept
	{
		static_assert( true == std::is_same_v<TChar, char> || true == std::is_same_v<TChar, wchar_t> );
		return StringEqual( InFirstString, InLastString, InLastStringSize );
	}

	SKL_FORCEINLINE std::wstring GetLastDirectoryW( const wchar_t* InPath ) noexcept
	{
		const std::filesystem::path Temp{ InPath };
		return Temp.parent_path().filename();
	}
}

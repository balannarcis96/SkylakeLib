//!
//! \file SkylakeAssert.h
//! 
//! \brief Debug and release assert capabilities for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

namespace SKL
{
    extern thread_local std::array<char, 4098> GSklAssertWorkBuffer;

    SKL_NOINLINE SKL_NORETURN inline void SklAssertFailHandler( const char* InExpressionAsString, const char* InFileName, size_t LineNumber ) noexcept
    {
        ( void )snprintf( GSklAssertWorkBuffer.data(), GSklAssertWorkBuffer.size(), "\u001b[31mAssert \"%s\" failed!\nAt:%s:%llu \n\u001b[37m", InExpressionAsString, InFileName, LineNumber );
        ( void )printf( GSklAssertWorkBuffer.data() );
        SKL_BREAK();
        abort();
    }
    
    template<typename T>
    SKL_NOINLINE SKL_NORETURN inline void SklAssertEqualityFailHandler( T Value1, T Value2, const char* InExpressionAsString, const char* InFileName, size_t LineNumber ) noexcept
    {
        std::string Value1Str;
        std::string Value2Str;

        if constexpr ( std::is_enum_v<T> )
        {
            Value1Str = std::to_string( std::underlying_type_t<T>( Value1 ) );
            Value2Str = std::to_string( std::underlying_type_t<T>( Value2 ) );
        }
        else
        {
            Value1Str = std::to_string( Value1 );
            Value2Str = std::to_string( Value2 );
        }

        ( void )snprintf( GSklAssertWorkBuffer.data()
                        , GSklAssertWorkBuffer.size()
                        , "\u001b[31mEquality Assert \"%s\" failed!\nValue1:%s Value2:%s\nAt:%s:%llu \n\u001b[37m"
                        , InExpressionAsString
                        , Value1Str.c_str()
                        , Value2Str.c_str()
                        , InFileName
                        , LineNumber );
        ( void )printf( GSklAssertWorkBuffer.data() );
        SKL_BREAK();
        abort();
    }
    
    SKL_NOINLINE SKL_NORETURN inline void SklAssertFailHandler( const char* InExpressionAsString, const char* InFileName, size_t LineNumber, const char* InMessage ) noexcept
    {
        ( void )snprintf( GSklAssertWorkBuffer.data(), GSklAssertWorkBuffer.size(), "\u001b[31mAssert \"%s\" failed!\nAt:%s:%llu \nMessage:%s\n\u001b[37m", InExpressionAsString, InFileName, LineNumber, InMessage );
        ( void )printf( GSklAssertWorkBuffer.data() );
        SKL_BREAK();
        abort();
    }
}

#if defined(SKL_BUILD_SHIPPING)
    #define SKL_ASSERT(expression)
    #define SKL_ASSERT_MSG(expression, msg)
    #define SKL_ASSERT_EQUAL(expression1, expression2)
#else
    #define SKL_ASSERT(expression)                                             \
        if( false == !!( expression ) ) SKL_UNLIKELY                           \
        {                                                                      \
            SKL::SklAssertFailHandler( #expression, __FILE__, __LINE__ );      \
        }

    #define SKL_ASSERT_MSG(expression, msg)                                    \
        if( false == !!( expression ) ) SKL_UNLIKELY                           \
        {                                                                      \
            SKL::SklAssertFailHandler( #expression, __FILE__, __LINE__, msg ); \
        }
        
    #define SKL_ASSERT_EQUAL(expression1, expression2)                                                                                        \
    {                                                                                                                                         \
        const auto Expression1Result = expression1;                                                                                           \
        const auto Expression2Result = expression2;                                                                                           \
        const bool bEqualResult{ Expression1Result == Expression2Result };                                                                    \
        if( false == bEqualResult ) SKL_UNLIKELY                                                                                              \
        {                                                                                                                                     \
            SKL::SklAssertEqualityFailHandler( Expression1Result, Expression2Result, #expression1 " == " #expression2, __FILE__, __LINE__ );  \
        }                                                                                                                                     \
    }

#endif

#if !defined(SKL_NO_ASSERTS)
    #define SKL_ASSERT_ALLWAYS(expression)                                     \
        if( false == !!( expression ) ) SKL_UNLIKELY                           \
        {                                                                      \
            SKL::SklAssertFailHandler( #expression, __FILE__, __LINE__ );      \
        }
        
    #define SKL_ASSERT_ALLWAYS_MSG(expression, msg)                            \
        if( false == !!( expression ) ) SKL_UNLIKELY                           \
        {                                                                      \
            SKL::SklAssertFailHandler( #expression, __FILE__, __LINE__, msg ); \
        }
#else
    #define SKL_ASSERT_ALLWAYS(expression)
    #define SKL_ASSERT_ALLWAYS_MSG(expression, msg)
#endif


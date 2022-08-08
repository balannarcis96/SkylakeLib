//!
//! \file AdvancedSignleDispatch.h
//! 
//! \brief Header only version of the Advanced Signle Dispatch project
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
//#pragma once -- Not needed for SkylakeLibHeaderOnly

#include <type_traits>
#include <memory>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
	#define ASD_FORCEINLINE __attribute__((always_inline))
	#define ASD_NOINLINE __attribute__( ( noinline ) )
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
	#define ASD_FORCEINLINE __forceinline
	#if( _MSC_VER >= 1300 )
		#define ASD_NOINLINE __declspec( noinline )
	#else
		#define ASD_NOINLINE
	#endif
#else
	#define ASD_FORCEINLINE 
#endif

//c++17 and latter
#define ASD_NODISCARD [[nodiscard]]

#if __cplusplus > 201703 
#define ASD_UNLIKELY [[unlikely]]
#define ASD_LIKELY [[likely]]
#define ASD_FALLTHROUGH [[fallthrough]]
#else
#define ASD_UNLIKELY 
#define ASD_LIKELY
#define ASD_FALLTHROUGH
#endif

#if defined(__INTEL_COMPILER)
    #define ASD_COMPILER_NAME "intel"
    #error "Unsupported yet!"
#elif defined(__GNUC__) && !defined(__clang__)
    #define ASD_COMPILER_NAME "gcc"
    #define ASD_CDECL      /* __attribute__((cdecl))     */ // GCC DOESNT ALLOW TO SPECIFY THE CALLING CONVENTION?
    #define ASD_STDCALL    /* __attribute__((stdcall))   */ // GCC DOESNT ALLOW TO SPECIFY THE CALLING CONVENTION?
    #define ASD_THISCALL   /* __attribute__((thiscall))  */ // GCC DOESNT ALLOW TO SPECIFY THE CALLING CONVENTION?
    #define ASD_FASTCALL   /* __attribute__((fastcall))  */ // GCC DOESNT ALLOW TO SPECIFY THE CALLING CONVENTION?
    #define ASD_VECTORCALL /* __attribute__((vectorcall))*/ // GCC DOESNT ALLOW TO SPECIFY THE CALLING CONVENTION?
    #if defined(_WIN32)
        #error "Unsupported yet!"
    #endif
#elif defined(__clang__)
    #define ASD_COMPILER_NAME "clang"
    #define ASD_CDECL      __attribute__((cdecl))
    #define ASD_STDCALL    __attribute__((cdecl))
    #define ASD_THISCALL   __attribute__((cdecl))
    #define ASD_FASTCALL   __attribute__((cdecl))
    #define ASD_VECTORCALL __attribute__((vectorcall))
    #if defined(_WIN32)
        #error "Unsupported yet!"
    #endif
#elif defined(_MSC_VER)
    #define ASD_COMPILER_NAME "msvc"
    #define ASD_CDECL      __cdecl
    #define ASD_STDCALL    __stdcall
    #define ASD_THISCALL   __thiscall
    #define ASD_FASTCALL   __fastcall
    #define ASD_VECTORCALL __vectorcall
    #if !defined(_WIN32)
        #error "Unsupported yet!"
    #endif
#else
    #define ASD_COMPILER_NAME "unknown"
    #error "Unsupported yet!"
#endif

namespace ASD
{
	struct CallingConventions
	{
	    using Cdecl      = void( ASD_CDECL* )();
	    using Stdcall    = void( ASD_STDCALL* )();
	    using Thiscall   = void( ASD_THISCALL* )();
	    using Fastcall   = void( ASD_FASTCALL* )();
	    using Vectorcall = void( ASD_VECTORCALL* )();

		template<typename T>
	    static constexpr bool IsCdecl       = std::is_same_v<Cdecl, T>;
	    static constexpr bool HasStdcall    = !IsCdecl<Stdcall>;
	    static constexpr bool HasThiscall   = !IsCdecl<Thiscall>;
	    static constexpr bool HasFastcall   = !IsCdecl<Fastcall>;
	    static constexpr bool HasVectorcall = !IsCdecl<Vectorcall>;
	};
}

namespace ASD
{
	template<typename TCallingConvention, typename ReturnType, typename ...Args>
	struct FunctionTraitsBase
	{
		using TReturnType					   = ReturnType;
		using TArgsTypes					   = std::tuple< Args... >;
		template<std::size_t N> using TNthArg  = std::tuple_element_t< N, TArgsTypes >;
		using CallingConvention				   = TCallingConvention;

		static constexpr std::size_t ArgsCount  = sizeof...( Args );
		static constexpr bool		 IsMethod   = false;
		static constexpr bool		 IsDelegate = false;

		static constexpr const char* GetCallingConventionName()
		{
			if ( std::is_same<CallingConvention, CallingConventions::Vectorcall>() && CallingConventions::HasVectorcall ) { return "vectorcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Fastcall>()   && CallingConventions::HasFastcall ) { return "fastcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Thiscall>()   && CallingConventions::HasThiscall ) { return "thiscall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Stdcall>()    && CallingConventions::HasStdcall ) { return "stdcall"; }
			
			return "cdecl";
		}
	};

	template<typename F, int = 0>
	struct FunctionTraits;

	template< typename ReturnType, typename ...Args >																			
	struct FunctionTraits< ReturnType( ASD_CDECL * )(Args...), 0 >																
		: FunctionTraitsBase< CallingConventions::Cdecl, ReturnType, Args... >													
	{																															
		using TBase						 = FunctionTraitsBase< CallingConventions::Cdecl, ReturnType, Args... >;				
		using TPointer		             = ReturnType ( ASD_CDECL * )(Args...);													
		constexpr static bool IsNoexcept = false;																				
		constexpr static bool IsVariadic = !std::is_same_v< void( ASD_CDECL * )(Args...), void( ASD_CDECL * )( Args... ) >;		
																																
		FunctionTraits() : Pointer{ nullptr } {}																				
		FunctionTraits( TPointer Pointer ) : Pointer{ Pointer } {}																
		FunctionTraits( const FunctionTraits& Other ) = default;																
																																
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const													
		{																														
			if constexpr ( !std::is_void<ReturnType>() )																		
				return Pointer( std::forward<Args>( args )... );																
			else																												
				Pointer( std::forward<Args>( args )... );																		
		}																														
protected:																														
		TPointer Pointer;																										
	};																															
	
	template< typename ReturnType, typename ...Args >																			
	struct FunctionTraits< ReturnType( ASD_CDECL * )(Args...) noexcept, 0 >														
		: FunctionTraitsBase< CallingConventions::Cdecl, ReturnType, Args... >													
	{																															
		using TBase						 = FunctionTraitsBase< CallingConventions::Cdecl, ReturnType, Args... >;				
		using TPointer		             = ReturnType ( ASD_CDECL * )(Args...) noexcept;										
		constexpr static bool IsNoexcept = true;																				
		constexpr static bool IsVariadic = !std::is_same_v< void( ASD_CDECL * )(Args...), void( ASD_CDECL * )( Args... ) >;		
																																
		FunctionTraits() : Pointer{ nullptr } {}																				
		FunctionTraits( TPointer Pointer ) : Pointer{ Pointer } {}																
		FunctionTraits( const FunctionTraits& Other ) = default;																
																																
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const noexcept											
		{																														
			if constexpr ( !std::is_void<ReturnType>() )																		
				return Pointer( std::forward<Args>( args )... );																
			else																												
				Pointer( std::forward<Args>( args )... );																		
		}																														
protected:																														
		TPointer Pointer;																										
	};

	//! As in some cases most calling conventions collapse to __cdecl we need to handle the type redefinition
#define ASD_MAKE_TRAITS( CallingConv, CallingConvType )																		    \
	template< typename ReturnType, typename ...Args >																			\
	struct FunctionTraits< ReturnType( CallingConv * )(Args...),																\
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ : 0 >									\
		: FunctionTraitsBase< CallingConvType, ReturnType, Args... >															\
	{																															\
		using TBase						 = FunctionTraitsBase< CallingConvType, ReturnType, Args... >;							\
		using TPointer		             = ReturnType ( CallingConv * )(Args...);												\
		constexpr static bool IsNoexcept = false;																				\
		constexpr static bool IsVariadic = !std::is_same_v< void( CallingConv * )(Args...), void( CallingConv * )(Args...) >;	\
																																\
		FunctionTraits() : Pointer{ nullptr } {}																				\
		FunctionTraits( TPointer Pointer ) : Pointer{ Pointer } {}																\
		FunctionTraits( const FunctionTraits& Other ) = default;																\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( Args... args ) const													\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( std::forward<Args>( args )... );																\
			else																												\
				Pointer( std::forward<Args>( args )... );																		\
		}																														\
protected:																														\
		TPointer Pointer;																										\
	};																															\
	template< typename ReturnType, typename ...Args >																			\
	struct FunctionTraits< ReturnType( CallingConv * )(Args...) noexcept,														\
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ : 0 >									\
		: FunctionTraitsBase< CallingConvType, ReturnType, Args... >															\
	{																															\
		using TBase						 = FunctionTraitsBase< CallingConvType, ReturnType, Args... >;							\
		using TPointer		             = ReturnType ( CallingConv * )(Args...) noexcept;										\
		constexpr static bool IsNoexcept = true;																				\
		constexpr static bool IsVariadic = !std::is_same_v< void( CallingConv * )(Args...), void( CallingConv * )(Args...) >;	\
																																\
		FunctionTraits() : Pointer{ nullptr } {}																				\
		FunctionTraits( TPointer Pointer ) : Pointer{ Pointer } {}																\
		FunctionTraits( const FunctionTraits& Other ) = default;																\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( Args... args ) const	noexcept										\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( std::forward<Args>( args )... );																\
			else																												\
				Pointer( std::forward<Args>( args )... );																		\
		}																														\
protected:																														\
		TPointer Pointer;																										\
	};

	// Function Traits (variadic functions not supported!)
	ASD_MAKE_TRAITS( ASD_STDCALL,	 CallingConventions::Stdcall );
	ASD_MAKE_TRAITS( ASD_THISCALL,   CallingConventions::Thiscall );
	ASD_MAKE_TRAITS( ASD_VECTORCALL, CallingConventions::Vectorcall );
	ASD_MAKE_TRAITS( ASD_FASTCALL,   CallingConventions::Fastcall );

#undef ASD_MAKE_TRAITS
}

namespace ASD
{
    //!
    //! Simple function pointer wrapper
    //!
    //! \details Usage: FnPtr<void(*)( int )> ptr = DoSmth;
    //! \details sizeof( FnPtr< ... > ) == sizeof( void * )
    //!
    template<typename TFnPtrSignature>
    class FnPtr : public FunctionTraits<TFnPtrSignature>
    {
    public:
        using TFunctionTraits   = FunctionTraits<TFnPtrSignature>;
        using TFunctionPointer  = typename TFunctionTraits::TPointer;
        using TReturnType       = typename TFunctionTraits::TReturnType;
        using TArgsTypes        = typename TFunctionTraits::TArgsTypes;
		using CallingConvention	= typename TFunctionTraits::CallingConvention;

		static constexpr std::size_t ArgsCount = TFunctionTraits::ArgsCount;
        constexpr static bool IsNoexcept = TFunctionTraits::IsNoexcept;
		constexpr static bool IsVariadic = TFunctionTraits::IsVariadic;
        
		static constexpr const char* GetCallingConventionName()
		{
			return TFunctionTraits::GetCallingConventionName();
		}

        FnPtr() : TFunctionTraits( nullptr ) {}
        FnPtr( TFunctionPointer Pointer ) :  TFunctionTraits( Pointer ) {}
        
        ASD_FORCEINLINE void operator =( TFunctionPointer InPointer )
        {
             this->Pointer = InPointer;
        }

	    ASD_FORCEINLINE bool operator==( TFunctionPointer InPointer ) const
	    {
	        return  this->Pointer == InPointer;
	    }

        ASD_FORCEINLINE explicit operator bool() const
        {
	        return false == IsNull();
        }

        ASD_NODISCARD bool IsNull() const
        {
	        return nullptr == this->Pointer;
        }

        static_assert( false == ASD::CallingConventions::HasThiscall || std::is_same_v<ASD::CallingConventions::Thiscall, CallingConvention>, "This call is unsupported!" );
    };
    
    static_assert( sizeof ( FnPtr<void( ASD_CDECL* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_THISCALL* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_FASTCALL* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_VECTORCALL* )( void )> ) == sizeof( void * ) );
                                            
    static_assert( sizeof ( FnPtr<void( ASD_CDECL* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_THISCALL* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_FASTCALL* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( FnPtr<void( ASD_VECTORCALL* )( void ) noexcept> ) == sizeof( void * ) );
}

namespace ASD
{
	template<typename TCallingConvention, typename TClass, typename ReturnType, typename ...Args>
	struct MethodTraitsBase
	{
		using TReturnType					   = ReturnType;
		using TClassType					   = TClass;
		using TArgsTypes					   = std::tuple< Args... >;
		template<std::size_t N> using TNthArg  = std::tuple_element_t< N, TArgsTypes >;
		using CallingConvention				   = TCallingConvention;

		static constexpr std::size_t ArgsCount  = sizeof...( Args );
		static constexpr bool		 IsMethod   = true;
		static constexpr bool		 IsDelegate = false;

		static constexpr const char* GetCallingConventionName()
		{
			if ( std::is_same<CallingConvention, CallingConventions::Vectorcall>() && CallingConventions::HasVectorcall ) { return "vectorcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Fastcall>()   && CallingConventions::HasFastcall ) { return "fastcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Thiscall>()   && CallingConventions::HasThiscall ) { return "thiscall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Stdcall>()    && CallingConventions::HasStdcall ) { return "stdcall"; }
			
			return "cdecl";
		}
	};

	template<typename F, int = 0>
	struct MethodTraits;

	template<typename TClass, typename ReturnType, typename ...Args>															
	struct MethodTraits<ReturnType( ASD_CDECL TClass::* )( Args... ), 0>														
		: MethodTraitsBase<CallingConventions::Cdecl, TClass, ReturnType, Args...>													
	{																														
		using TBase						 = MethodTraitsBase< CallingConventions::Cdecl, TClass, ReturnType, Args... >;					
		using TPointer		             = ReturnType ( ASD_CDECL * )( void*, Args... );									
		using TMethodPointer		     = ReturnType ( ASD_CDECL TClass::* )( Args... );		
		constexpr static bool IsNoexcept = false;																			
		constexpr static bool IsVariadic = false;
																															
		MethodTraits() : Pointer{ nullptr } {}																				
		MethodTraits( TMethodPointer Pointer ) : Pointer{ *reinterpret_cast<TPointer*>( &Pointer ) } {}										
		MethodTraits( const MethodTraits& Other ) = default;																
																															
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( TClass* Instance, Args... args ) const												
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( Instance, std::forward<Args>( args )... );															
			else																											
				Pointer( Instance, std::forward<Args>( args )... );																	
		}		

		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( TClass& Instance, Args... args ) const												
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( &Instance, std::forward<Args>( args )... );															
			else																											
				Pointer( &Instance, std::forward<Args>( args )... );																	
		}	

protected:																													
		TPointer Pointer;																									
	};		
	
	template<typename TClass, typename ReturnType, typename ...Args>															
	struct MethodTraits<ReturnType( ASD_CDECL TClass::* )( Args... ) noexcept, 0>														
		: MethodTraitsBase<CallingConventions::Cdecl, TClass, ReturnType, Args...>													
	{																														
		using TBase						 = MethodTraitsBase<CallingConventions::Cdecl, TClass, ReturnType, Args...>;					
		using TPointer		             = ReturnType ( ASD_CDECL * )( void*, Args... ) noexcept;									
		using TMethodPointer		     = ReturnType ( ASD_CDECL TClass::* )( Args... ) noexcept;									
		constexpr static bool IsNoexcept = true;																			
		constexpr static bool IsVariadic = false;
																															
		MethodTraits() : Pointer{ nullptr } {}																				
		MethodTraits( TMethodPointer Pointer ) : Pointer{ *reinterpret_cast<TPointer*>( &Pointer ) } {}															
		MethodTraits( const MethodTraits& Other ) = default;																
																															
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( TClass* Instance, Args... args ) const	noexcept											
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( Instance, std::forward<Args>( args )... );															
			else																											
				Pointer( Instance, std::forward<Args>( args )... );																	
		}		

		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( TClass& Instance, Args... args ) const noexcept											
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( &Instance, std::forward<Args>( args )... );															
			else																											
				Pointer( &Instance, std::forward<Args>( args )... );																	
		}	

protected:																													
		TPointer Pointer;																									
	};

	//! As in some cases most calling conventions collapse to __cdecl we need to handle the type redefinition
#define ASD_MAKE_TRAITS( CallingConv, CallingConvType )																			\
	template<typename TClass, typename ReturnType, typename ...Args>															\
	struct MethodTraits<ReturnType( CallingConv TClass::* )( Args... ),															\
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ : 0 >									\
		: MethodTraitsBase< CallingConventions::Cdecl, TClass, ReturnType, Args... >											\
	{																															\
		using TBase						 = MethodTraitsBase< CallingConvType, TClass, ReturnType, Args... >;					\
		using TPointer		             = ReturnType ( CallingConv * )( void*, Args... );										\
		using TMethodPointer		     = ReturnType ( CallingConv TClass::* )( Args... );										\
		constexpr static bool IsNoexcept = false;																				\
		constexpr static bool IsVariadic = false;																				\
																																\
		MethodTraits() : Pointer{ nullptr } {}																					\
		MethodTraits( TMethodPointer Pointer ) : Pointer{ *reinterpret_cast<TPointer*>( &Pointer ) } {}							\
		MethodTraits( const MethodTraits& Other ) = default;																	\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( TClass* Instance, Args... args ) const								\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( Instance, std::forward<Args>( args )... );														\
			else																												\
				Pointer( Instance, std::forward<Args>( args )... );																\
		}																														\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( TClass& Instance, Args... args ) const								\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( &Instance, std::forward<Args>( args )... );														\
			else																												\
				Pointer( &Instance, std::forward<Args>( args )... );															\
		}																														\
																																\
protected:																														\
		TPointer Pointer;																										\
	};																															\
	template<typename TClass, typename ReturnType, typename ...Args>															\
	struct MethodTraits<ReturnType( CallingConv TClass::* )( Args... ) noexcept,												\
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ + 50 : 0 >								\
		: MethodTraitsBase< CallingConventions::Cdecl, TClass, ReturnType, Args... >											\
	{																															\
		using TBase						 = MethodTraitsBase< CallingConventions::Cdecl, TClass, ReturnType, Args... >;			\
		using TPointer		             = ReturnType ( CallingConv * )( void*, Args... ) noexcept;								\
		using TMethodPointer		     = ReturnType ( CallingConv TClass::* )( Args... ) noexcept;							\
		constexpr static bool IsNoexcept = true;																				\
		constexpr static bool IsVariadic = false;																				\
																																\
		MethodTraits() : Pointer{ nullptr } {}																					\
		MethodTraits( TMethodPointer Pointer ) : Pointer{ *reinterpret_cast<TPointer*>( &Pointer ) } {}							\
		MethodTraits( const MethodTraits& Other ) = default;																	\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( TClass* Instance, Args... args ) const	noexcept					\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( Instance, std::forward<Args>( args )... );														\
			else																												\
				Pointer( Instance, std::forward<Args>( args )... );																\
		}																														\
																																\
		ASD_FORCEINLINE ReturnType CallingConv operator()( TClass& Instance, Args... args ) const noexcept						\
		{																														\
			if constexpr ( !std::is_void<ReturnType>() )																		\
				return Pointer( &Instance, std::forward<Args>( args )... );														\
			else																												\
				Pointer( &Instance, std::forward<Args>( args )... );															\
		}																														\
																																\
protected:																														\
		TPointer Pointer;																										\
	};

	// Function Traits (variadic functions not supported!)
	ASD_MAKE_TRAITS( ASD_STDCALL,	 CallingConventions::Stdcall );
	ASD_MAKE_TRAITS( ASD_THISCALL,   CallingConventions::Thiscall );
	ASD_MAKE_TRAITS( ASD_VECTORCALL, CallingConventions::Vectorcall );
	ASD_MAKE_TRAITS( ASD_FASTCALL,   CallingConventions::Fastcall );

#undef ASD_MAKE_TRAITS
}

namespace ASD
{
    class DummyClass;

    //!
    //! Simple class function pointer(method) wrapper
    //!
    //! \details Usage: MethodPtr<void(TClass::*)( int )> ptr = &TClass::DoSmth; ptr( Instance, ... )
    //! \details sizeof( MethodPtr< ... > ) == sizeof( void * )
    //!
    template<typename TFnPtrSignature>
    class MethodPtr : public MethodTraits<TFnPtrSignature>
    {
    public:
        using TMethodTraits     = MethodTraits<TFnPtrSignature>;
        using TFunctionPointer  = typename TMethodTraits::TPointer;
        using TMethodPointer    = typename TMethodTraits::TMethodPointer;
        using TReturnType       = typename TMethodTraits::TReturnType;
        using TArgsTypes        = typename TMethodTraits::TArgsTypes;
		using CallingConvention	= typename TMethodTraits::CallingConvention;
        using TClass            = typename TMethodTraits::TClassType;

		static constexpr std::size_t ArgsCount = TMethodTraits::ArgsCount;
        constexpr static bool IsNoexcept       = TMethodTraits::IsNoexcept;
		constexpr static bool IsVariadic       = TMethodTraits::IsVariadic;
        
		static constexpr const char* GetCallingConventionName()
		{
			return TMethodTraits::GetCallingConventionName();
		}

        MethodPtr() : TMethodTraits() {}
        MethodPtr( TMethodPointer Pointer ) : TMethodTraits( Pointer ) {}
	        
	    ASD_FORCEINLINE void operator=( TMethodPointer InPointer )
	    {
	        this->Pointer = *reinterpret_cast<TFunctionPointer*>( &InPointer );
	    }
        
	    ASD_FORCEINLINE bool operator==( TMethodPointer InPointer ) const
	    {
	        return this->Pointer == *reinterpret_cast<TFunctionPointer*>( &InPointer );
	    }

        ASD_FORCEINLINE explicit operator bool() const
        {
	        return false == IsNull(  );
        }

        ASD_NODISCARD bool IsNull() const
        {
	        return this->Pointer == nullptr;
        }
    };

    static_assert( sizeof ( MethodPtr<void( DummyClass::*)( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_CDECL DummyClass::* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_THISCALL DummyClass::* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_STDCALL DummyClass::* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_FASTCALL DummyClass::* )( void )> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_VECTORCALL DummyClass::* )( void )> ) == sizeof( void * ) );

    static_assert( sizeof ( MethodPtr<void( DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_CDECL DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_THISCALL DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_STDCALL DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_FASTCALL DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
    static_assert( sizeof ( MethodPtr<void( ASD_VECTORCALL DummyClass::* )( void ) noexcept> ) == sizeof( void * ) );
}

namespace ASD
{
	template<template<typename ...> typename PointerWrapper, typename TCallingConvention, typename TClass, typename ReturnType, typename ...Args>
	struct DelegateTraitsBase
	{
		using TReturnType					   = ReturnType;
		using TClassType					   = TClass;
		using TArgsTypes					   = std::tuple< Args... >;
		template<std::size_t N> using TNthArg  = std::tuple_element_t< N, TArgsTypes >;
		using CallingConvention				   = TCallingConvention;
		template<typename ...TArgs>
		using TPointerWrapper                  = std::conditional_t<std::is_void_v<PointerWrapper<TArgs...>>, TClass*, PointerWrapper<TArgs...>>;
		using TInstancePointer                 = TPointerWrapper<TClass>;

		static constexpr std::size_t ArgsCount  = sizeof...( Args );
		static constexpr bool		 IsMethod   = true;
		static constexpr bool		 IsDelegate = true;

		static constexpr const char* GetCallingConventionName()
		{
			if ( std::is_same<CallingConvention, CallingConventions::Vectorcall>() && CallingConventions::HasVectorcall ) { return "vectorcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Fastcall>()   && CallingConventions::HasFastcall ) { return "fastcall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Thiscall>()   && CallingConventions::HasThiscall ) { return "thiscall"; }
			if ( std::is_same<CallingConvention, CallingConventions::Stdcall>()    && CallingConventions::HasStdcall ) { return "stdcall"; }
			
			return "cdecl";
		}
	};

	template<template<typename ...> typename TPointerWrapper, typename F, int = 0>
	struct DelegateTraits;

	template<template<typename ...> typename TPointerWrapper, typename TClass, typename ReturnType, typename ...Args>															
	struct DelegateTraits<TPointerWrapper, ReturnType( ASD_CDECL TClass::* )( Args... ), 0>														
		: DelegateTraitsBase<TPointerWrapper, CallingConventions::Cdecl, TClass, ReturnType, Args...>													
	{																														
		using TBase						 = DelegateTraitsBase<TPointerWrapper, CallingConventions::Cdecl, TClass, ReturnType, Args...>;					
		using TPointer		             = ReturnType ( ASD_CDECL * )( void*, Args... );									
		using TMethodPointer		     = ReturnType ( ASD_CDECL TClass::* )( Args... );		
		using TInstancePointer           = typename TBase::TInstancePointer;
		constexpr static bool IsNoexcept = false;																			
		constexpr static bool IsVariadic = false;
																													
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const									
		{																											
			if constexpr ( !std::is_void<ReturnType>() )																	
                if constexpr( std::is_class<TInstancePointer>() )
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );															
                else
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );															
			else																											
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																	
		}		
																																
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( Args... args ) const								
		{																											
			if constexpr ( !std::is_void<ReturnType>() )																	
                if constexpr( std::is_class<TInstancePointer>() )
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );															
                else
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );															
			else																											
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																	
		}		
																														
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( TClass* OtherInstancePointer, Args... args ) const										
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( OtherInstancePointer, std::forward<Args>( args )... );															
			else																											
				Pointer( OtherInstancePointer, std::forward<Args>( args )... );																	
		}		
																													
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( TClass& OtherInstance, Args... args ) const								
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( &OtherInstance, std::forward<Args>( args )... );															
			else																											
				Pointer( &OtherInstance, std::forward<Args>( args )... );																	
		}		

protected:																													
		TPointer		 Pointer;	
		TInstancePointer InstancePointer;
	};		
	
	template<template<typename ...> typename TPointerWrapper, typename TClass, typename ReturnType, typename ...Args>															
	struct DelegateTraits<TPointerWrapper, ReturnType( ASD_CDECL TClass::* )( Args... ) noexcept, 0>														
		: DelegateTraitsBase<TPointerWrapper, CallingConventions::Cdecl, TClass, ReturnType, Args...>													
	{																														
		using TBase						 = DelegateTraitsBase<TPointerWrapper, CallingConventions::Cdecl, TClass, ReturnType, Args...>;					
		using TPointer		             = ReturnType ( ASD_CDECL * )( void*, Args... ) noexcept;									
		using TMethodPointer		     = ReturnType ( ASD_CDECL TClass::* )( Args... ) noexcept;									
		using TInstancePointer           = typename TBase::TInstancePointer;
		constexpr static bool IsNoexcept = true;																			
		constexpr static bool IsVariadic = false;
																																		
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const noexcept											
		{																											
			if constexpr ( !std::is_void<ReturnType>() )																	
                if constexpr( std::is_class<TInstancePointer>() )
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );															
                else
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );															
			else																											
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																	
		}		
																																
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( Args... args ) const	noexcept											
		{																											
			if constexpr ( !std::is_void<ReturnType>() )																	
                if constexpr( std::is_class<TInstancePointer>() )
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );															
                else
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );															
			else																											
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																	
		}		
																														
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( TClass* OtherInstancePointer, Args... args ) const noexcept											
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( OtherInstancePointer, std::forward<Args>( args )... );															
			else																											
				Pointer( OtherInstancePointer, std::forward<Args>( args )... );																	
		}		
																													
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( TClass& OtherInstance, Args... args ) const	noexcept											
		{																													
			if constexpr ( !std::is_void<ReturnType>() )																	
				return Pointer( &OtherInstance, std::forward<Args>( args )... );															
			else																											
				Pointer( &OtherInstance, std::forward<Args>( args )... );																	
		}		

protected:																													
		TPointer		 Pointer;	
		TInstancePointer InstancePointer;																									
	};

	//! As in some cases most calling conventions collapse to __cdecl we need to handle the type redefinition
#define ASD_MAKE_TRAITS( CallingConv, CallingConvType )																																															  \
																																																												  \
	template<template<typename ...> typename TPointerWrapper, typename TClass, typename ReturnType, typename ...Args>																															  \
	struct DelegateTraits<TPointerWrapper, ReturnType( CallingConv TClass::* )( Args... ), 																																					      \
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ : 0 >																																					  \
		: DelegateTraitsBase<TPointerWrapper, CallingConvType, TClass, ReturnType, Args...>																																						  \
	{																																																											  \
		using TBase						 = DelegateTraitsBase<TPointerWrapper, CallingConvType, TClass, ReturnType, Args...>;																											          \
		using TPointer		             = ReturnType ( CallingConv * )( void*, Args... );																																						  \
		using TMethodPointer		     = ReturnType ( CallingConv TClass::* )( Args... );																																						  \
		using TInstancePointer           = typename TBase::TInstancePointer;																																									  \
		constexpr static bool IsNoexcept = false;																																																  \
		constexpr static bool IsVariadic = false;																																																  \
																																																												  \
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const noexcept																																						      \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
                if constexpr( std::is_class<TInstancePointer>() )																																												  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );																										  \
                else																																																							  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																											  \
			else																																																								  \
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																														  \
		}																																																										  \
																																																												  \
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( Args... args ) const	noexcept																																							  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
                if constexpr( std::is_class<TInstancePointer>() )																																												  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );																										  \
                else																																																							  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																											  \
			else																																																								  \
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																														  \
		}																																																										  \
																																																											  	  \
		ASD_FORCEINLINE ReturnType CallingConv Dispatch( TClass* OtherInstancePointer, Args... args ) const																																		  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
				return Pointer( OtherInstancePointer, std::forward<Args>( args )... );																																							  \
			else																																																								  \
				Pointer( OtherInstancePointer, std::forward<Args>( args )... );																																									  \
		}																																																										  \
																																																												  \
		ASD_FORCEINLINE ReturnType CallingConv Dispatch( TClass& OtherInstance, Args... args ) const																																			  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
				return Pointer( &OtherInstance, std::forward<Args>( args )... );																																								  \
			else																																																								  \
				Pointer( &OtherInstance, std::forward<Args>( args )... );																																										  \
		}																																																										  \
																																																												  \
protected:																																																										  \
		TPointer		 Pointer;																																																				  \
		TInstancePointer InstancePointer;																																																		  \
	};																																																											  \
	template<template<typename ...> typename TPointerWrapper, typename TClass, typename ReturnType, typename ...Args>																															  \
	struct DelegateTraits<TPointerWrapper, ReturnType( CallingConv TClass::* )( Args... ) noexcept, 																																			  \
				std::is_same_v< CallingConventions::Cdecl, CallingConvType > ? __LINE__ + 50 : 0 >																																		          \
		: DelegateTraitsBase<TPointerWrapper, CallingConvType, TClass, ReturnType, Args...>																																						  \
	{																																																											  \
		using TBase						 = DelegateTraitsBase<TPointerWrapper, CallingConvType, TClass, ReturnType, Args...>;																													  \
		using TPointer		             = ReturnType ( CallingConv * )( void*, Args... ) noexcept;																																				  \
		using TMethodPointer		     = ReturnType ( CallingConv TClass::* )( Args... ) noexcept;																																		      \
		using TInstancePointer           = typename TBase::TInstancePointer;																																									  \
		constexpr static bool IsNoexcept = true;																																																  \
		constexpr static bool IsVariadic = false;																																																  \
																																																												  \
		ASD_FORCEINLINE ReturnType ASD_CDECL operator()( Args... args ) const noexcept																																						      \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
                if constexpr( std::is_class<TInstancePointer>() )																																												  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );																										  \
                else																																																							  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																											  \
			else																																																								  \
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																														  \
		}																																																										  \
																																																												  \
		ASD_FORCEINLINE ReturnType ASD_CDECL Dispatch( Args... args ) const	noexcept																																							  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
                if constexpr( std::is_class<TInstancePointer>() )																																												  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer.get() ) ), std::forward<Args>( args )... );																										  \
                else																																																							  \
				    return Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																											  \
			else																																																								  \
				Pointer( const_cast<void*>(static_cast<const void*>( InstancePointer ) ), std::forward<Args>( args )... );																														  \
		}																																																										  \
																																																												  \
		ASD_FORCEINLINE ReturnType CallingConv Dispatch( TClass* OtherInstancePointer, Args... args ) const	noexcept																															  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
				return Pointer( OtherInstancePointer, std::forward<Args>( args )... );																																							  \
			else																																																								  \
				Pointer( OtherInstancePointer, std::forward<Args>( args )... );																																									  \
		}																																																										  \
																																																												  \
		ASD_FORCEINLINE ReturnType CallingConv Dispatch( TClass& OtherInstance, Args... args ) const noexcept																																	  \
		{																																																										  \
			if constexpr ( !std::is_void<ReturnType>() )																																														  \
				return Pointer( &OtherInstance, std::forward<Args>( args )... );																																								  \
			else																																																								  \
				Pointer( &OtherInstance, std::forward<Args>( args )... );																																										  \
		}																																																										  \
																																																												  \
protected:																																																										  \
		TPointer		 Pointer;																																																				  \
		TInstancePointer InstancePointer;																																																		  \
	};																																																											  
																																																												  
	// Function Traits (variadic functions not supported!)
	ASD_MAKE_TRAITS( ASD_STDCALL,	 CallingConventions::Stdcall );
	ASD_MAKE_TRAITS( ASD_THISCALL,   CallingConventions::Thiscall );
	ASD_MAKE_TRAITS( ASD_VECTORCALL, CallingConventions::Vectorcall );
	ASD_MAKE_TRAITS( ASD_FASTCALL,   CallingConventions::Fastcall );

#undef ASD_MAKE_TRAITS
}

namespace ASD
{
	//!
    //! Base class for all delegate types
    //!
    //! \details See RawDelegate< ... >, UniqueDelegate< ... >, SharedDelegate< ... >
    //! \details sizeof( DelegateBase< ... > ) == sizeof( void * ) * 2
    //!
    template<template<typename ...> typename TPointerWrapper, typename TFnPtrSignature>
    class DelegateBase : public DelegateTraits<TPointerWrapper, TFnPtrSignature>
    {
    public:
        using TDelegateTraits    = DelegateTraits<TPointerWrapper, TFnPtrSignature>;
        using TFunctionPointer   = typename TDelegateTraits::TPointer;
        using TMethodPointer     = typename TDelegateTraits::TMethodPointer;
        using TReturnType        = typename TDelegateTraits::TReturnType;
        using TArgsTypes         = typename TDelegateTraits::TArgsTypes;
		using CallingConvention	 = typename TDelegateTraits::CallingConvention;
        using TClass             = typename TDelegateTraits::TClassType;
		using TInstancePointer   = typename TDelegateTraits::TInstancePointer;
        
		static constexpr std::size_t ArgsCount = TDelegateTraits::ArgsCount;
        constexpr static bool IsNoexcept       = TDelegateTraits::IsNoexcept;
		constexpr static bool IsVariadic       = TDelegateTraits::IsVariadic;
                
#if defined(_MSC_VER)
        DelegateBase() : DelegateTraits() {}
        DelegateBase( TMethodPointer MethodPointer ) : DelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = nullptr;          
        }
        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        DelegateBase( TMethodPointer MethodPointer, const TInstancePointer& InInstancePointer ) : DelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = InInstancePointer;          
        }
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        DelegateBase( TMethodPointer MethodPointer, TInstancePointer&& InInstancePointer ) : DelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = std::forward<TInstancePointer>( InInstancePointer );          
        }

#else
        DelegateBase() : TDelegateTraits() {}
        DelegateBase( TMethodPointer MethodPointer ) : TDelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = nullptr;          
        }
        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        DelegateBase( TMethodPointer MethodPointer, const TInstancePointer& InInstancePointer ) : TDelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = InInstancePointer;          
        }
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        DelegateBase( TMethodPointer MethodPointer, TInstancePointer&& InInstancePointer ) : TDelegateTraits() 
        {
            this->Pointer         = *reinterpret_cast<TFunctionPointer*>( &MethodPointer );
            this->InstancePointer = std::forward<TInstancePointer>( InInstancePointer );          
        }
#endif
        //copy ctor
        template<typename = std::enable_if_t<true == std::is_copy_constructible_v<TInstancePointer>>>
        DelegateBase( const DelegateBase& Other )
        {
            this->Pointer         = Other.Pointer;
            this->InstancePointer = Other.InstancePointer;
        }
        //move ctor
        template<typename = std::enable_if_t<true == std::is_move_constructible_v<TInstancePointer>>>
        DelegateBase( DelegateBase&& Other ) noexcept
        {
            this->Pointer         = Other.Pointer;
            this->InstancePointer = std::move( Other.InstancePointer );

            Other.Pointer = nullptr;
        }
        //copy assignment operator
        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        DelegateBase& operator=( const DelegateBase& Other )
        {
            if( this == &Other ) ASD_UNLIKELY
            {
                return *this;
            }

            this->Pointer         = Other.Pointer;
            this->InstancePointer = Other.InstancePointer;

            return *this;
        }
        //move assignment operator
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        DelegateBase& operator=( DelegateBase&& Other ) noexcept
        {
            if( this == &Other ) ASD_UNLIKELY
            {
                return *this;
            }

            this->Pointer         = Other.Pointer;
            this->InstancePointer = std::move( Other.InstancePointer );

            Other.Pointer = nullptr;

            return *this;
        }

        ASD_FORCEINLINE explicit operator bool() const
        {
	        return false == IsNull(  );
        }

        ASD_NODISCARD ASD_FORCEINLINE bool IsNull() const
        {
	        return this->Pointer == nullptr || this->InstancePointer == nullptr;
        }
        
        ASD_NODISCARD ASD_FORCEINLINE bool HasMethod() const
        {
	        return this->Pointer != nullptr;
        }
        
        ASD_NODISCARD ASD_FORCEINLINE bool HasInstance() const
        {
	        return false == ( this->InstancePointer == nullptr );
        }
        
        ASD_NODISCARD ASD_FORCEINLINE TMethodPointer GetMethod() const
        {
            union TMethodPtrBuilder
            {
                TFunctionPointer FnPtr;
                TMethodPointer   MethodPtr;
            };

            TMethodPtrBuilder Builder;
            memset( &Builder, 0, sizeof( Builder ) );

            Builder.FnPtr = this->Pointer;

	        return Builder.MethodPtr;;
        }
        
        ASD_NODISCARD ASD_FORCEINLINE TClass* GetInstance() const
        {
            if constexpr( std::is_class_v<TInstancePointer> )
            {
	           return this->InstancePointer.get();
            }
            else
            {
	           return this->InstancePointer;
            }
        }
        
        template<typename = std::enable_if_t<true == std::is_copy_constructible_v<TInstancePointer>>>
        ASD_NODISCARD ASD_FORCEINLINE TInstancePointer GetInstancePtr() const
        {
	        return this->InstancePointer;
        }

        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        ASD_FORCEINLINE void SetInstance( const TInstancePointer& InInstancePointer )
        {
            this->InstancePointer = InInstancePointer;
        }
        
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        ASD_FORCEINLINE void SetInstance( TInstancePointer&& InInstancePointer )
        {
            this->InstancePointer = std::forward<TInstancePointer>( InInstancePointer );
        }

        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        ASD_NODISCARD TInstancePointer ResetInstance( const TInstancePointer& InInstancePtr )
        {
            TInstancePointer Temp = this->InstancePointer;
            this->InstancePointer = InInstancePtr;

            return Temp;
        }
        
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        ASD_NODISCARD TInstancePointer ResetInstance( TInstancePointer&& InInstancePtr )
        {
            TInstancePointer Temp = std::move( this->InstancePointer );
            this->InstancePointer = std::forward<TInstancePointer> ( InInstancePtr );

            return Temp;
        }

        ASD_NODISCARD TInstancePointer ReleaseInstance()
        {
            TInstancePointer Temp = std::move ( this->InstancePointer );
            this->InstancePointer = nullptr;

            return Temp;
        }

        ASD_FORCEINLINE void SetMethod( TMethodPointer InMethodPointer )
        {
            this->Pointer = *reinterpret_cast<TFunctionPointer*>( &InMethodPointer );
        }
        
        template<typename = std::enable_if_t<true == std::is_move_assignable_v<TInstancePointer>>>
        ASD_FORCEINLINE void SetMethodAndInstance( TMethodPointer InMethodPointer, TInstancePointer&& InInstancePointer )
        {
            SetMethod( InMethodPointer );
            SetInstance( std::forward<TInstancePointer>( InInstancePointer ) );
        }

        template<typename = std::enable_if_t<true == std::is_copy_assignable_v<TInstancePointer>>>
        ASD_FORCEINLINE void SetMethodAndInstance( TMethodPointer InMethodPointer, const TInstancePointer& InInstancePointer )
        {
            SetMethod( InMethodPointer );
            SetInstance( InInstancePointer );
        }
    };
}

namespace ASD
{
    template<typename T>
    struct TRawPointerWrapper
    {
        TRawPointerWrapper() : Pointer{ nullptr }{}
        TRawPointerWrapper( T* Pointer ) : Pointer{ Pointer }{}

        TRawPointerWrapper( const TRawPointerWrapper& Other ) : Pointer{ Other.Pointer } {}
        TRawPointerWrapper& operator=( const TRawPointerWrapper& Other ) 
        {
            Pointer = Other.Pointer;
            return *this;
        }

        TRawPointerWrapper( TRawPointerWrapper&& Other ) noexcept : Pointer{ Other.Pointer } 
        {
            Other.Pointer = nullptr;
        }

        TRawPointerWrapper& operator=( TRawPointerWrapper&& Other ) noexcept
        {
            if( this == &Other ) ASD_UNLIKELY
            {
                return *this;
            }

            Pointer = Other.Pointer;
            Other.Pointer = nullptr;

            return *this;
        }

        ASD_FORCEINLINE T* get() const 
        {
            return Pointer;
        }
        
        ASD_FORCEINLINE bool operator==( const TRawPointerWrapper& Other ) const 
        {
            return Pointer == Other.Pointer;
        }

        ASD_FORCEINLINE bool operator==( T const * Other ) const 
        {
            return Pointer == Other;
        }

        ASD_FORCEINLINE bool operator!=( T const * Other ) const 
        {
            return Pointer != Other;
        }

        T* Pointer;
    };

    template<typename TFnPtrSignature>
    using RawDelegate = DelegateBase<TRawPointerWrapper, TFnPtrSignature>;
    
    template<typename TFnPtrSignature>
    using UniqueDelegate = DelegateBase<std::unique_ptr, TFnPtrSignature>;
    
    template<typename TFnPtrSignature>
    using SharedDelegate = DelegateBase<std::shared_ptr, TFnPtrSignature>;
    
    template<typename TFnPtrSignature>
    using Delegate = RawDelegate<TFnPtrSignature>;
}

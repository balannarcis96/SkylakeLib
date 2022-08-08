//!
//! \file RStatus.h
//! 
//! \brief Result Status abstraction for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

//! Base type of the RStatus
using RStatusType = int32_t;
using RStatusNumericType = RStatusType;    

enum class RStatus : RStatusNumericType 
{
    Success                             = 0
    , Fail                              = 1
    , Timeout                           = 2
    , Aborted                           = 3
    , InvalidParamters                  = 4
    , OperationOverflows                = 5
    , AllocationFailed                  = 6
    , InvalidPoistion                   = 7
    , InvalidOffset                     = 8
    , SystemTerminated                  = 9
    , SystemFailure                     = 10
    , SuccessAsyncIORequestCancelled    = 13

    , MAX
};

#define RSTATUS_TO_BOOL( Status ) ( ( Status ) == SKL::RSuccess )
#define RSTATUS_TO_NUMERIC( Status ) ( ( RStatusNumericType )( Status ) )
#define RSTATUS_FROM_NUMERIC( Status ) ( ( RStatus )( Status ) )
#define RSTATUS_FROM_BOOL( bValue ) RSTATUS_FROM_NUMERIC( ( RSTATUS_TO_NUMERIC( SKL::RSuccess ) + static_cast< int >( !( bValue ) ) ) )

//! value to start extending the result status values
constexpr RStatusNumericType RStatusExtensionStart = RSTATUS_TO_NUMERIC( RStatus::MAX );

//! value representing the maximum value of the Skylake Lib result status value
constexpr RStatusNumericType RStatusSkylakeLibMax  = RSTATUS_TO_NUMERIC( RStatus::MAX );
   
//! global accessible compile time constants 
constexpr RStatus RSuccess                          { RStatus::Success };
constexpr RStatus RFail                             { RStatus::Fail };
constexpr RStatus RTimeout                          { RStatus::Timeout };
constexpr RStatus RAborted                          { RStatus::Aborted };
constexpr RStatus RInvalidParamters                 { RStatus::InvalidParamters };
constexpr RStatus ROperationOverflows               { RStatus::OperationOverflows };
constexpr RStatus RInvalidPoistion                  { RStatus::InvalidPoistion };
constexpr RStatus RInvalidOffset                    { RStatus::InvalidOffset };
constexpr RStatus RSystemTerminated                 { RStatus::SystemTerminated };
constexpr RStatus RSystemFailure                    { RStatus::SystemFailure };
constexpr RStatus RSuccessAsyncIORequestCancelled   { RStatus::SuccessAsyncIORequestCancelled };

SKL_FORCEINLINE bool operator!( const RStatus &Status ) noexcept
{
    return Status != RSuccess;
}

SKL_FORCEINLINE bool RStatusToBool( const RStatus Status ) noexcept
{
    return Status == RSuccess;
}

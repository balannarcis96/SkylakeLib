//!
//! \file RStatus.h
//! 
//! \brief Result Status abstraction for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

namespace SKL
{
    //! Base type of the RStatus
    using RStatusType = int32_t;
    using RStatusNumericType = RStatusType;    
    using RStatus = RStatusNumericType;    

    enum ERStatus : RStatusNumericType 
    {
        RStatus_Success = 0
        , RStatus_Fail                             
        , RStatus_Timeout                          
        , RStatus_Aborted                          
        , RStatus_AlreadyPerformed
        , RStatus_InvalidParamters                 
        , RStatus_OperationOverflows               
        , RStatus_AllocationFailed                 
        , RStatus_InvalidPoistion                  
        , RStatus_InvalidOffset                    
        , RStatus_SystemTerminated                 
        , RStatus_SystemFailure                    
        , RStatus_SuccessAsyncIORequestCancelled   
        , RStatus_ExecutedSync
        , RStatus_NotSupported
        , RStatus_ServerInstanceFinalized
        , RStatus_Pending

        , RStatus_MAX
    };

    #define RSTATUS_TO_BOOL( Status ) ( ( Status ) == SKL::RSuccess )
    #define RSTATUS_TO_NUMERIC( Status ) ( ( RStatusNumericType )( Status ) )
    #define RSTATUS_FROM_NUMERIC( Status ) ( ( RStatus )( Status ) )
    #define RSTATUS_FROM_BOOL( bValue ) RSTATUS_FROM_NUMERIC( ( RSTATUS_TO_NUMERIC( SKL::RSuccess ) + static_cast<RStatusNumericType>( !( bValue ) ) ) )

    //! value to start extending the result status values
    constexpr RStatusNumericType RStatusExtensionStart = RSTATUS_TO_NUMERIC( RStatus_MAX );

    //! value representing the maximum value of the Skylake Lib result status value
    constexpr RStatusNumericType RStatusSkylakeLibMax  = RSTATUS_TO_NUMERIC( RStatus_MAX );
   
    //! global accessible compile time constants 
    constexpr RStatus RSuccess                        { RStatus_Success };
    constexpr RStatus RFail                           { RStatus_Fail };
    constexpr RStatus RTimeout                        { RStatus_Timeout };
    constexpr RStatus RAborted                        { RStatus_Aborted };
    constexpr RStatus RAlreadyPerformed               { RStatus_AlreadyPerformed };
    constexpr RStatus RInvalidParamters               { RStatus_InvalidParamters };
    constexpr RStatus ROperationOverflows             { RStatus_OperationOverflows };
    constexpr RStatus RAllocationFailed               { RStatus_AllocationFailed };
    constexpr RStatus RInvalidPoistion                { RStatus_InvalidPoistion };
    constexpr RStatus RInvalidOffset                  { RStatus_InvalidOffset };
    constexpr RStatus RSystemTerminated               { RStatus_SystemTerminated };
    constexpr RStatus RExecutedSync                   { RStatus_ExecutedSync };
    constexpr RStatus RSystemFailure                  { RStatus_SystemFailure };
    constexpr RStatus RSuccessAsyncIORequestCancelled { RStatus_SuccessAsyncIORequestCancelled };
    constexpr RStatus RNotSupported                   { RStatus_NotSupported };
    constexpr RStatus RServerInstanceFinalized        { RStatus_ServerInstanceFinalized };
    constexpr RStatus RPending                        { RStatus_Pending };
}

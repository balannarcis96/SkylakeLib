//!
//! \file ServerInstanceConfig.h
//! 
//! \brief Server instance configuration abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    namespace ServerInstanceConfig
    {
        //! Workers group configuration
        struct WorkerGroupConfig
        {
            WorkerGroupConfig() noexcept = default;
            WorkerGroupConfig( const WorkerGroupTag& GroupTag ) noexcept : Tag{ GroupTag } { ( void )Tag.Validate(); }

            //! Set the group tag [mandatory]
            void SetTag( const WorkerGroupTag& GroupTag ) noexcept 
            {
                Tag = GroupTag;
                ( void )Tag.Validate();
            }
            
            //! Is this config valid
            bool IsValid() const noexcept { return true == Tag.IsValid(); }

            //! Set functor to be called each time a worker in the group ticks [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
            template<typename TFunctor>
            void SetWorkerTickHandler( TFunctor&& InOnWorkerTick ) noexcept 
            {
                OnWorkerTick += std::forward<TFunctor>( InOnWorkerTick );
            }
        
            //! Set functor to be called each time a worker in the group starts [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
            template<typename TFunctor>
            void SetWorkerStartHandler( TFunctor&& InOnWorkerStart ) noexcept 
            {
                OnWorkerStart += std::forward<TFunctor>( InOnWorkerStart );
            }
        
            //! Set functor to be called each time a worker in the group stops [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
            template<typename TFunctor>
            void SetWorkerStopHandler( TFunctor&& InOnWorkerStop ) noexcept 
            {
                OnWorkerStop += std::forward<TFunctor>( InOnWorkerStop );
            }
        
            //! Add new tcp async acceptor for this worker group
            bool AddTCPAsyncAcceptor( const TCPAcceptorConfig& Config ) noexcept 
            {
                if( false == Tag.bSupportesTCPAsyncAcceptors )
                {
                    SKL_ERR_FMT( "ApplicationWorkerGroupConfig::AddTCPAsyncAcceptor() Async TCP acceptors are not supported for this workers group [bSupportesTCPAsyncAcceptors == false]!" );
                    return false;
                }

                TCPAcceptorConfigs.push_back( Config );
                return true;
            }

            //! DO NOT CALL
            const WorkerGroup::WorkerTickTask& GetTaskToDispatch() const noexcept 
            {
                return OnWorkerTick;
            }

            const wchar_t* GetName() const noexcept
            {
                return nullptr == Tag.Name ? L"Unnamed [ApplicationWorkerGroupConfig]" : Tag.Name;
            }

            bool Validate() const noexcept
            {
                if( false == Tag.Validate() )
                {
                    return false;
                }       

                return true;
            }

        private:
            WorkerGroupTag                  Tag               {}; //!< Group tag
            WorkerGroup::WorkerTickTask     OnWorkerTick      {}; //!< Task to be executed each time a worker in the group ticks
            WorkerGroup::WorkerTask         OnWorkerStart     {}; //!< Task to be executed each time a worker in the group start
            WorkerGroup::WorkerTask         OnWorkerStop      {}; //!< Task to be executed each time a worker in the group stops
            std::vector<TCPAcceptorConfig>  TCPAcceptorConfigs{}; //!< List of all tcp async acceptors to create, to be handled by the workers in the group

            friend class SKL::ServerInstance;
        }; 
            
        struct ServerInstanceConfig
        {
            ServerInstanceConfig() noexcept = default;
            ServerInstanceConfig( const wchar_t* Name ) noexcept : Name { Name } {}

            //! Add new worker group config 
            SKL_FORCEINLINE void AddNewGroup( WorkerGroupConfig&& GroupConfig ) noexcept 
            {
                WorkerGroups.emplace_back( std::forward<WorkerGroupConfig>( GroupConfig ) );
            }

            //! Is this config valid
            bool IsValid() const noexcept 
            { 
                if( nullptr == Name )
                {
                    SKL_ERR( "ApplicationWorkersConfig No name supplied!" );
                    return false;
                }

                if( true == WorkerGroups.empty() )
                {   
                    SKL_ERR_FMT( "ApplicationWorkersConfig[%ws] No worker groups configured!", Name );
                    return false;
                }

                for( const auto& WorkerGroupConfigItem: WorkerGroups )
                {
                    ( void )WorkerGroupConfigItem.Validate();
                    if( false == WorkerGroupConfigItem.IsValid() )
                    {
                        SKL_ERR_FMT( "ApplicationWorkersConfig[%ws] Worker group %ws is not valid!", Name, WorkerGroupConfigItem.GetName() );
                        return false;
                    }
                }

                return true;
            }

            SKL_FORCEINLINE void SetWillCaptureCallingThread( bool bWillCaptureCallingThread ) noexcept 
            {
                this->bWillCaptureCallingThread = bWillCaptureCallingThread;
            }

        private:
            const wchar_t*                 Name                      { nullptr }; //!< Workers manager instance name
            std::vector<WorkerGroupConfig> WorkerGroups              {};          //!< Config for all needed worker groups
            bool                           bWillCaptureCallingThread { true };    //!< Will this server instance use the calling thread as a worker on start

            friend class SKL::ServerInstance;
        };  
    }
}
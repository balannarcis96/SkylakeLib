//!
//! \file KPI.h
//! 
//! \brief SkylakeLib KPI - Key Performance indicator
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct KPITimeValue
    {
        KPITimeValue() noexcept
        {
            Update();
            Begin();
        }

        SKL_FORCEINLINE void Update() noexcept
        {
            int64_t FQ;
            LoadPerformanceFrequency( FQ );
            Frequency = static_cast<double>( FQ );
        }

        SKL_FORCEINLINE void Begin() noexcept
        {
            int64_t PC;
            LoadPerformanceCounter( PC );
            Start = static_cast<double>( PC );
        }
        
        SKL_FORCEINLINE void BeginUpdated() noexcept
        {
            Update();

            int64_t PC;
            LoadPerformanceCounter( PC );
            Start = static_cast<double>( PC );
        }

        SKL_FORCEINLINE SKL_NODISCARD double GetElapsedSeconds() const noexcept
        {
            int64_t Now;
            LoadPerformanceCounter( Now );
            return static_cast<double>( ( static_cast<double>( Now ) - Start ) / Frequency );
        }

    private:
        double Frequency;
        double Start;
    };

    template<bool bAtomic>
    struct KPIValueAveragePoint
    {
        using TIndex = std::conditional_t<bAtomic, std::relaxed_value<size_t>, size_t>;
        using TValue = std::conditional_t<bAtomic, std::relaxed_value<double>, double>;

        SKL_FORCEINLINE void SetValue( double Value ) noexcept
        {
            const size_t TargetIndex{ Index++ & CKPIPointsToAverageFrom };
            ValuePoints[TargetIndex] = Value;
        }
        SKL_NODISCARD double GetValue() const noexcept
        {
            double OutValue = 0.0;
        
            int32_t i = 0;
            do 
            {
                OutValue += ValuePoints[i];             
            } while ( ++i < CKPIPointsToAverageFrom );

            return OutValue / CKPIPointsToAverageFrom; 
        }

    private:
        TIndex Index{ 0U };
        TValue ValuePoints[CKPIPointsToAverageFrom]{};
    };

    struct KPI_WorkerEnqueueCounters
    {
        void Reset() noexcept
        {
            TasksQueue_EnqueuedCount                       = 0U;
            DelayedTasksQueue_EnqueuedCount                = 0U;
            AODSharedObjectDelayedTasksQueue_EnqueuedCount = 0U;
            AODStaticObjectDelayedTasksQueue_EnqueuedCount = 0U;
            AODCustomObjectDelayedTasksQueue_EnqueuedCount = 0U;
        }

        uint64_t TasksQueue_EnqueuedCount{ 0U };
        uint64_t DelayedTasksQueue_EnqueuedCount{ 0U };
        uint64_t AODSharedObjectDelayedTasksQueue_EnqueuedCount{ 0U };
        uint64_t AODStaticObjectDelayedTasksQueue_EnqueuedCount{ 0U };
        uint64_t AODCustomObjectDelayedTasksQueue_EnqueuedCount{ 0U };
    };

    struct KPI_WorkerDequeueCounters
    {
        void Reset() noexcept
        {
            TasksQueue_DequeuedCount                       = 0U;
            DelayedTasksQueue_DequeuedCount                = 0U;
            AODSharedObjectDelayedTasksQueue_DequeuedCount = 0U;
            AODStaticObjectDelayedTasksQueue_DequeuedCount = 0U;
            AODCustomObjectDelayedTasksQueue_DequeuedCount = 0U;
        }

        uint64_t TasksQueue_DequeuedCount{ 0U };
        uint64_t DelayedTasksQueue_DequeuedCount{ 0U };
        uint64_t AODSharedObjectDelayedTasksQueue_DequeuedCount{ 0U };
        uint64_t AODStaticObjectDelayedTasksQueue_DequeuedCount{ 0U };
        uint64_t AODCustomObjectDelayedTasksQueue_DequeuedCount{ 0U };
    };

    class KPIContext: public ITLSSingleton<KPIContext>
    {
    public:
        using TKPIValueAveragePoint = KPIValueAveragePoint<false>;

        RStatus Initialize() noexcept override;
        const char *GetName() const noexcept override { return "[KPIContext]"; }

        // Alloc counters
        template<EKPIValuePoints KPIValuePoint>
        SKL_FORCEINLINE SKL_NODISCARD static uint64_t GetAllocCount() noexcept { return GetInstance()->MemoryAllocationsCounters[static_cast<int32_t>( KPIValuePoint )]; }
        template<EKPIValuePoints KPIValuePoint>
        static void IncrementAllocCount() noexcept { ( void )++GetInstance()->MemoryAllocationsCounters[static_cast<int32_t>( KPIValuePoint )]; }
        
        // Averageable values
        template<EKPIValuePoints KPIValuePoint>
        SKL_FORCEINLINE SKL_NODISCARD static double Static_GetAverageKPIValue() noexcept 
        { 
            return GetInstance()->GetAverageKPIValue<KPIValuePoint>();
        }
        SKL_FORCEINLINE SKL_NODISCARD static double Static_GetAverageKPIValue( EKPIValuePoints KPIValuePoint ) noexcept 
        { 
            return GetInstance()->GetAverageKPIValue( KPIValuePoint );
        }

        template<EKPIValuePoints KPIValuePoint>
        SKL_FORCEINLINE static void Static_SetAverageKPIValue( double Value ) noexcept 
        { 
            GetInstance()->SetAverageKPIValue<KPIValuePoint>( Value );
        }
        SKL_FORCEINLINE static void Static_SetAverageKPIValue( EKPIValuePoints KPIValuePoint, double Value ) noexcept 
        { 
            GetInstance()->SetAverageKPIValue( KPIValuePoint, Value );
        }
   
        template<EKPIValuePoints KPIValuePoint>
        SKL_FORCEINLINE SKL_NODISCARD double GetAverageKPIValue() const noexcept 
        { 
            return AverageValuePoints[static_cast<int32_t>( KPIValuePoint )].GetValue();
        }
        SKL_FORCEINLINE SKL_NODISCARD double GetAverageKPIValue( EKPIValuePoints KPIValuePoint ) const noexcept 
        { 
            return AverageValuePoints[static_cast<int32_t>( KPIValuePoint )].GetValue();
        }

        template<EKPIValuePoints KPIValuePoint>
        SKL_FORCEINLINE void SetAverageKPIValue( double Value ) noexcept 
        { 
            AverageValuePoints[static_cast<int32_t>( KPIValuePoint )].SetValue( Value );
        }
        SKL_FORCEINLINE void SetAverageKPIValue( EKPIValuePoints KPIValuePoint, double Value ) noexcept 
        { 
            AverageValuePoints[static_cast<int32_t>( KPIValuePoint )].SetValue( Value );
        }
   
        // Enqueue counters
        SKL_FORCEINLINE SKL_NODISCARD static KPI_WorkerEnqueueCounters& GetWorkerEnqueueCounter( int32_t TargetWorkerIndex ) noexcept
        {
            return GetInstance()->WorkerEnqueueCounters[TargetWorkerIndex];
        }

        #if defined(SKL_KPI_QUEUE_SIZES)
        static constexpr auto CMaxEnqueueCouneters = 256;

        SKL_FORCEINLINE static void Increment_DelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerEnqueueCounters[WorkerIndex].DelayedTasksQueue_EnqueuedCount;
        }
        SKL_FORCEINLINE static void Increment_TasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerEnqueueCounters[WorkerIndex].TasksQueue_EnqueuedCount;
        }
        SKL_FORCEINLINE static void Increment_AODSharedObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerEnqueueCounters[WorkerIndex].AODSharedObjectDelayedTasksQueue_EnqueuedCount;
        }
        SKL_FORCEINLINE static void Increment_AODStaticObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerEnqueueCounters[WorkerIndex].AODStaticObjectDelayedTasksQueue_EnqueuedCount;
        }
        SKL_FORCEINLINE static void Increment_AODCustomObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerEnqueueCounters[WorkerIndex].AODCustomObjectDelayedTasksQueue_EnqueuedCount;
        }

        SKL_FORCEINLINE static void Decrement_DelayedTasksQueueSize( uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerDequeueCounters.DelayedTasksQueue_DequeuedCount += Count;
        }
        SKL_FORCEINLINE static void Decrement_TasksQueueSize( uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerDequeueCounters.TasksQueue_DequeuedCount += Count;
        }
        SKL_FORCEINLINE static void Decrement_AODSharedObjectDelayedTasksQueueSize( uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerDequeueCounters.AODSharedObjectDelayedTasksQueue_DequeuedCount += Count;
        }
        SKL_FORCEINLINE static void Decrement_AODStaticObjectDelayedTasksQueueSize( uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerDequeueCounters.AODStaticObjectDelayedTasksQueue_DequeuedCount += Count;
        }
        SKL_FORCEINLINE static void Decrement_AODCustomObjectDelayedTasksQueueSize( uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerDequeueCounters.AODCustomObjectDelayedTasksQueue_DequeuedCount += Count;
        }

        static void ClearEnqueueAndDequeueCounters() noexcept
        {
            auto* Instance = GetInstance();

            for( auto i = 0; i < CMaxEnqueueCouneters; ++i )
            {
                Instance->WorkerEnqueueCounters[i].Reset();
            }

            Instance->WorkerDequeueCounters.Reset();
        }
        #endif
    private:
        KPITimeValue               TimeValue[32]{};
        TKPIValueAveragePoint      AverageValuePoints[static_cast<int32_t>( EKPIValuePoints::Max )]{};
        uint64_t                   MemoryAllocationsCounters[static_cast<int32_t>( EKPIValuePoints::Max )];

        #if defined(SKL_KPI_QUEUE_SIZES)
        KPI_WorkerEnqueueCounters WorkerEnqueueCounters[CMaxEnqueueCouneters]{};
        KPI_WorkerDequeueCounters WorkerDequeueCounters{};
        #endif
    };
}
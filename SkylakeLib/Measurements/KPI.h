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

    struct KPI_WorkerSummableCounters
    {
        uint64_t TasksQueueSize{ 0U };
        uint64_t DelayedTasksQueueSize{ 0U };
        uint64_t AODSharedObjectDelayedTasksQueueSize{ 0U };
        uint64_t AODStaticObjectDelayedTasksQueueSize{ 0U };
        uint64_t AODCustomObjectDelayedTasksQueueSize{ 0U };
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
   
        // Summable counters
        SKL_FORCEINLINE SKL_NODISCARD KPI_WorkerSummableCounters& GetWorkerSummableCounter( int32_t TargetWorkerIndex ) noexcept
        {
            return WorkerSummableCounters[TargetWorkerIndex];
        }

        #if defined(SKL_KPI_QUEUE_SIZES)
        SKL_FORCEINLINE static void Increment_DelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerSummableCounters[WorkerIndex].DelayedTasksQueueSize;
        }
        SKL_FORCEINLINE static void Increment_TasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerSummableCounters[WorkerIndex].TasksQueueSize;
        }
        SKL_FORCEINLINE static void Increment_AODSharedObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerSummableCounters[WorkerIndex].AODSharedObjectDelayedTasksQueueSize;
        }
        SKL_FORCEINLINE static void Increment_AODStaticObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerSummableCounters[WorkerIndex].AODStaticObjectDelayedTasksQueueSize;
        }
        SKL_FORCEINLINE static void Increment_AODCustomObjectDelayedTasksQueueSize( int32_t WorkerIndex ) noexcept
        {
            ( void )++GetInstance()->WorkerSummableCounters[WorkerIndex].AODCustomObjectDelayedTasksQueueSize;
        }

        SKL_FORCEINLINE static void Decrement_DelayedTasksQueueSize( int32_t WorkerIndex, uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerSummableCounters[WorkerIndex].DelayedTasksQueueSize -= Count;
        }
        SKL_FORCEINLINE static void Decrement_TasksQueueSize( int32_t WorkerIndex, uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerSummableCounters[WorkerIndex].TasksQueueSize -= Count;
        }
        SKL_FORCEINLINE static void Decrement_AODSharedObjectDelayedTasksQueueSize( int32_t WorkerIndex, uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerSummableCounters[WorkerIndex].AODSharedObjectDelayedTasksQueueSize -= Count;
        }
        SKL_FORCEINLINE static void Decrement_AODStaticObjectDelayedTasksQueueSize( int32_t WorkerIndex, uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerSummableCounters[WorkerIndex].AODStaticObjectDelayedTasksQueueSize -= Count;
        }
        SKL_FORCEINLINE static void Decrement_AODCustomObjectDelayedTasksQueueSize( int32_t WorkerIndex, uint64_t Count = 1 ) noexcept
        {
            GetInstance()->WorkerSummableCounters[WorkerIndex].AODCustomObjectDelayedTasksQueueSize -= Count;
        }
        #endif
    private:
        KPITimeValue               TimeValue[32]{};
        TKPIValueAveragePoint      AverageValuePoints[static_cast<int32_t>( EKPIValuePoints::Max )]{};
        uint64_t                   MemoryAllocationsCounters[static_cast<int32_t>( EKPIValuePoints::Max )];

        #if defined(SKL_KPI_QUEUE_SIZES)
        KPI_WorkerSummableCounters WorkerSummableCounters[256];
        #endif
    };
}
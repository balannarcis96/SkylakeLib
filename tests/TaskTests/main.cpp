#include <gtest/gtest.h>

#include <SkylakeLib.h>

template<typename TLambda>
SKL::ITask* CreateTask( TLambda&& lambda )
{
    constexpr size_t CTaskNeededSize = sizeof( TLambda );
    using TaskType = SKL::Task<CTaskNeededSize>;
    
    TaskType* NewTask = new TaskType();

    NewTask->SetDispatch( std::forward<TLambda>( lambda ) );

    return NewTask;
}

namespace Task_Tests_Suite
{
    struct MyType
    {
        int a { 0 };
    };

    TEST( Task_Tests_Suite, Task_Construct_Destruct )
    {
        auto SPtr = std::make_shared<MyType>();
        ASSERT_TRUE( SPtr.use_count() == 1 );
        
        {
            SKL::Task<24> NewTask;
            int aaa = 5;

            auto NewLambda = [ SPtr, aaa ]( SKL::ITask* /*Self*/ ) noexcept -> void
            {
                printf( "This is from the task! aaa:%d\n", aaa );
            };

            NewTask += std::move( NewLambda );

            ASSERT_TRUE( SPtr.use_count() == 2 );

            NewTask.Dispatch();
            NewTask.Dispatch();
            NewTask.Dispatch();

            SKL::ITask* TaskBase = &NewTask;

            TaskBase->Dispatch();
            TaskBase->Dispatch();
            TaskBase->Dispatch();

            ASSERT_TRUE( SPtr.use_count() == 2 );

            TaskBase->Clear();

            ASSERT_TRUE( SPtr.use_count() == 1 );
        }
       
        ASSERT_TRUE( SPtr.use_count() == 1 );
    }

    TEST( Task_Tests_Suite, Task_Construct_Destruct_Dynamic )
    {
        auto SPtr = std::make_shared<MyType>();
        ASSERT_TRUE( SPtr.use_count() == 1 );
        
        //Assumed task type
        using ProtoTaskType = SKL::Task<sizeof( SPtr )>;

        SKL::ITask* NewTask = CreateTask([ SPtr ]( SKL::ITask* /*Self*/ ) noexcept 
        {
            printf( "FromTask value:%d\n", SPtr->a );
        }); 
        
        ASSERT_TRUE( SPtr.use_count() == 2 );
                
        NewTask->Dispatch();
        NewTask->Dispatch();
        NewTask->Dispatch();
    
        ASSERT_TRUE( SPtr.use_count() == 2 );
        
        NewTask->Clear();

        ASSERT_TRUE( SPtr.use_count() == 1 );

        //ASSERT_TRUE( NewTask->GetTotalSize() == 40 );

        // We cast back here for correct size deletion.
        // Inside the library all of the tasks are allocated through the memory manager
        // which tracks the allocated block size indifferent of the allocated type so the 
        // block size is known at deallocation indifferent of the pointer type.
        ProtoTaskType* SupposedTask{ reinterpret_cast<ProtoTaskType*>( NewTask ) };
        delete SupposedTask;
        NewTask = nullptr;
    }

    TEST( Task_Tests_Suite, AsyncIoTask_API_Test )
    {
        using BufferType = SKL::AsyncIOBuffer<128, 32>;
        uint32_t nbt { 400 };

        auto NewTask      { SKL::MakeShared<BufferType>() };
        auto Interface    { NewTask->GetInterface() };

        Interface.Buffer[2] = 0xF1;

        auto SPtr = std::make_shared<MyType>();
        ASSERT_TRUE( SPtr.use_count() == 1 );
        
        NewTask->SetCompletionHandler( [&nbt, SelfPtr = NewTask.get(), SPtr ]( SKL::IAsyncIOTask& Self, uint32_t NumberOfBytesTransferred ) noexcept -> void
        {
            ASSERT_TRUE( NumberOfBytesTransferred == nbt );
            ASSERT_TRUE( SelfPtr == &Self );

            auto Interface { Self.GetInterface() };
            ASSERT_TRUE( 0xF1 == Interface.Buffer[2] );
        } );
        ASSERT_TRUE( SPtr.use_count() == 2 );
        NewTask->Dispatch( nbt );
        ASSERT_TRUE( SPtr.use_count() == 2 );
        NewTask->Clear();
        ASSERT_TRUE( SPtr.use_count() == 1 );
    }

    TEST( Task_Tests_Suite, AsyncIoTask_Stream_API )
    {
        using BufferType = SKL::AsyncIOBuffer<1024, 32>;
        auto NewTask { SKL::MakeShared<BufferType>() };
        ASSERT_TRUE( nullptr != NewTask.get() );

        auto Interface{ NewTask->GetInterface() };
        auto Stream   { NewTask->GetStream() };

        ASSERT_TRUE( Interface.Buffer == Stream->GetBuffer() );
        ASSERT_TRUE( Interface.Length == Stream->GetBufferSize() );
        ASSERT_TRUE( Interface.Length == Stream->GetRemainingSize() );
        ASSERT_TRUE( 0 == Stream->GetPosition() );
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetPosition() );

        Stream->ForwardToEnd();
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetPosition() );
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetBufferSize() );
        ASSERT_TRUE( 0 == Stream->GetRemainingSize() );
        ASSERT_TRUE( true == Stream->IsEOS() );

        Stream->ForwardToEnd( Stream->GetBufferSize() );
        ASSERT_TRUE( Interface.Length == Stream->GetRemainingSize() );
        ASSERT_TRUE( 0 == Stream->GetPosition() );
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetPosition() );
        ASSERT_TRUE( false == Stream->IsEOS() );

        Stream->Reset();
        ASSERT_TRUE( Interface.Length == Stream->GetRemainingSize() );
        ASSERT_TRUE( 0 == Stream->GetPosition() );
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetPosition() );

        Stream->WriteT( uint32_t( 5 ) );
        ASSERT_TRUE( sizeof( uint32_t ) == Stream->GetPosition() );
        ASSERT_TRUE( sizeof( uint32_t ) == NewTask->GetPosition() );

        Stream->Reset();
        ASSERT_TRUE( Interface.Length == Stream->GetRemainingSize() );
        ASSERT_TRUE( 0 == Stream->GetPosition() );
        ASSERT_TRUE( NewTask->GetPosition() == Stream->GetPosition() );

        ASSERT_TRUE( 5 == Stream->ReadT<uint32_t>() );
        ASSERT_TRUE( sizeof( uint32_t ) == Stream->GetPosition() );
        ASSERT_TRUE( sizeof( uint32_t ) == NewTask->GetPosition() );
    }

    TEST( Task_Tests_Suite, AsyncIoTask_Transaction_API )
    {
        using BufferType = SKL::AsyncIOBuffer<128, 32>;
        auto NewTask { SKL::MakeShared<BufferType>() };
        ASSERT_TRUE( nullptr != NewTask.get() );

        SKL_ASYNCIO_BUFFER_TRANSACTION( NewTask )
        {
            ASSERT_TRUE( 0 == Transaction.GetPosition() );
            Transaction.WriteT( uint32_t( 5 ) );
            ASSERT_TRUE( sizeof( uint32_t ) == Transaction.GetPosition() );
            Transaction.Rollback();
        }

        ASSERT_TRUE( 0 == NewTask->GetPosition() );

        SKL_ASYNCIO_BUFFER_TRANSACTION( NewTask )
        {
            ASSERT_TRUE( 0 == Transaction.GetPosition() );
            Transaction.WriteT( uint32_t( 5 ) );
            ASSERT_TRUE( sizeof( uint32_t ) == Transaction.GetPosition() );
        }

        ASSERT_TRUE( sizeof( uint32_t ) == NewTask->GetPosition() );

        {
            auto Stream{ NewTask->GetStream() };
            ASSERT_TRUE( sizeof( uint32_t ) == Stream->GetPosition() );
        }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}

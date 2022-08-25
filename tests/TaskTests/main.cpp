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

namespace TaskTests
{
    struct MyType
    {
        int a { 0 };
    };

    TEST( TaskTests, Task_Construct_Destruct )
    {
        auto SPtr = std::make_shared<MyType>();
        ASSERT_TRUE( SPtr.use_count() == 1 );
        
        {
            SKL::Task<24> NewTask;
            int aaa = 5;

            auto NewLambda = [ SPtr, aaa ]( SKL::ITask* Self ) noexcept -> void
            {
                puts("This is from the task!");
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

    TEST( TaskTests, Task_Construct_Destruct_Dynamic )
    {
        auto SPtr = std::make_shared<MyType>();
        ASSERT_TRUE( SPtr.use_count() == 1 );
        
        SKL::ITask* NewTask = CreateTask([ SPtr ]( SKL::ITask* Self ) noexcept 
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

        delete NewTask;
        NewTask = nullptr;
    }

    TEST( TaskTests, AsyncIoTask_API )
    {
        using BufferType = SKL::AsyncIOBuffer<128, 32>;
        uint32_t nbt { 400 };

        auto NewTask      { SKL::MakeShared<BufferType>() };
        auto Interface    { NewTask->GetInterface() };
        auto SpanIterface { NewTask->get_span() };

        Interface.Buffer[2] = 0xF1;
        ASSERT_TRUE( 0xF1 == SpanIterface[2] );

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
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
#include <gtest/gtest.h>

#include <SkylakeLib.h>

template<typename TLambda>
SKL::IAsyncTask* CreateTask( TLambda&& lambda )
{
    constexpr size_t CTaskNeededSize = sizeof( TLambda );
    using TaskType = SKL::AsyncTask<CTaskNeededSize>;
    
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
            SKL::AsyncTask<24> NewTask;
            int aaa = 5;

            auto NewLambda = [ SPtr, aaa ]() noexcept -> void
            {
                puts("This is from the task!");
            };

            NewTask += std::move( NewLambda );

            ASSERT_TRUE( SPtr.use_count() == 2 );

            NewTask.Dispatch();
            NewTask.Dispatch();
            NewTask.Dispatch();

            SKL::IAsyncTask* TaskBase = &NewTask;

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
        
        SKL::IAsyncTask* NewTask = CreateTask([ SPtr ]() noexcept 
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

        ASSERT_TRUE( NewTask->GetTotalSize() == 40 );

        delete NewTask;
        NewTask = nullptr;
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
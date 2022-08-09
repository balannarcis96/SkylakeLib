#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace TLSTests
{
    struct MyType { int a; };

    using MyTLSValue_i8     = SKL::TLSValue<int8_t, 551155>;
    using MyTLSValue_u8     = SKL::TLSValue<uint8_t, 551155>;
    using MyTLSValue_i16    = SKL::TLSValue<int16_t, 551155>;
    using MyTLSValue_u16    = SKL::TLSValue<uint16_t, 551155>;
    using MyTLSValue_i32    = SKL::TLSValue<int32_t, 551155>;
    using MyTLSValue_u32    = SKL::TLSValue<uint32_t, 551155>;
    using MyTLSValue_i64    = SKL::TLSValue<int64_t, 551155>;
    using MyTLSValue_u64    = SKL::TLSValue<uint64_t, 551155>;
    using MyTLSValue_single = SKL::TLSValue<float, 551155>;
    using MyTLSValue_double = SKL::TLSValue<double, 551155>;
    using MyTLSValue_string = SKL::TLSValue<std::string, 551155>;

    using MyTLSPtr = SKL::TLSValue<MyType>;

    TEST( TLSTests, Get_Set_TLS_Value )
    {
        MyTLSValue_u32::SetValue( 55 );
        const uint32_t Value = MyTLSValue_u32::GetValue();
        ASSERT_TRUE( 55 == Value );
    }

    TEST( TLSTests, Get_Set_TLS_Value_Signed_Unsigned_Limits )
    {
        MyTLSValue_i8::SetValue( std::numeric_limits<int8_t>::max() );
        MyTLSValue_u8::SetValue( std::numeric_limits<uint8_t>::max() );
        MyTLSValue_i16::SetValue( std::numeric_limits<int16_t>::max() );
        MyTLSValue_u16::SetValue( std::numeric_limits<uint16_t>::max() );
        MyTLSValue_i32::SetValue( std::numeric_limits<int32_t>::max() );
        MyTLSValue_u32::SetValue( std::numeric_limits<uint32_t>::max() );
        MyTLSValue_i64::SetValue( std::numeric_limits<int64_t>::max() );
        MyTLSValue_u64::SetValue( std::numeric_limits<uint64_t>::max() );
        MyTLSValue_single::SetValue( std::numeric_limits<float>::max() );
        MyTLSValue_double::SetValue( std::numeric_limits<double>::max() );

        ASSERT_TRUE( std::numeric_limits<int8_t>::max() == MyTLSValue_i8::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint8_t>::max() == MyTLSValue_u8::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int16_t>::max() == MyTLSValue_i16::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint16_t>::max() == MyTLSValue_u16::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int32_t>::max() == MyTLSValue_i32::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == MyTLSValue_u32::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int64_t>::max() == MyTLSValue_i64::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint64_t>::max() == MyTLSValue_u64::GetValue() );
        ASSERT_TRUE( std::numeric_limits<float>::max() == MyTLSValue_single::GetValue() );
        ASSERT_TRUE( std::numeric_limits<double>::max() == MyTLSValue_double::GetValue() );

        MyTLSValue_i8::SetValue( std::numeric_limits<int8_t>::min() );
        MyTLSValue_u8::SetValue( std::numeric_limits<uint8_t>::min() );
        MyTLSValue_i16::SetValue( std::numeric_limits<int16_t>::min() );
        MyTLSValue_u16::SetValue( std::numeric_limits<uint16_t>::min() );
        MyTLSValue_i32::SetValue( std::numeric_limits<int32_t>::min() );
        MyTLSValue_u32::SetValue( std::numeric_limits<uint32_t>::min() );
        MyTLSValue_i64::SetValue( std::numeric_limits<int64_t>::min() );
        MyTLSValue_u64::SetValue( std::numeric_limits<uint64_t>::min() );
        MyTLSValue_single::SetValue( std::numeric_limits<float>::min() );
        MyTLSValue_double::SetValue( std::numeric_limits<double>::min() );

        ASSERT_TRUE( std::numeric_limits<int8_t>::min() == MyTLSValue_i8::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint8_t>::min() == MyTLSValue_u8::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int16_t>::min() == MyTLSValue_i16::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint16_t>::min() == MyTLSValue_u16::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int32_t>::min() == MyTLSValue_i32::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint32_t>::min() == MyTLSValue_u32::GetValue() );
        ASSERT_TRUE( std::numeric_limits<int64_t>::min() == MyTLSValue_i64::GetValue() );
        ASSERT_TRUE( std::numeric_limits<uint64_t>::min() == MyTLSValue_u64::GetValue() );
        ASSERT_TRUE( std::numeric_limits<float>::min() == MyTLSValue_single::GetValue() );
        ASSERT_TRUE( std::numeric_limits<double>::min() == MyTLSValue_double::GetValue() );
    }

    TEST( TLSTests, Get_Set_TLS_Ptr )
    {
        // nullptr is the default 
        MyType* Value = MyTLSPtr::GetValuePtr();
        ASSERT_TRUE( nullptr == Value );
    
        auto NewInstance = std::make_unique<MyType>();
        ASSERT_TRUE( nullptr != NewInstance );
        NewInstance->a = 20;
    
        MyTLSPtr::SetValuePtr( NewInstance.get() );

        Value = MyTLSPtr::GetValuePtr();
        ASSERT_TRUE( NewInstance.get() == Value );
        ASSERT_TRUE( Value->a == NewInstance->a );
    }

    TEST( TLSTests, Get_Set_TLS_Ptr_Ex )
    {
        // nullptr is the default 
        std::string* Value = MyTLSValue_string::GetValuePtr();
        ASSERT_TRUE( nullptr == Value );
    
        auto NewInstance = std::make_unique<std::string>( "A1B2C3" );
        ASSERT_TRUE( nullptr != NewInstance );
        ASSERT_TRUE( *NewInstance == "A1B2C3" );
    
        MyTLSValue_string::SetValuePtr( NewInstance.get() );

        Value = MyTLSValue_string::GetValuePtr();
        ASSERT_TRUE( NewInstance.get() == Value );
        ASSERT_TRUE( *Value == "A1B2C3" );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
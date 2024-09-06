#include <gtest/gtest.h>
#include <zeus/foundation/dep_inject/managed_object.h>
#include <zeus/foundation/dep_inject/object_container.h>
#include <zeus/foundation/dep_inject/injected_object.h>
#include <zeus/foundation/dep_inject/auto_register_object.hpp>
#include <zeus/foundation/dep_inject/auto_type_register.h>
//using namespace zeus;

class BaesInterface0
{
public:
    BaesInterface0() {}
    virtual ~BaesInterface0() {}
};

class BaseInterface : public BaesInterface0
{
public:
    BaseInterface() {}
    virtual ~BaseInterface() {}
};

class MainType1
    : public BaseInterface
    , public zeus::AutoRegisteObject<MainType1, BaseInterface, BaesInterface0>
{
public:
    MainType1() : BaseInterface() {}
    ~MainType1() {}

private:
};

class MainType2
    : public BaseInterface
    , public zeus::ManagedObject
{
public:
    MainType2() : BaseInterface(), ManagedObject() {}
    ~MainType2() {}

private:
};

class InjectedType
    : public zeus::InjectedObject
    , public zeus::AutoRegisteObject<MainType1, BaseInterface, BaesInterface0>
{
public:
    InjectedType() : InjectedObject() {}
    ~InjectedType() override {}
public:
    INJECT_TYPE_OBJECT(MainType1, _mainObject)
    INJECT_TYPE_OBJECT(BaseInterface, _typeObject)
};

TEST(DepInject, Container)
{
    zeus::ObjectConstainer container;

    bool registerResult = container.RegisterType<MainType1, BaseInterface, BaesInterface0>();
    EXPECT_TRUE(registerResult);
    registerResult = container.RegisterType<MainType2, BaseInterface, BaesInterface0>("MainType2");
    EXPECT_TRUE(registerResult);
    auto autoGenerateObject1 = container.GetObjectReference<MainType1>();
    EXPECT_TRUE(autoGenerateObject1);
    auto autoGenerateObject2 = container.GetObjectReference<MainType2>("MainType2");
    EXPECT_TRUE(autoGenerateObject2);
    EXPECT_FALSE(container.GetObjectReference<MainType1>("MainType2"));
    EXPECT_FALSE(container.GetObjectReference<MainType2>("MainType1"));
    auto mainObject1 = std::make_shared<MainType1>();
    registerResult   = container.AddObject<MainType1, BaseInterface, BaesInterface0>(mainObject1, "mainObject");
    EXPECT_TRUE(registerResult);

    auto cacheObject = container.GetObjectReference<BaseInterface>("mainObject");

    EXPECT_EQ(cacheObject, mainObject1);
    EXPECT_NE(autoGenerateObject1, cacheObject);
    EXPECT_NE(autoGenerateObject2, cacheObject);
}

TEST(DepInject, AutoInject)
{
    zeus::ObjectConstainer container;

    bool registerResult = container.RegisterType<MainType1, BaseInterface, BaesInterface0>();
    EXPECT_TRUE(registerResult);

    InjectedType injectedObject;
    injectedObject.Inject(container);
    EXPECT_TRUE(injectedObject._mainObject);
    EXPECT_TRUE(injectedObject._typeObject);
    EXPECT_EQ(injectedObject._mainObject, injectedObject._typeObject);
    injectedObject.Uninject();
    EXPECT_FALSE(injectedObject._mainObject);
    EXPECT_FALSE(injectedObject._typeObject);
}

TEST(DepInject, AutoRegister)
{
    zeus::ObjectConstainer container;
    zeus::AutoTypeRegister::ExecuteTypeRegisterTask(container, false);

    InjectedType injectedObject;
    injectedObject.Inject(container);
    EXPECT_TRUE(injectedObject._mainObject);
    EXPECT_TRUE(injectedObject._typeObject);
    EXPECT_EQ(injectedObject._mainObject, injectedObject._typeObject);
    injectedObject.Uninject();
    EXPECT_FALSE(injectedObject._mainObject);
    EXPECT_FALSE(injectedObject._typeObject);
}
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <NetConfAgentMock.hpp>
#include <MobileClient.hpp>

using ::testing::_;
using ::testing::Return;
using ::testing::Assign;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::SaveArg;

class MobileClientTest : public testing::Test
{
protected:
    void SetUp() override
    {
        auto tempMock = std::make_unique<testing::StrictMock<NetConfAgentMock>>();
        _mock = tempMock.get();
        _mobileClient = std::make_unique<mobileclient::MobileClient>(std::move(tempMock));
    }

    void registerUser() {
        EXPECT_CALL(*_mock, initSysrepo());
        std::string one = "/mobile-network:core/subscribers[number='+380911111111']/state";
        std::string two;
        std::map<std::string, std::string> testMap = {
            {one, two}
        };

        std::string moduleName = "mobile-network";
        std::string value = "idle";
        EXPECT_CALL(*_mock, fetchData(testMap));
        EXPECT_CALL(*_mock, registerOperData(_, moduleName));
        EXPECT_CALL(*_mock, changeData(one, value));
        EXPECT_CALL(*_mock, subscribeForModelChanges(_, moduleName));

        EXPECT_TRUE(_mobileClient->registerClient("+380911111111"));
    }

    void callImitation() {
        registerUser();
        std::string hostXpathIncomingNumber = "/mobile-network:core/subscribers[number='+380911111111']/incomingNumber";
        std::string incomingNumber = "+380922222222";
        std::map<std::string, std::string> testMap = {
            {hostXpathIncomingNumber, incomingNumber}
        };
        EXPECT_CALL(*_mock, fetchData(_))
                .WillOnce(DoAll(SetArgReferee<0>(testMap), Return(true)));
        _mobileClient->handleModuleChange("active");
    }

    std::unique_ptr<mobileclient::MobileClient> _mobileClient;
    testing::StrictMock<NetConfAgentMock> *_mock;
};


TEST_F(MobileClientTest, shouldSucceedToCreate)
{
    mobileclient::MobileClient user;
}

TEST_F(MobileClientTest, shouldSucceedToSetName)
{
    _mobileClient->setName("Edo");
}

TEST_F(MobileClientTest, shouldFailToRegisterDueToClient)
{
    registerUser();
    _mobileClient->registerClient("+380922222222");
    _mobileClient->call("+380911111111");
    _mobileClient->getXpathState();
}

TEST_F(MobileClientTest, shouldFailToRegisterDueToSysrepo)
{
    EXPECT_CALL(*_mock, initSysrepo());
    std::string one = "/mobile-network:core/subscribers[number='+380911111111']/state";
    std::string two;
    std::map<std::string, std::string> testMap = {
        {one, two}
    };

    std::string state = "idle";
    std::map<std::string, std::string> testMapSecond = {
        {one, state}
    };

    EXPECT_CALL(*_mock, fetchData(testMap))
            .WillOnce(DoAll(SetArgReferee<0>(testMapSecond), Return(true)));
    EXPECT_FALSE(_mobileClient->registerClient("+380911111111"));
}

TEST_F(MobileClientTest, shouldSucceedToUnregister)
{
    std::string one = "/mobile-network:core/subscribers[number='']";
    EXPECT_CALL(*_mock, changeData(one, std::string()));
    EXPECT_CALL(*_mock, closeSys());
    EXPECT_TRUE(_mobileClient->unregisterClient());
}

TEST_F(MobileClientTest, shouldFailToUnregister)
{
    _mobileClient->handleModuleChange("busy");
    _mobileClient->unregisterClient();
}


TEST_F(MobileClientTest, shouldFailToCall)
{
    EXPECT_CALL(*_mock, fetchData(_))
                .Times(2);
    _mobileClient->call("+380911111111");
    _mobileClient->handleModuleChange("busy");
    _mobileClient->call("number");

    _mobileClient->handleModuleChange("idle");
    std::string one = "/mobile-network:core/subscribers[number='+380911111111']/state";
    std::string two;
    std::map<std::string, std::string> testMap = {
        {one, two}
    };

    std::string state = "busy";
    std::map<std::string, std::string> testMapSecond = {
        {one, state}
    };
    EXPECT_CALL(*_mock, fetchData(testMap))
            .WillOnce(DoAll(SetArgReferee<0>(testMapSecond), Return(true)));
    _mobileClient->call("+380911111111");
}

TEST_F(MobileClientTest, shouldSucceedToCall)
{
    registerUser();

    std::string one = "/mobile-network:core/subscribers[number='+380922222222']/state";
    std::string two;
    std::map<std::string, std::string> testMap = {
        {one, two}
    };

    std::string state = "idle";
    std::map<std::string, std::string> testMapSecond = {
        {one, state}
    };

    std::string guestXpathState = "/mobile-network:core/subscribers[number='+380922222222']/state";
    std::string hostXpathState = "/mobile-network:core/subscribers[number='+380911111111']/state";
    std::string changeState = "active";
    std::string guestXpathIncomingNumber = "/mobile-network:core/subscribers[number='+380922222222']/incomingNumber";
    std::string hostXpathIncomingNumber = "/mobile-network:core/subscribers[number='+380911111111']/incomingNumber";
    EXPECT_CALL(*_mock, fetchData(testMap))
            .WillOnce(DoAll(SetArgReferee<0>(testMapSecond), Return(true)));
    EXPECT_CALL(*_mock, changeData(guestXpathState, changeState));
    EXPECT_CALL(*_mock, changeData(hostXpathState, changeState));
    EXPECT_CALL(*_mock, changeData(guestXpathIncomingNumber, std::string("+380911111111")));
    EXPECT_CALL(*_mock, changeData(hostXpathIncomingNumber, std::string("+380922222222")));
    _mobileClient->call("+380922222222");
    _mobileClient->handleModuleChange("active");
}

TEST_F(MobileClientTest, shouldFailToAnswer)
{
    EXPECT_CALL(*_mock, fetchData(_));
    _mobileClient->answer();
}

TEST_F(MobileClientTest, shouldSucceedToAnswer)
{
    callImitation();

    std::string hostXpathIncomingNumber = "/mobile-network:core/subscribers[number='+380911111111']/incomingNumber";
    std::string incomingNumber = "+380922222222";
    std::map<std::string, std::string> testMap = {
        {hostXpathIncomingNumber, incomingNumber}
    };

    std::string state = "busy";
    std::string hostXpathState = "/mobile-network:core/subscribers[number='+380911111111']/state";
    std::string guestXpathState = "/mobile-network:core/subscribers[number='+380922222222']/state";
    EXPECT_CALL(*_mock, fetchData(_))
            .WillOnce(DoAll(SetArgReferee<0>(testMap), Return(true)));
    EXPECT_CALL(*_mock, changeData(guestXpathState, state));
    EXPECT_CALL(*_mock, changeData(hostXpathState, state));
    _mobileClient->answer();
}

TEST_F(MobileClientTest, shouldFailToReject)
{
    EXPECT_CALL(*_mock, fetchData(_));
    _mobileClient->reject();
}

TEST_F(MobileClientTest, shouldSucceedToReject)
{
    callImitation();    
    EXPECT_CALL(*_mock, fetchData(_));
    EXPECT_CALL(*_mock, changeData(_, _))
            .Times(4);
    _mobileClient->reject();
}

TEST_F(MobileClientTest, shouldFailToCallEnd)
{
    EXPECT_CALL(*_mock, fetchData(_));
    _mobileClient->callEnd();
}

TEST_F(MobileClientTest, shouldSuccedToCallEnd)
{
    _mobileClient->handleModuleChange("busy");
    EXPECT_CALL(*_mock, fetchData(_));
    EXPECT_CALL(*_mock, changeData(_, _))
            .Times(4);
    _mobileClient->callEnd();
}

TEST_F(MobileClientTest, shouldSucceedToHandleOperData)
{
    std::string one;
    std::string two;
    _mobileClient->handleOperData(one, two);
    _mobileClient->handleModuleChange("idle");
    _mobileClient->handleModuleChange("deleted");
}

TEST_F(MobileClientTest, shouldSucceedToStopClient)
{
    EXPECT_CALL(*_mock, closeSys());
    EXPECT_TRUE(_mobileClient->stopClient());
}

TEST_F(MobileClientTest, shouldFailToStopClient)
{
    _mobileClient->handleModuleChange("busy");
    EXPECT_FALSE(_mobileClient->stopClient());
}

int main(int argc, char** argv) {
    // use this main function if 'main : true' isn't set in the gtest dependency 
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();    
}

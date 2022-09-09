/*
 *  Copyright 2021 (c) Microsoft Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "LinkManagerStateMachineActiveActiveTest.h"
#include "link_prober/LinkProberStateMachineBase.h"

#define VALIDATE_STATE(p, m, l)                                                     \
    do {                                                                            \
        mTestCompositeState = mFakeMuxPort.getCompositeState();                     \
        EXPECT_EQ(ps(mTestCompositeState), link_prober::LinkProberState::Label::p); \
        EXPECT_EQ(ms(mTestCompositeState), mux_state::MuxState::Label::m);          \
        EXPECT_EQ(ls(mTestCompositeState), link_state::LinkState::Label::l);        \
    } while (0)

#define VALIDATE_PEER_STATE(p, m)                                                                 \
    do {                                                                                          \
        EXPECT_EQ(mFakeMuxPort.getPeerLinkProberState(), link_prober::LinkProberState::Label::p); \
        EXPECT_EQ(mFakeMuxPort.getPeerMuxState(), mux_state::MuxState::Label::m);                 \
    } while (0)

namespace test
{

LinkManagerStateMachineActiveActiveTest::LinkManagerStateMachineActiveActiveTest()
    : mDbInterfacePtr(std::make_shared<FakeDbInterface>(&mIoService)),
      mFakeMuxPort(
          mDbInterfacePtr,
          mMuxConfig,
          mPortName,
          mServerId,
          mIoService,
          mPortCableType
      )
{
    mMuxConfig.setTimeoutIpv4_msec(10);
    mMuxConfig.setPositiveStateChangeRetryCount(mPositiveUpdateCount);
    mMuxConfig.setMuxStateChangeRetryCount(mPositiveUpdateCount);
    mMuxConfig.setLinkStateChangeRetryCount(mPositiveUpdateCount);
}

void LinkManagerStateMachineActiveActiveTest::runIoService(uint32_t count)
{
    if (count == 0) {
        if (mIoService.stopped()) {
            mIoService.restart();
        }
        mIoService.run();
    }

    for (uint8_t i = 0; i < count; i++) {
        if (mIoService.stopped()) {
            mIoService.restart();
        }
        mIoService.run_one();
    }
}

void LinkManagerStateMachineActiveActiveTest::pollIoService(uint32_t count)
{
    if (count == 0) {
        if (mIoService.stopped()) {
            mIoService.restart();
        }
        mIoService.poll();
    }

    for (uint8_t i = 0; i < count; i++) {
        if (mIoService.stopped()) {
            mIoService.restart();
        }
        mIoService.poll_one();
    }
}

void LinkManagerStateMachineActiveActiveTest::postLinkProberEvent(link_prober::LinkProberState::Label label, uint32_t count)
{
    switch (label) {
        case link_prober::LinkProberState::Active:
            for (uint8_t i = 0; i < mMuxConfig.getPositiveStateChangeRetryCount(); i++) {
                mFakeMuxPort.mFakeLinkProber->postLinkProberEvent<link_prober::IcmpSelfEvent&>(
                    link_prober::LinkProberStateMachineBase::getIcmpSelfEvent()
                );
                runIoService(count);
            }
            break;
        case link_prober::LinkProberState::Unknown:
            for (uint8_t i = 0; i < mMuxConfig.getNegativeStateChangeRetryCount(); i++) {
                mFakeMuxPort.mFakeLinkProber->postLinkProberEvent<link_prober::IcmpUnknownEvent&>(
                    link_prober::LinkProberStateMachineBase::getIcmpUnknownEvent()
                );
                runIoService(count);
            }
            break;
        default:
            break;
    }
}

void LinkManagerStateMachineActiveActiveTest::postPeerLinkProberEvent(link_prober::LinkProberState::Label label, uint32_t count)
{
    switch (label) {
        case link_prober::LinkProberState::PeerActive:
            for (uint8_t i = 0; i < mMuxConfig.getPositiveStateChangeRetryCount(); i++) {
                mFakeMuxPort.mFakeLinkProber->postLinkProberEvent<link_prober::IcmpPeerActiveEvent&>(
                    link_prober::LinkProberStateMachineBase::getIcmpPeerActiveEvent()
                );
                runIoService(count);
            }
            break;
        case link_prober::LinkProberState::PeerUnknown:
            for (uint8_t i = 0; i < mMuxConfig.getNegativeStateChangeRetryCount(); i++) {
                mFakeMuxPort.mFakeLinkProber->postLinkProberEvent<link_prober::IcmpPeerUnknownEvent&>(
                    link_prober::LinkProberStateMachineBase::getIcmpPeerUnknownEvent()
                );
                runIoService(count);
            }
            break;
        default:
            break;
    }
}

void LinkManagerStateMachineActiveActiveTest::postMuxEvent(mux_state::MuxState::Label label, uint32_t count)
{
    mux_state::MuxStateMachine& muxStateMachine = mFakeMuxPort.getMuxStateMachine();
    for (uint8_t i = 0; i < mMuxConfig.getMuxStateChangeRetryCount(); i++) {
        switch (label) {
            case mux_state::MuxState::Active:
                muxStateMachine.postMuxStateEvent(mux_state::MuxStateMachine::getActiveEvent());
                break;
            case mux_state::MuxState::Standby:
                muxStateMachine.postMuxStateEvent(mux_state::MuxStateMachine::getStandbyEvent());
                break;
            case mux_state::MuxState::Unknown:
                muxStateMachine.postMuxStateEvent(mux_state::MuxStateMachine::getUnknownEvent());
                break;
            case mux_state::MuxState::Error:
                muxStateMachine.postMuxStateEvent(mux_state::MuxStateMachine::getErrorEvent());
                break;
            default:
                break;
        }
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::postLinkEvent(link_state::LinkState::Label label, uint32_t count)
{
    link_state::LinkStateMachine& linkStateMachine = mFakeMuxPort.getLinkStateMachine();
    for (uint8_t i = 0; i < mMuxConfig.getLinkStateChangeRetryCount(); i++) {
        switch (label) {
            case link_state::LinkState::Up:
                linkStateMachine.postLinkStateEvent(link_state::LinkStateMachine::getUpEvent());
                break;
            case link_state::LinkState::Down:
                linkStateMachine.postLinkStateEvent(link_state::LinkStateMachine::getDownEvent());
                break;
            default:
                break;
        }
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::handleMuxState(std::string state, uint32_t count)
{
    for (uint8_t i = 0; i < mPositiveUpdateCount; i++) {
        mFakeMuxPort.handleMuxState(state);
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::handlePeerMuxState(std::string state, uint32_t count)
{
    for (uint8_t i = 0; i < mPositiveUpdateCount; i++) {
        mFakeMuxPort.handlePeerMuxState(state);
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::handleProbeMuxState(std::string state, uint32_t count)
{
    for (uint8_t i = 0; i < mPositiveUpdateCount; i++) {
        mFakeMuxPort.handleProbeMuxState(state);
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::handleLinkState(std::string linkState, uint32_t count)
{
    for (uint8_t i = 0; i < mMuxConfig.getLinkStateChangeRetryCount(); i++) {
        mFakeMuxPort.handleLinkState(linkState);
        runIoService(count);
    }
}

void LinkManagerStateMachineActiveActiveTest::handleMuxConfig(std::string config, uint32_t count)
{
    mFakeMuxPort.handleMuxConfig(config);
    runIoService(count);
}

void LinkManagerStateMachineActiveActiveTest::activateStateMachine(bool enable_feature_default_route)
{
    mMuxConfig.enableDefaultRouteFeature(enable_feature_default_route);
    mFakeMuxPort.activateStateMachine();
}

void LinkManagerStateMachineActiveActiveTest::setMuxActive()
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    postLinkProberEvent(link_prober::LinkProberState::Active, 3);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("active", 3);
    VALIDATE_STATE(Active, Active, Up);
}

void LinkManagerStateMachineActiveActiveTest::setMuxStandby()
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    postLinkProberEvent(link_prober::LinkProberState::Unknown, 3);
    VALIDATE_STATE(Unknown, Standby, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);

    handleMuxState("standby", 3);
    VALIDATE_STATE(Unknown, Standby, Up);
}

void LinkManagerStateMachineActiveActiveTest::postDefaultRouteEvent(std::string routeState, uint32_t count)
{
    mFakeMuxPort.handleDefaultRouteState(routeState);
    runIoService(count);
}

const common::MuxPortConfig &LinkManagerStateMachineActiveActiveTest::getMuxPortConfig()
{
    return mFakeMuxPort.getMuxPortConfig();
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActive)
{
    setMuxActive();
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveLinkProberUnknown)
{
    setMuxActive();

    postLinkProberEvent(link_prober::LinkProberState::Unknown, 3);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mSuspendTxProbeCallCount, 1);
    VALIDATE_STATE(Unknown, Standby, Up);

    handleMuxState("standby", 3);
    VALIDATE_STATE(Unknown, Standby, Up);

    postLinkProberEvent(link_prober::LinkProberState::Active, 3);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 3);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveLinkDown)
{
    setMuxActive();

    postLinkEvent(link_state::LinkState::Label::Down, 2);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);
    VALIDATE_STATE(Active, Standby, Down);

    postLinkProberEvent(link_prober::LinkProberState::Unknown, 3);
    VALIDATE_STATE(Unknown, Standby, Down);

    postLinkEvent(link_state::LinkState::Label::Up, 2);
    VALIDATE_STATE(Unknown, Standby, Up);

    postLinkProberEvent(link_prober::LinkProberState::Active, 3);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 3);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("active", 3);
    VALIDATE_STATE(Active, Active, Up);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveConfigStandby)
{
    setMuxActive();

    handleMuxConfig("standby", 1);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);
    VALIDATE_STATE(Active, Standby, Up);

    handleMuxState("standby", 3);
    VALIDATE_STATE(Active, Standby, Up);

    postLinkProberEvent(link_prober::LinkProberState::Active, 3);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    VALIDATE_STATE(Active, Standby, Up);

    handleMuxConfig("auto", 1);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 3);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);
    VALIDATE_STATE(Active, Active, Up);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveLinkProberPeerActive)
{
    setMuxActive();

    VALIDATE_PEER_STATE(PeerWait, Wait);

    postPeerLinkProberEvent(link_prober::LinkProberState::PeerActive);
    VALIDATE_PEER_STATE(PeerActive, Active);
    EXPECT_EQ(mDbInterfacePtr->mSetPeerMuxStateInvokeCount, 0);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveLinkProberPeerUnknown)
{
    setMuxActive();

    VALIDATE_PEER_STATE(PeerWait, Wait);

    postPeerLinkProberEvent(link_prober::LinkProberState::PeerUnknown, 3);
    VALIDATE_PEER_STATE(PeerUnknown, Standby);
    EXPECT_EQ(mDbInterfacePtr->mSetPeerMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetPeerMuxState, mux_state::MuxState::Label::Standby);

    handlePeerMuxState("standby", 1);
    VALIDATE_PEER_STATE(PeerUnknown, Standby);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActiveConfigDetachedLinkProberPeerUnknown)
{
    setMuxActive();

    postPeerLinkProberEvent(link_prober::LinkProberState::PeerActive);
    VALIDATE_PEER_STATE(PeerActive, Active);

    handleMuxConfig("detach", 1);
    postPeerLinkProberEvent(link_prober::LinkProberState::PeerUnknown, 3);

    VALIDATE_PEER_STATE(PeerUnknown, Active);
    EXPECT_EQ(mDbInterfacePtr->mSetPeerMuxStateInvokeCount, 0);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxStandby)
{
    setMuxStandby();
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxStandbyLinkDown)
{
    setMuxStandby();

    postLinkEvent(link_state::LinkState::Label::Down, 2);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    VALIDATE_STATE(Unknown, Standby, Down);

    postLinkEvent(link_state::LinkState::Label::Up, 2);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    VALIDATE_STATE(Unknown, Standby, Up);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxStandbyLinkProberPeerActive)
{
    setMuxStandby();

    VALIDATE_PEER_STATE(PeerWait, Wait);

    postPeerLinkProberEvent(link_prober::LinkProberState::PeerActive);
    VALIDATE_PEER_STATE(PeerActive, Active);
    EXPECT_EQ(mDbInterfacePtr->mSetPeerMuxStateInvokeCount, 0);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxStandbyLinkProberPeerUnknown)
{
    setMuxStandby();

    VALIDATE_PEER_STATE(PeerWait, Wait);

    postPeerLinkProberEvent(link_prober::LinkProberState::PeerUnknown);
    VALIDATE_PEER_STATE(PeerUnknown, Wait);
    EXPECT_EQ(mDbInterfacePtr->mSetPeerMuxStateInvokeCount, 0);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, MuxActivDefaultRouteState)
{
    setMuxActive();

    postDefaultRouteEvent("ok", 1);
    EXPECT_FALSE(mMuxConfig.getIfEnableDefaultRouteFeature());
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mShutdownTxProbeCallCount, 0);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mRestartTxProbeCallCount, 1);

    postDefaultRouteEvent("na", 1);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mShutdownTxProbeCallCount, 0);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mRestartTxProbeCallCount, 2);

    mMuxConfig.enableDefaultRouteFeature(true);
    postDefaultRouteEvent("na", 1);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mShutdownTxProbeCallCount, 1);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mRestartTxProbeCallCount, 2);

    postDefaultRouteEvent("ok", 1);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mShutdownTxProbeCallCount, 1);
    EXPECT_EQ(mFakeMuxPort.mFakeLinkProber->mRestartTxProbeCallCount, 3);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, LinkmgrdBootupSequenceHeartBeatFirst)
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    // the first toggle fails because the the inital mux state
    // is standby when linkmgrd first boots up
    postLinkProberEvent(link_prober::LinkProberState::Active, 4);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    // now linkmgrd should be stuck in mux wait timeout

    handleProbeMuxState("unknown", 3);
    VALIDATE_STATE(Active, Unknown, Up);

    // now linkmgrd should be stuck in mux probe timeout
    runIoService(4);

    // xcvrd now answers the mux probe
    handleProbeMuxState("active", 4);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, LinkmgrdBootupSequenceHeartBeatFirstMuxUnknown)
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    // the first toggle fails because the the inital mux state
    // is standby when linkmgrd first boots up
    postLinkProberEvent(link_prober::LinkProberState::Active, 4);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("unknown", 3);
    VALIDATE_STATE(Active, Unknown, Up);

    handleProbeMuxState("unknown", 3);
    VALIDATE_STATE(Active, Unknown, Up);

    // xcvrd now answers the mux probe
    handleProbeMuxState("active", 4);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, LinkmgrdBootupSequenceMuxConfigActiveProbeActive)
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    postLinkProberEvent(link_prober::LinkProberState::Unknown, 3);
    VALIDATE_STATE(Unknown, Standby, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);

    handleMuxState("unknown", 3);
    VALIDATE_STATE(Unknown, Unknown, Up);

    handleMuxConfig("active", 2);
    VALIDATE_STATE(Unknown, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("unknown", 5);
    VALIDATE_STATE(Unknown, Unknown, Up);

    handleProbeMuxState("unknown", 3);
    VALIDATE_STATE(Unknown, Unknown, Up);

    // xcvrd now answers the mux probe
    handleProbeMuxState("active", 4);
    VALIDATE_STATE(Unknown, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 3);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("active", 3);
    VALIDATE_STATE(Unknown, Active, Up);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, LinkmgrdBootupSequenceMuxConfigActiveProbeStandby)
{
    activateStateMachine();
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    postLinkProberEvent(link_prober::LinkProberState::Unknown, 3);
    VALIDATE_STATE(Unknown, Standby, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Standby);

    handleMuxState("unknown", 3);
    VALIDATE_STATE(Unknown, Unknown, Up);

    handleMuxConfig("active", 2);
    VALIDATE_STATE(Unknown, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 2);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);

    handleMuxState("unknown", 5);
    VALIDATE_STATE(Unknown, Unknown, Up);

    handleProbeMuxState("unknown", 4);
    VALIDATE_STATE(Unknown, Unknown, Up);

    // xcvrd now answers the mux probe
    handleProbeMuxState("standby", 4);
    EXPECT_EQ(mDbInterfacePtr->mSetMuxStateInvokeCount, 3);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxState, mux_state::MuxState::Label::Active);
    handleMuxState("active", 3);
    VALIDATE_STATE(Unknown, Active, Up);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, LinkmgrdBootupSequenceWriteActive)
{
    activateStateMachine(true);
    VALIDATE_STATE(Wait, Wait, Down);

    postLinkEvent(link_state::LinkState::Up);
    VALIDATE_STATE(Wait, Wait, Up);

    handleMuxState("active", 3);
    VALIDATE_STATE(Wait, Active, Up);

    postLinkProberEvent(link_prober::LinkProberState::Active, 4);
    VALIDATE_STATE(Active, Active, Up);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxLinkmgrState, link_manager::LinkManagerStateMachineBase::Label::Unhealthy);

    postDefaultRouteEvent("ok", 1);
    EXPECT_EQ(mDbInterfacePtr->mLastSetMuxLinkmgrState, link_manager::LinkManagerStateMachineBase::Label::Healthy);
}

TEST_F(LinkManagerStateMachineActiveActiveTest, SetMuxConfigAutoBeforeInit)
{
    EXPECT_EQ(getMuxPortConfig().getMode(), common::MuxPortConfig::Mode::Auto);
    handleMuxConfig("active", 1);
    EXPECT_EQ(getMuxPortConfig().getMode(), common::MuxPortConfig::Mode::Active);
}

} /* namespace test */

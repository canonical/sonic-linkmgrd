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

#include <boost/bind/bind.hpp>

#include "MuxPort.h"
#include "common/MuxException.h"
#include "common/MuxLogger.h"
#include "link_manager/LinkManagerStateMachineBase.h"

namespace link_manager {
LinkProberEvent LinkManagerStateMachineBase::mLinkProberEvent;
MuxStateEvent LinkManagerStateMachineBase::mMuxStateEvent;
LinkStateEvent LinkManagerStateMachineBase::mLinkStateEvent;

std::vector<std::string> LinkManagerStateMachineBase::mLinkProberStateName = {"Active", "Standby", "Unknown", "Wait"};
std::vector<std::string> LinkManagerStateMachineBase::mMuxStateName = {"Active", "Standby", "Unknown", "Error", "Wait"};
std::vector<std::string> LinkManagerStateMachineBase::mLinkStateName = {"Up", "Down"};
std::vector<std::string> LinkManagerStateMachineBase::mLinkHealthName = {"Uninitialized", "Unhealthy", "Healthy"};

//
// ---> LinkManagerStateMachineBase::LinkManagerStateMachineBase(
//             boost::asio::io_service::strand &strand,
//             common::MuxPortConfig &muxPortConfig,
//             CompositeState initialCompositeState);
//
// class constructor
//
LinkManagerStateMachineBase::LinkManagerStateMachineBase(
    mux::MuxPort *muxPortPtr,
    boost::asio::io_service::strand &strand,
    common::MuxPortConfig &muxPortConfig,
    CompositeState initialCompositeState)
    : StateMachine(strand, muxPortConfig),
      mCompositeState(initialCompositeState),
      mMuxPortPtr(muxPortPtr),
      mMuxStateMachine(this, strand, muxPortConfig, ms(mCompositeState)),
      mLinkStateMachine(this, strand, muxPortConfig, ls(mCompositeState))
{
    switch (mMuxPortConfig.getPortCableType()) {
        case common::MuxPortConfig::PortCableType::ActiveStandby:
            mLinkProberStateMachinePtr = std::make_shared<link_prober::LinkProberStateMachineActiveStandby>(
                this, strand, mMuxPortConfig, ps(mCompositeState)
            );
            break;
        default:
            break;
    }
}

//
// ---> initializeTransitionFunctionTable()
//
// initialize transition function table to NOOP functions
//
void LinkManagerStateMachineBase::initializeTransitionFunctionTable()
{
    MUXLOGWARNING("Initialize State Transition Table With NO-OP...");
    for (uint8_t linkProberState = link_prober::LinkProberState::Label::Active;
         linkProberState < link_prober::LinkProberState::Label::Count;
         linkProberState++) {
        for (uint8_t muxState = mux_state::MuxState::Label::Active;
             muxState < mux_state::MuxState::Label::Count; muxState++) {
            for (uint8_t linkState = link_state::LinkState::Label::Up;
                 linkState < link_state::LinkState::Label::Count; linkState++) {
                mStateTransitionHandler[linkProberState][muxState][linkState] = boost::bind(&LinkManagerStateMachineBase::noopTransitionFunction, this, boost::placeholders::_1);
            }
        }
    }
}

//
// ---> noopTransitionFunction(CompositeState &nextState)
//
// NO-OP transition function
//
void LinkManagerStateMachineBase::noopTransitionFunction(CompositeState &nextState)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handlePeerStateChange(LinkProberEvent& event, link_prober::LinkProberState::Label state)
//
// handle peer LinkProberEvent
//
void LinkManagerStateMachineBase::handlePeerStateChange(LinkProberEvent& event, link_prober::LinkProberState::Label state)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSwssBladeIpv4AddressUpdate(boost::asio::ip::address address);
//
// initialize LinkProber component. Note if this is the last component to be initialized,
// state machine will be activated
//
void LinkManagerStateMachineBase::handleSwssBladeIpv4AddressUpdate(boost::asio::ip::address address)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSwssSoCIpv4AddressUpdate(boost::asio::ip::address address);
//
// initialize LinkProber component. Note if this is the last component to be initialized,
// state machine will be activated
//
void LinkManagerStateMachineBase::handleSwssSoCIpv4AddressUpdate(boost::asio::ip::address address)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleGetServerMacAddressNotification(std::array<uint8_t, ETHER_ADDR_LEN> address);
//
// handle get Server MAC address
//
void LinkManagerStateMachineBase::handleGetServerMacAddressNotification(std::array<uint8_t, ETHER_ADDR_LEN> address)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleGetMuxStateNotification(mux_state::MuxState::Label label);
//
// handle get MUX state notification
//
void LinkManagerStateMachineBase::handleGetMuxStateNotification(mux_state::MuxState::Label label)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleProbeMuxStateNotification(mux_state::MuxState::Label label);
//
// handle MUX state notification. Source of notification could be app_db via xcvrd
//
void LinkManagerStateMachineBase::handleProbeMuxStateNotification(mux_state::MuxState::Label label)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleMuxStateNotification(mux_state::MuxState::Label label);
//
// handle MUX state notification
//
void LinkManagerStateMachineBase::handleMuxStateNotification(mux_state::MuxState::Label label)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSwssLinkStateNotification(const link_state::LinkState::Label label);
//
// handle link state change notification
//
void LinkManagerStateMachineBase::handleSwssLinkStateNotification(const link_state::LinkState::Label label)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

// ---> handlePeerLinkStateNotification(const link_state::LinkState::Label label);
//
// handle peer link state change notification
//
void LinkManagerStateMachineBase::handlePeerLinkStateNotification(const link_state::LinkState::Label label)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleMuxConfigNotification(const common::MuxPortConfig::Mode mode);
//
// handle MUX configuration change notification
//
void LinkManagerStateMachineBase::handleMuxConfigNotification(const common::MuxPortConfig::Mode mode)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSuspendTimerExpiry();
//
// handle suspend timer expiry notification from LinkProber
//
void LinkManagerStateMachineBase::handleSuspendTimerExpiry()
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSwitchActiveCommandCompletion();
//
// handle completion of sending switch command to peer ToR
//
void LinkManagerStateMachineBase::handleSwitchActiveCommandCompletion()
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleSwitchActiveRequestEvent();
//
// handle switch active request from peer ToR
//
void LinkManagerStateMachineBase::handleSwitchActiveRequestEvent()
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handleDefaultRouteStateNotification(const DefaultRoute routeState);
//
// handle default route state notification from routeorch
//
void LinkManagerStateMachineBase::handleDefaultRouteStateNotification(const DefaultRoute routeState)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
//
// ---> shutdownOrRestartLinkProberOnDefaultRoute();
//
// shutdown or restart link prober based on default route state
//
void LinkManagerStateMachineBase::shutdownOrRestartLinkProberOnDefaultRoute()
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

//
// ---> handlePostPckLossRatioNotification(const uint64_t unknownEventCount, const uint64_t expectedPacketCount);
//
// handle post pck loss ratio
//
void LinkManagerStateMachineBase::handlePostPckLossRatioNotification(const uint64_t unknownEventCount, const uint64_t expectedPacketCount)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

// ---> handleResetLinkProberPckLossCount();
//
// reset link prober heartbeat packet loss count
//
void LinkManagerStateMachineBase::handleResetLinkProberPckLossCount()
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
}

// ---> LinkManagerStateMachineBase::setComponentInitState(uint8_t component)
//
// set component inti state. This method is used for testing
//
void LinkManagerStateMachineBase::setComponentInitState(uint8_t component)
{
    MUXLOGINFO(mMuxPortConfig.getPortName());
};

} /* namespace link_manager */

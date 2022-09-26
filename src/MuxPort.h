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

/*
 * MuxPort.h
 *
 *  Created on: Oct 7, 2020
 *      Author: tamer
 */

#ifndef MUXPORT_H_
#define MUXPORT_H_

#include <string>
#include <memory>

#include "link_prober/LinkProber.h"
#include "link_prober/LinkProberStateMachine.h"
#include "link_manager/LinkManagerStateMachine.h"

#include "common/MuxPortConfig.h"
#include "DbInterface.h"

namespace test {
class MuxManagerTest;
class FakeMuxPort;
}

namespace mux
{

/**
 *@class MuxPort
 *
 *@brief Hold MUX configuration data, state machines and link prober
 */
class MuxPort: public std::enable_shared_from_this<MuxPort>
{
public:
    /**
    *@method MuxPort
    *
    *@brief class default constructor
    */
    MuxPort() = delete;

    /**
    *@method MuxPort
    *
    *@brief class copy constructor
    *
    *@param MuxPort (in)  reference to MuxPort object to be copied
    */
    MuxPort(const MuxPort &) = delete;

    /**
    *@method MuxPort
    *
    *@brief class constructor
    *
    *@param dbInterfacePtr (in) pointer to DbInterface object
    *@param muxConfig (in)      reference to MuxConfig object
    *@param portName (in)       reference to port name
    *@param serverId (in)       server/blade id
    *@param ioService (in)      reference to Boost IO Service object
    */
    MuxPort(
        std::shared_ptr<mux::DbInterface> dbInterfacePtr,
        common::MuxConfig &muxConfig,
        const std::string &portName,
        uint16_t serverId,
        boost::asio::io_service &ioService
    );

    /**
    *@method ~MuxPort
    *
    *@brief class destructor
    */
    virtual ~MuxPort() = default;

    /**
    *@method getMuxPortConfig
    *
    *@brief getter for MuxPortConfig object
    *
    *@return reference to MuxPortConfig object
    */
    inline const common::MuxPortConfig& getMuxPortConfig() const {return mMuxPortConfig;};

    /**
    *@method setMuxState
    *
    *@brief set MUX state in APP DB for orchagent processing
    *
    *@param label (in)      label of target state
    *
    *@return none
    */
    virtual inline void setMuxState(mux_state::MuxState::Label label) {mDbInterfacePtr->setMuxState(mMuxPortConfig.getPortName(), label);};

    /**
    *@method getMuxState
    *
    *@brief retrieve the current MUX state
    *
    *@param portName (in)   MUX/port name
    *
    *@return none
    */
    inline void getMuxState() {mDbInterfacePtr->getMuxState(mMuxPortConfig.getPortName());};

    /**
    *@method probeMuxState
    *
    *@brief trigger xcvrd to read MUX state using i2c
    *
    *@param portName (in)   MUX/port name
    *
    *@return label of MUX state
    */
    inline void probeMuxState() {mDbInterfacePtr->probeMuxState(mMuxPortConfig.getPortName());};

    /**
    *@method setMuxLinkmgrState
    *
    *@brief set MUX LinkMgr state in State DB for cli processing
    *
    *@param label (in)      label of target state
    *
    *@return none
    */
    inline void setMuxLinkmgrState(link_manager::LinkManagerStateMachine::Label label) {
        mDbInterfacePtr->setMuxLinkmgrState(mMuxPortConfig.getPortName(), label);
    };

    /**
    *@method postMetricsEvent
    *
    *@brief post MUX metrics event
    *
    *@param metrics (in)    metrics to post
    *@param label (in)      label of target state
    *
    *@return none
    */
    virtual inline void postMetricsEvent(
        link_manager::LinkManagerStateMachine::Metrics metrics,
        mux_state::MuxState::Label label
    ) {
        mDbInterfacePtr->postMetricsEvent(mMuxPortConfig.getPortName(), metrics, label);
    };

    /**
     * @method postSwitchCause
     * 
     * @brief post mux switch cause 
     * 
     * @param cause (in) switch cause to post 
     * 
     * @return none
     */
    virtual inline void postSwitchCause(
        link_manager::LinkManagerStateMachine::SwitchCause cause
    ) {
        mDbInterfacePtr->postSwitchCause(mMuxPortConfig.getPortName(), cause);
    };

    /**
     * @method postLinkProberMetricsEvent
     * 
     * @brief post link prober pck loss event
     * 
     * @param metrics (in) metrics to post
     * 
     * @return none
    */
    inline void postLinkProberMetricsEvent(link_manager::LinkManagerStateMachine::LinkProberMetrics metrics) {
        mDbInterfacePtr->postLinkProberMetricsEvent(mMuxPortConfig.getPortName(), metrics);
    };

    /**
     * @method postPckLossRatio
     * 
     * @brief post pck loss ratio update to state db 
     * 
     * @param unknownEventCount (in) count of missing icmp packets
     * @param expectedPacketCount (in) count of expected icmp packets 
     * 
     * @return none
    */
    inline void postPckLossRatio(const uint64_t unknownEventCount, const uint64_t expectedPacketCount) {
        mDbInterfacePtr->postPckLossRatio(mMuxPortConfig.getPortName(), unknownEventCount, expectedPacketCount);
    };

    /**
    *@method setServerIpv4Address
    *
    *@brief setter for server/blade IPv4 address
    *
    *@param address (in) server IPv4 address
    *
    *@return none
    */
    inline void setServerIpv4Address(const boost::asio::ip::address &address) {mMuxPortConfig.setBladeIpv4Address(address);};

    /**
    *@method handleBladeIpv4AddressUpdate
    *
    *@brief update server/blade IPv4 Address
    *
    *@param addres (in)  server/blade IP address
    *
    *@return none
    */
     void handleBladeIpv4AddressUpdate(boost::asio::ip::address addres);

    /**
    *@method handleLinkState
    *
    *@brief handles link state updates
    *
    *@param linkState (in)  link state
    *
    *@return none
    */
    void handleLinkState(const std::string &linkState);

    /**
     * @method handlePeerLinkState
     * 
     * @brief handles peer's link state updates 
     * 
     * @param linkState (in) peer's link state
     * 
     * @return none 
    */
    void handlePeerLinkState(const std::string &linkState);

    /**
    *@method handleGetServerMacAddress
    *
    *@brief handles get Server MAC address
    *
    *@param address (in)    Server MAC address
    *
    *@return none
    */
    void handleGetServerMacAddress(const std::array<uint8_t, ETHER_ADDR_LEN> &address);

    /**
    *@method handleGetMuxState
    *
    *@brief handles get MUX state updates
    *
    *@param muxState (in)           link state
    *
    *@return none
    */
    void handleGetMuxState(const std::string &muxState);

    /**
    *@method handleProbeMuxState
    *
    *@brief handles probe MUX state updates
    *
    *@param muxState (in)           link state
    *
    *@return none
    */
    void handleProbeMuxState(const std::string &muxState);

    /**
    *@method handleMuxState
    *
    *@brief handles MUX state updates
    *
    *@param muxState (in)           link state
    *
    *@return none
    */
    void handleMuxState(const std::string &muxState);

    /**
    *@method handleMuxConfig
    *
    *@brief handles MUX config updates when switching between auto/active/manual
    *
    *@param config (in)     MUX new config; auto/active/manual
    *
    *@return none
    */
    void handleMuxConfig(const std::string &config);

    /**
     * @method handleDefaultRouteState(const std::string &routeState)
     * 
     * @brief handles default route state notification
     * 
     * @param routeState 
     * 
     * @return none
    */
    void handleDefaultRouteState(const std::string &routeState);

    /**
     * @method resetPckLossCount
     * 
     * @brief reset ICMP packet loss count 
     * 
     * @return none
    */
    void resetPckLossCount();

    /**
     * @method warmRestartReconciliation
     * 
     * @brief port warm restart reconciliation procedure
     * 
     * @return none
     */
    void warmRestartReconciliation();

protected:
    friend class test::MuxManagerTest;
    friend class test::FakeMuxPort;
    /**
    *@method getLinkManagerStateMachine
    *
    *@brief getter for LinkManagerStateMachine object (used during unit test)
    *
    *@return pointer to LinkManagerStateMachine object
    */
    link_manager::LinkManagerStateMachine* getLinkManagerStateMachine() {return &mLinkManagerStateMachine;};

    /**
    *@method setComponentInitState
    *
    *@brief setter for state machine component initial state (used during unit test)
    *
    *@param component (in)  component index
    */
    void setComponentInitState(uint8_t component) {mLinkManagerStateMachine.setComponentInitState(component);};

private:
    std::shared_ptr<mux::DbInterface> mDbInterfacePtr = nullptr;
    common::MuxPortConfig mMuxPortConfig;
    boost::asio::io_service::strand mStrand;

    link_manager::LinkManagerStateMachine mLinkManagerStateMachine;
};

} /* namespace mux */

#endif /* MUXPORT_H_ */

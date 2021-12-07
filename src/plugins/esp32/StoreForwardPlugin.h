#pragma once

#include "SinglePortPlugin.h"
#include "concurrency/OSThread.h"
#include "mesh/generated/storeforward.pb.h"

#include "configuration.h"
#include <Arduino.h>
#include <functional>

struct PacketHistoryStruct {
    uint32_t time;
    uint32_t to;
    uint32_t from;
    bool ack;
    uint8_t payload[Constants_DATA_PAYLOAD_LEN];
    pb_size_t payload_size;
};

class StoreForwardPlugin : public SinglePortPlugin, private concurrency::OSThread
{
    // bool firstTime = 1;
    bool busy = 0;
    uint32_t busyTo;
    char routerMessage[80];

    uint32_t receivedRecord[50][2] = {{0}};

    PacketHistoryStruct *packetHistory;
    uint32_t packetHistoryCurrent = 0;

    PacketHistoryStruct *packetHistoryTXQueue;
    uint32_t packetHistoryTXQueue_size;
    uint32_t packetHistoryTXQueue_index = 0;

    uint32_t packetTimeMax = 2000;

  public:
    StoreForwardPlugin();

    /**
     Update our local reference of when we last saw that node.
     @return 0 if we have never seen that node before otherwise return the last time we saw the node.
     */
    void historyAdd(const MeshPacket &mp);
    void historyReport();
    void historySend(uint32_t msAgo, uint32_t to);

    uint32_t historyQueueCreate(uint32_t msAgo, uint32_t to);

    /**
     * Send our payload into the mesh
     */
    void sendPayload(NodeNum dest = NODENUM_BROADCAST, uint32_t packetHistory_index = 0);
    void sendMessage(NodeNum dest, char *str);
    virtual MeshPacket *allocReply();
    /*
      Override the wantPortnum method.
      */
    virtual bool wantPortnum(PortNum p) { return true; };

  private:
    void populatePSRAM();

    // S&F Defaults
    uint32_t historyReturnMax = 250; // 250 records
    uint32_t historyReturnWindow = 240; // 4 hours
    uint32_t records = 0; // Calculated
    bool heartbeat = false; // No heartbeat.

  protected:
    virtual int32_t runOnce();

    /** Called to handle a particular incoming message

    @return ProcessMessage::STOP if you've guaranteed you've handled this message and no other handlers should be considered for
    it
    */
    virtual ProcessMessage handleReceived(const MeshPacket &mp);
    virtual ProcessMessage handleReceivedProtobuf(const MeshPacket &mp, StoreAndForward *p);

};

extern StoreForwardPlugin *storeForwardPlugin;
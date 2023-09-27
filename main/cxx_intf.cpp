#include "cxx_intf.h"

#include <MicroOcpp/Core/Connection.h>

#define MSG_GETDIAGNOSTICS "[2,\"msgId\",\"GetConfiguration\",{}]"

class CustomLoopback : public MicroOcpp::Connection {
private:
    MicroOcpp::ReceiveTXTcallback receiveTXT;

    bool connected = true; //for switching off the communication
    unsigned long lastRecv = 0;
    unsigned long lastConn = 0;
public:
    void loop() { }
    bool sendTXT(const char *msg, size_t length) {
        if (!connected) {
            return true;
        }
        if (receiveTXT) {
            lastRecv = mocpp_tick_ms();
            return receiveTXT(msg, length);
        } else {
            return false;
        }
    }
    void setReceiveTXTcallback(MicroOcpp::ReceiveTXTcallback &receiveTXT) {this->receiveTXT = receiveTXT;}
    unsigned long getLastRecv() {return lastRecv;}
    unsigned long getLastConnected() {return lastConn;}
    void setConnected(bool connected) {
        if (connected) {
            lastConn = mocpp_tick_ms();
        }
        this->connected = connected;
    }
    bool isConnected() {return connected;}
};

OCPP_Connection *ocpp_loopback_make() {
    auto conn = new CustomLoopback();
    return reinterpret_cast<OCPP_Connection*>(conn);
}

void ocpp_loopback_free(OCPP_Connection *c_conn) {
    auto conn = reinterpret_cast<CustomLoopback*>(c_conn);
    delete conn;
}

void ocpp_loopback_set_connected(OCPP_Connection *c_conn, bool connected) {
    auto conn = reinterpret_cast<CustomLoopback*>(c_conn);
    conn->setConnected(connected);
}

void ocpp_send_GetDiagnostics(OCPP_Connection *c_conn) {
    auto conn = reinterpret_cast<MicroOcpp::Connection*>(c_conn);

    conn->sendTXT(MSG_GETDIAGNOSTICS, sizeof(MSG_GETDIAGNOSTICS) - 1);
}

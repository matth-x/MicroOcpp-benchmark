#ifndef MICROOCPP_CXX_INTF_H
#define MICROOCPP_CXX_INTF_H

#include <MicroOcppMongooseClient_c.h>

#ifdef __cplusplus
extern "C" {
#endif

OCPP_Connection *ocpp_loopback_make();
void ocpp_loopback_free(OCPP_Connection *);
void ocpp_loopback_set_connected(OCPP_Connection *, bool);

void ocpp_send_GetDiagnostics(OCPP_Connection *);

#ifdef __cplusplus
}
#endif

#endif

#include "pdc_rpc_origin.h"

#ifdef RPC_TYPE_MERCURY
#include "pdc_rpc_mercury.h"
// #elif RPC_TYPE_GRPC
// #include "pdc_rpc_grpc.h"
// #elif RPC_TYPE_THRIFT
// #include "pdc_rpc_thrift.h"
#endif

void
defaultCallMethod(RpcRequest *request, RpcResponse *response)
{
    perr_t err = SUCCESS;

    // Initialize
    err = rpcEngine->rpcInterface->originImpl.initialize();
    if (err != SUCCESS) {
        printf("Failed to initialize.\n");
        return;
    }

    // get server address
    const char *server_address = ;

    // Prepare connection
    err = rpcEngine->rpcInterface->originImpl.prepareConnection();
    if (err != SUCCESS) {
        printf("Failed to prepare connection.\n");
        return;
    }

    // Create RPC
    void *rpc;
    err = rpcEngine->rpcInterface->originImpl.createRpc(&rpc, request->method);
    if (err != SUCCESS) {
        printf("Failed to create RPC.\n");
        return;
    }

    // Perform RPC
    err = rpcEngine->rpcInterface->originImpl.performRpc(rpc, request);
    if (err != SUCCESS) {
        printf("Failed to perform RPC.\n");
        return;
    }

    // Get response
    err = rpcEngine->rpcInterface->originImpl.getResponse(rpc, response);
    if (err != SUCCESS) {
        printf("Failed to get response.\n");
        return;
    }

    // Finalize
    err = rpcEngine->rpcInterface->originImpl.finalize();
    if (err != SUCCESS) {
        printf("Failed to finalize.\n");
        return;
    }

    printf("RPC call succeeded.\n");
}

void
initializeRpcEnginForOrigin()
{
    rpcEngineInit();
    rpcEngine->rpcInterface = (RpcInterface *)calloc(1, sizeof(RpcInterface));
    if (rpcEngine->rpcInterface == NULL) {
        printf("Error with calloc in %s\n", __func__);
        return NULL;
    }
    rpcEngine->rpcInterface->callMethod = defaultCallMethod;
}
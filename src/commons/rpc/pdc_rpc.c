#include "pdc_rpc.h"

RpcEngine *rpcEngine = NULL;

void
rpcEngineInit()
{
    if (rpcEngine != NULL) {
        printf("Error: rpcEngine already initialized.\n");
        return;
    }
    rpcEngine = (RpcEngine *)calloc(1, sizeof(RpcEngine));
    if (rpcEngine == NULL) {
        printf("Error with calloc in %s\n", __func__);
        return NULL;
    }
}

void
rpcEngineFinalize(RpcEngine *rpcEngine)
{
    if (rpcEngine == NULL) {
        printf("No need to finalize rpcEngine\n");
        return;
    }
    free(rpcEngine);
    return;
}

RpcEngine *
getRpcEngine()
{
    return rpcEngine;
}

RpcInterface *
getRpcInterface()
{
    return rpcEngine->rpcInterface;
}
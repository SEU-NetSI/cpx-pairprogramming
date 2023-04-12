#include "pmsis.h"
#include "bsp/bsp.h"
#include "cpx.h"
#include "octoMap.h"
#include "octoTree.h"
#include "communicate.h"
#include <string.h>

static CPXPacket_t packet;
octoMap_t octoMapData;

void mapInit()
{
    octoMap_t* octoMap = &octoMapData;
    octoMapInit(octoMap);

    // print octoMap
    cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]sizeof(octoNode) = %lu\n", sizeof(octoNode_t));
    cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]octoTree->center = (%d, %d, %d), origin = (%d, %d, %d)\n", 
        octoMap->octoTree->center.x, octoMap->octoTree->center.y, octoMap->octoTree->center.z, 
        octoMap->octoTree->origin.x, octoMap->octoTree->origin.y, octoMap->octoTree->origin.z);
    cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]octoTree->resolution = %d, maxDepth = %d, width = %d\n",
        octoMap->octoTree->resolution, octoMap->octoTree->maxDepth, octoMap->octoTree->width);
    cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]root->children = %d, logOdds = %d, isLeaf = %d\n", 
        octoMap->octoTree->root->children, octoMap->octoTree->root->logOdds, octoMap->octoTree->root->isLeaf);
    cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]octoNodeSet->freeQE = %d, fullQE = %d, length = %d, numFree = %d, numOccupied = %d\n", 
        octoMap->octoNodeSet->freeQueueEntry, octoMap->octoNodeSet->fullQueueEntry, 
        octoMap->octoNodeSet->length, octoMap->octoNodeSet->numFree, octoMap->octoNodeSet->numOccupied);
}

void CPXListeningTask(void)
{
    cpxInit();
    cpxEnableFunction(CPX_F_APP);
    mapInit();
    octoMap_t* octoMap = &octoMapData;

    while (1) 
    {
        cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Listening...\n");
        cpxReceivePacketBlocking(CPX_F_APP, &packet);
        uint8_t sourceId = packet.data[0];
        uint8_t reqType = packet.data[1];
        uint16_t seq = packet.data[2];
        if (reqType == MAPPING_REQ) {
            uint8_t mappingRequestPayloadLength = packet.data[3];
            coordinate_pair_t mappingRequestPayload[mappingRequestPayloadLength];
            memcpy(mappingRequestPayload, &packet.data[4], sizeof(coordinate_pair_t)*mappingRequestPayloadLength);
            cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]First pair: (%d, %d, %d) - (%d, %d, %d)\n", 
                mappingRequestPayload[0].startPoint.x, mappingRequestPayload[0].startPoint.y, mappingRequestPayload[0].startPoint.z,
                mappingRequestPayload[0].endPoint.x, mappingRequestPayload[0].endPoint.y, mappingRequestPayload[0].endPoint.z);
            cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Second pair: (%d, %d, %d) - (%d, %d, %d)\n", 
                mappingRequestPayload[1].startPoint.x, mappingRequestPayload[1].startPoint.y, mappingRequestPayload[1].startPoint.z,
                mappingRequestPayload[1].endPoint.x, mappingRequestPayload[1].endPoint.y, mappingRequestPayload[1].endPoint.z);
        } else {
            cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Receive CPX other request from: %d, seq: %d, reqType: %d\n", sourceId, seq, reqType);
        }



        // cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Received!!!\n");
        // coordinate_pair_t coord_pairs[3];
        // memcpy(coord_pairs, &packet.data[0], packet.dataLength);
        // cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]First pair: (%d, %d, %d) - (%d, %d, %d)\n", 
        //     coord_pairs[0].startPoint.x, coord_pairs[0].startPoint.y, coord_pairs[0].startPoint.z,
        //     coord_pairs[0].endPoint.x, coord_pairs[0].endPoint.y, coord_pairs[0].endPoint.z);
        // cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]First pair: (%d, %d, %d) - (%d, %d, %d)\n", 
        //     coord_pairs[1].startPoint.x, coord_pairs[1].startPoint.y, coord_pairs[1].startPoint.z,
        //     coord_pairs[1].endPoint.x, coord_pairs[1].endPoint.y, coord_pairs[1].endPoint.z);
        // cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]First pair: (%d, %d, %d) - (%d, %d, %d)\n", 
        //     coord_pairs[2].startPoint.x, coord_pairs[2].startPoint.y, coord_pairs[2].startPoint.z,
        //     coord_pairs[2].endPoint.x, coord_pairs[2].endPoint.y, coord_pairs[2].endPoint.z);
    }
}

void CPXListeningInit(void)
{
    cpxInit();
    cpxEnableFunction(CPX_F_APP);
    mapInit();
    octoMap_t* octoMap = &octoMapData;

    while (1)
    {
        cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Listening...\n");

        cpxReceivePacketBlocking(CPX_F_APP,&packet);
        coordinate_pair_t coords[3];
        memcpy(coords, &packet.data[0], packet.dataLength-0*sizeof(uint8_t));
        char msg[50]="[GAP8-Edge]Get Msg from: ";
        cpxPrintToConsole(LOG_TO_CRTP, msg);
    }
}

void OctoMapTestTask(void)
{
    cpxInit();
    // while (1) {
    //     cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Hello world\n");
    //     pi_time_wait_us(1000*1000);
    // }
    
    octoMap_t* octoMap = &octoMapData;
    mapInit();

    int updateTime = 0;
    // double startTime = pi_time();
    while(1) {
        coordinate_t startPoint = {0, 0, 0};
        coordinate_t endPoint = {100, 100, 100};
        octoTreeRayCasting(octoMap->octoTree, octoMap, &startPoint, &endPoint);
        updateTime++;
        if (updateTime % 1000 == 0) {
            // double endTime = pi_time();
            cpxPrintToConsole(LOG_TO_CRTP, "[GAP8-Edge]Update %d times, time: %f\n", updateTime);
            // startTime = endTime;
            pi_time_wait_us(1000*1000);
        }
    }
}

// void start_example(void)
// {
//   pi_bsp_init();
//   cpxInit();
//    while (1)
//   {
//       cpxPrintToConsole(LOG_TO_CRTP, "Hello World\n");
//       pi_time_wait_us(1000*1000);
//   }
// }

int main(void)
{
    pi_bsp_init();
    return pmsis_kickoff((void *)OctoMapTestTask);
}

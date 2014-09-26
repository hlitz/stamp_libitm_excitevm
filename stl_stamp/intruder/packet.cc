/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include "packet.h"


/* =============================================================================
 * packet_compareFlowId
 * =============================================================================
 */
__attribute__ ((transaction_safe))
long
packet_compareFlowId (const void* aPtr, const void* bPtr)
{
    packet_t* aPacketPtr = (packet_t*)aPtr;
    packet_t* bPacketPtr = (packet_t*)bPtr;

    return (aPacketPtr->flowId - bPacketPtr->flowId);
}


/* =============================================================================
 * packet_compareFragmentId
 * =============================================================================
 */
__attribute__ ((transaction_safe))
long
packet_compareFragmentId (const void* aPtr, const void* bPtr)
{
    packet_t* aPacketPtr = (packet_t*)aPtr;
    packet_t* bPacketPtr = (packet_t*)bPtr;

    return (int)(aPacketPtr->fragmentId - bPacketPtr->fragmentId);
}


/* =============================================================================
 *
 * End of packet.c
 *
 * =============================================================================
 */

//Bank.cpp
//
//Class file for bank object
//

#include "bank.h"
#include "buspacket.h"
#include <cstdlib>
#include <iomanip>

using namespace std;
using namespace DRAMSim;
unsigned NUM_COLS = 1024;
Bank::Bank(ostream &dramsim_log_):
		rowEntries(NUM_COLS),
		dramsim_log(dramsim_log_)
{}

/* The bank class is just a glorified sparse storage data structure
 * that keeps track of written data in case the simulator wants a
 * function DRAM model
 *
 * A vector of size NUM_COLS keeps a linked list of rows and their
 * associated values.
 *
 * write() adds an entry to the proper linked list or replaces the
 * 	value in a row that was already written
 *
 * read() searches for a node with the right row value, if not found
 * 	returns the tracer value 0xDEADBEEF
 * 
 *	TODO: if anyone wants to actually store data, see the 'data_storage' branch and perhaps try to merge that into master
 */



Bank::DataStruct *Bank::searchForRow(unsigned row, DataStruct *head)
{
	while (head != NULL)
	{
		if (head->row == row)
		{
			//found it
			return head;
		}
		//keep looking
		head = head->next;
	}
	//if we get here, didn't find it
	return NULL;
}

void Bank::read(BusPacket *busPacket)
{
	//DataStruct *rowHeadNode = rowEntries[busPacket->column];
	DataStruct *rowHeadNode = rowEntries[0];
	DataStruct *foundNode = NULL;

	if ((foundNode = Bank::searchForRow(busPacket->row, rowHeadNode)) == NULL)
	{
		// the row hasn't been written before, so it isn't in the list
		//if(SHOW_SIM_OUTPUT) DEBUG("== Warning - Read from previously unwritten row " << busPacket->row);
		//void *garbage = calloc(BL * (JEDEC_DATA_BUS_BITS/8),1);
		void *garbage = calloc(32,1);
		((long *)garbage)[0] = 0xdeadbeef; // tracer value
		busPacket->data = garbage;
		cout<<std::hex<<((long *)busPacket->data)[0]<<endl;
	}
	else // found it
	{
		busPacket->data = foundNode->data;
	}

	//the return packet should be a data packet, not a read packet
	busPacket->busPacketType = DATA;
}


void Bank::write(const BusPacket *busPacket)
{
	//TODO: move all the error checking to BusPacket so once we have a bus packet,
	//			we know the fields are all legal

	if (busPacket->column >= NUM_COLS)
	{
		//ERROR("== Error - Bus Packet column "<< busPacket->column <<" out of bounds");
		exit(-1);
	}

	// head of the list we need to search
	DataStruct *rowHeadNode = rowEntries[busPacket->column];
	DataStruct *foundNode = NULL;

	if ((foundNode = Bank::searchForRow(busPacket->row, rowHeadNode)) == NULL)
	{
		//not found
		DataStruct *newRowNode = (DataStruct *)malloc(sizeof(DataStruct));

		//insert at the head for speed
		//TODO: Optimize this data structure for speedier lookups?
		newRowNode->row = busPacket->row;
		newRowNode->data = busPacket->data;
		newRowNode->next = rowHeadNode;
		rowEntries[busPacket->column] = newRowNode;
	}
	else
	{
		// found it, just plaster in the new data
		foundNode->data = busPacket->data;
//		if (DEBUG_BANKS)
//		{
//			PRINTN(" -- Bank "<<busPacket->bank<<" writing to physical address 0x" << hex << busPacket->physicalAddress<<dec<<":");
//			busPacket->printData();
//			PRINT("");
//		}
	}
}


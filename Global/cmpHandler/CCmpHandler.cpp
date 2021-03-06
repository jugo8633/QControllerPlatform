/*
 * CWmpHandler.cpp
 *
 *  Created on: 2015年10月21日
 *      Author: Louis Ju
 */

#include "CCmpHandler.h"

#include <string.h>

#include "utility.h"
#include "common.h"
#include "packet.h"
#include "CDataHandler.cpp"
#include "LogHandler.h"

static CCmpHandler* cmpHandler = 0;

CCmpHandler::CCmpHandler()
{

}

CCmpHandler::~CCmpHandler()
{

}

CCmpHandler* CCmpHandler::getInstance()
{
	if(0 == cmpHandler)
	{
		cmpHandler = new CCmpHandler();
	}
	return cmpHandler;
}

int CCmpHandler::getCommand(const void *pData)
{
	int nCommand;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	nCommand = ntohl(pHeader->command_id);

	return nCommand;
}

int CCmpHandler::getLength(const void *pData)
{
	int nLength;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	nLength = ntohl(pHeader->command_length);

	return nLength;
}

int CCmpHandler::getStatus(const void *pData)
{
	int nStatus;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	nStatus = ntohl(pHeader->command_status);

	return nStatus;
}

int CCmpHandler::getSequence(const void *pData)
{
	int nSequence;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	nSequence = ntohl(pHeader->sequence_number);

	return nSequence;
}

void CCmpHandler::formatRespPacket(int nCommand, int nStatus, int nSequence, void ** pHeader)
{
	int nCommand_resp;
	nCommand_resp = generic_nack | nCommand;

	((CMP_HEADER*) *pHeader)->command_id = htonl(nCommand_resp);
	((CMP_HEADER*) *pHeader)->command_status = htonl(nStatus);
	((CMP_HEADER*) *pHeader)->sequence_number = htonl(nSequence);
	((CMP_HEADER*) *pHeader)->command_length = htonl(sizeof(CMP_HEADER));

}

void CCmpHandler::formatReqPacket(int nCommand, int nStatus, int nSequence, void ** pHeader)
{

	((CMP_HEADER*) *pHeader)->command_id = htonl(nCommand);
	((CMP_HEADER*) *pHeader)->command_status = htonl(nStatus);
	((CMP_HEADER*) *pHeader)->sequence_number = htonl(nSequence);
	((CMP_HEADER*) *pHeader)->command_length = htonl(sizeof(CMP_HEADER));

}

void CCmpHandler::formatHeader(int nCommand, int nStatus, int nSequence, void ** pHeader)
{

	((CMP_HEADER*) *pHeader)->command_id = htonl(nCommand);
	((CMP_HEADER*) *pHeader)->command_status = htonl(nStatus);
	((CMP_HEADER*) *pHeader)->sequence_number = htonl(nSequence);
	((CMP_HEADER*) *pHeader)->command_length = htonl(sizeof(CMP_HEADER));

}

int CCmpHandler::formatPacket(int nCommand, void ** pPacket, int nBodyLen)
{
	int nTotal_len;
	static int seqnum = 0;
	CMP_HEADER wmpHeader;

	wmpHeader.command_id = htonl(nCommand);
	wmpHeader.command_status = htonl(0);
	wmpHeader.sequence_number = htonl(++seqnum);
	nTotal_len = sizeof(CMP_HEADER) + nBodyLen;
	wmpHeader.command_length = htonl(nTotal_len);
	memcpy(*pPacket, &wmpHeader, sizeof(CMP_HEADER));

	return nTotal_len;
}

void CCmpHandler::getSourcePath(const void *pData, char **pPath)
{
	char* pBody;

	pBody = (char*) ((char *) pData + sizeof(CMP_HEADER));
	if(0 != pBody)
	{
		strcpy(*pPath, pBody);
	}
}

int CCmpHandler::parseBody(int nCommand, const void *pData, CDataHandler<std::string> &rData)
{
	if(0 < (getLength(pData) - sizeof(CMP_HEADER)))
	{
		char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
		if(isValidStr((const char*) pBody, MAX_SIZE))
		{
			//_log("isValidStr is true ");
			char temp[MAX_SIZE];
			memset(temp, 0, sizeof(temp));
			strcpy(temp, pBody);
			rData.setData("data", temp);
		}
	}
	else
	{
		_log("[CMP Parser]CMP body length error");
	}

	return rData.size();
}

int CCmpHandler::parseBody(const void *pData, vector<string> &vData)
{
	char * pch;
	char * pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));

	pch = strtok(pBody, " ");
	while(pch != NULL)
	{
		vData.push_back(string(pch));
		pch = strtok( NULL, " ");
	}

	return vData.size();
}

bool CCmpHandler::isAckPacket(int nCommand)
{
	unsigned int nCommand_resp;

	nCommand_resp = generic_nack & nCommand;

	if( generic_nack == nCommand_resp)
		return true;
	else
		return false;
}


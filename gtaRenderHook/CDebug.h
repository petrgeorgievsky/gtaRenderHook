#ifndef DEBUG_H
#define DEBUG_H
#include "stdafx.h"
using namespace std;
class CDebug
{
public:
	CDebug(string acFileName): pLogStream{ new ofstream(acFileName) }
	{}
	void printMsg(string msg) { if(pLogStream) *pLogStream << "Debug Message: "<< msg<<"\n"; }
	void printError(string msg) { if (pLogStream) *pLogStream << "Debug Error: " << msg << "\n"; }
	~CDebug()
	{
		if (pLogStream)
			delete pLogStream;
	}
private:
	ofstream* pLogStream = nullptr;
};
#endif // !DEBUG_H


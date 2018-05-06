#ifndef DEBUG_H
#define DEBUG_H
#include "SettingsHolder.h"
using namespace std;
typedef chrono::high_resolution_clock hr_clock;

// Debug Counter, useful for call counting, per frame or per session
class CCounter {
private:
	int		m_Count				= 0;
	int		m_FrameCount		= 0;
	int		m_FullChangeCount	= 0;
	bool	m_TimerStart		= false;
	int		m_minCount			= 100000000;
	int		m_maxCount			= 0;
	string	m_Name{ NULL };
public:
	explicit CCounter(const string& name) :m_Name{ name } {
	}
	// Increase count.
	void Increase() {
		if(m_TimerStart)
			m_Count++;
	}
	// Begin counting
	void Start() {
		m_TimerStart = true;
	}
	// Stop counting
	void Stop() {
		m_TimerStart = false;
		m_FullChangeCount += m_Count;
		m_FrameCount++;
		m_minCount = min(m_minCount, m_Count);
		m_maxCount = max(m_maxCount, m_Count);
		m_Count = 0;
	}
	// Return counter debug info.
	string GetCounterResult() {
		return  m_Name + " Count: " + to_string(m_Count) +
			"; Per Frame - Min:" + to_string(m_minCount) +
			"; Max:" + to_string(m_maxCount) +
			"; Avg:" + to_string(m_FullChangeCount / max(m_FrameCount,1));
	}
};



class CTimer {
private:
	chrono::time_point<hr_clock> beg;
	__int64 msCount = -1;
	string	m_Name{ NULL };
public:
	explicit CTimer(const string &Name) : m_Name{ Name } {
	}
	// Begin counting
	void Start() {
		beg = hr_clock::now();
	}
	// Stop counting
	void Stop() {
		msCount = chrono::duration_cast<std::chrono::microseconds>(hr_clock::now() - beg).count();
	}
	// Return counter debug info.
	string GetTimerResult() {
		char buff[100];
		snprintf(buff, sizeof(buff), "%.3f", (msCount / 1000.0));

		return m_Name +": " + string(buff)+" ms";
	}
};

class CDebug
{
public:
	explicit CDebug(const std::string &acFileName) :
		pLogStream { new ofstream(acFileName) },
		m_fileName{ acFileName }
	{
	}
	// Prints debug message to log file. logLevel: 0 - errors only, 1 - method logs, 2 - in/out log
	void printMsg(const string &msg, int logLevel) {
		if (gDebugSettings.DebugMessaging && logLevel<= gDebugSettings.DebugLevel) {
			if (pLogStream) {
				if (!pLogStream->is_open())
					pLogStream->open(m_fileName, fstream::out | fstream::app);
				*pLogStream << "Debug Message: " << msg << "\n";
				OutputDebugString((msg + "\n").c_str());
				pLogStream->close();
			}
		}
	}

	void printError(const string &msg) {
		if (gDebugSettings.DebugMessaging) {
			if (pLogStream) {
				if (!pLogStream->is_open())
					pLogStream->open(m_fileName, fstream::out | fstream::app);
				*pLogStream << "Debug Error: " << msg << "\n";
				pLogStream->close();
			}
			OutputDebugString(msg.c_str());
		}
		throw std::runtime_error(msg);
	}

	void SetD3DName(ID3D11DeviceChild* dc, const std::string& c_szName) {
		dc->SetPrivateData(WKPDID_D3DDebugObjectName, c_szName.length(), c_szName.data());
	}

	~CDebug()
	{
		delete pLogStream;
	}
private:
	ofstream* pLogStream = nullptr;
	string m_fileName{NULL};
};

extern CDebug* g_pDebug;
#endif // !DEBUG_H


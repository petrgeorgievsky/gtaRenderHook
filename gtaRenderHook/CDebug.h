#ifndef DEBUG_H
#define DEBUG_H
using namespace std;
class CDebug
{
public:
	CDebug(std::string acFileName) :m_fileName{ acFileName }, pLogStream { new ofstream(acFileName) }
	{}
	void printMsg(const string &msg) {
		if (pLogStream) {
			if(!pLogStream->is_open())
				pLogStream->open(m_fileName, fstream::out | fstream::app);
			*pLogStream << "Debug Message: " << msg << "\n";
			OutputDebugString(msg.c_str());
			pLogStream->close();
		}
	}
	void printError(const string &msg) {
		if (pLogStream) {
			if (!pLogStream->is_open())
				pLogStream->open(m_fileName, fstream::out | fstream::app);
			*pLogStream << "Debug Error: " << msg << "\n"; 
			pLogStream->close();
		}
		OutputDebugString(msg.c_str());
		throw std::runtime_error(msg);
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


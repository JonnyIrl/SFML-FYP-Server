#include "ExtraIncludes.h"
#include <string>
using namespace std;

//ClientS will use Client to get IP Address's etc. 
class Client
{
public:
	Client(const string &IP, const string &Port, bool ready);
	sf::IpAddress GetIPAddress(){ return m_IPAddress; }
	string GetName() { return m_Name; }
	bool GetConnected() { return m_connected; }
	void SetConnected(bool state) { m_connected = state; }
	bool GetReady() { return m_ready; }
	void SetReady(bool state) { m_ready = state; }

private:
	sf::IpAddress m_IPAddress;
	string m_Name;
	bool m_connected;
	bool m_ready;
};

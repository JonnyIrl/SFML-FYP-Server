#include "Client.h"

Client::Client(const string &IP, const string &Name, bool ready)
{
	m_IPAddress = IP;
	m_Name = Name;
	m_ready = ready;
}
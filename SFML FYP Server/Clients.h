#include "Client.h"
#include <vector>
using namespace std;

class Clients
{
public:
	Clients();
	void AddNewClient(const string &IP, const string &Name);
	vector<sf::IpAddress> BroadcastToAllClients();
	vector<sf::IpAddress> BroadcastToEveryoneExceptClient(const string &IPAddress);
	sf::IpAddress FindWhoSentMessage(string &IP);
	bool CheckIfClientExists(const string &IPAddress);
	bool CheckIfClientIsReady(const string &IP);
	vector<pair<sf::IpAddress, string>> TellNewConnectAboutOtherPlayers(const string &IP);
	int ClientsSize() { return clientList.size(); }
	vector<Client*> GetClients() { return clientList; }
	int GetClientIndex(const string &ip);

private:
	vector<Client*> clientList;
};

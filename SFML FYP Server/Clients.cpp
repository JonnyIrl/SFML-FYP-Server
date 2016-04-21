#include "Clients.h"

Clients::Clients()
{

}

void Clients::AddNewClient(const string &IP, const string &Name)
{
	clientList.push_back(new Client(IP, Name, false));
}

vector<sf::IpAddress> Clients::BroadcastToAllClients()
{
	vector<sf::IpAddress> IPs;
	for (int i = 0; i < clientList.size(); i++)
	{
		IPs.push_back(clientList.at(i)->GetIPAddress());
	}

	return IPs;
}

int Clients::GetClientIndex(const string &IP)
{
	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IP) != string::npos)
		{
			return i;
		}
	}
}

vector<pair<sf::IpAddress, string>> Clients::TellNewConnectAboutOtherPlayers(const string &IP)
{
	vector<pair<sf::IpAddress, string>> result;

	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IP) != string::npos)
		{
			
		}
		else
		{
			pair<sf::IpAddress, string> pairResult;
			pairResult.first = clientList.at(i)->GetIPAddress();
			pairResult.second = clientList.at(i)->GetName();
			result.push_back(pairResult);
			cout << "CLIENT TO SEND IP = " << clientList.at(i)->GetIPAddress().toString() << "AND NAME = " << clientList.at(i)->GetName() << endl;
			//return result;
		}

	}
	return result;
}

vector<sf::IpAddress> Clients::BroadcastToEveryoneExceptClient(const string &IPAddress)
{
	vector<sf::IpAddress> IPs;
	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IPAddress) != string::npos)
		{
			
		}

		else
		{
			IPs.push_back(clientList.at(i)->GetIPAddress());
			//cout << "PUSHED BACK IP = " << clientList.at(i)->GetIPAddress().toString() << endl;
		}
	}

	return IPs;
}

bool Clients::CheckIfClientExists(const string &IPAddress)
{
	bool found = false;
	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IPAddress) != string::npos)
		{
			found = true;
		}
	}

	return found;
}

bool Clients::CheckIfClientIsReady(const string &IP)
{
	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IP) != string::npos)
		{
			if (clientList.at(i)->GetReady())
			{
				return true;
			}

			else 
				return false;
		}
	}

	return false;
}

sf::IpAddress Clients::FindWhoSentMessage(string &IP)
{
	for (int i = 0; i < clientList.size(); i++)
	{
		//Check if the current clients IP is the one listed, if it is then dont add it to the vector
		if (clientList.at(i)->GetIPAddress().toString().find(IP) != string::npos)
		{
			cout << "PERSON WHO SENT MESSAGE IP = " << clientList.at(i)->GetIPAddress().toString();
			return clientList.at(i)->GetIPAddress();
		}
	}
}
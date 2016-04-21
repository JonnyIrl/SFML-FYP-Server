#include "ExtraIncludes.h"
#include "PacketType.h"
#include "Clients.h"
#include <iostream>
#include <list>
#include <vector>
using namespace std;

int CheckHowManyPlayersConnected();
sf::IpAddress ipAddress = "192.168.0.15";
sf::UdpSocket listener;
unsigned short serverPort = 5300;
unsigned short sendingPort = 5301;

vector<float> xPositions, yPositions, lootTypes;


list<string> historyMessages;
sf::SocketSelector selector;
void BeginListening();
void ReceivePacket();

void SendPacket(string msg, PacketType type, string IP);
void SendInitalConnectData(PacketType type, sf::IpAddress address);
void SendPositionPacket(PacketType type, float xPos, float yPos, int direction, string playerID, sf::IpAddress address);
void SendRespawnPacket(PacketType type, float xPos, float yPos, int direction, string playerID, sf::IpAddress address);
void SendRoomUpdatePacket(PacketType type, int room, string playerID, sf::IpAddress address);
void SendKillConfirmedPacket(PacketType type, string playerID, sf::IpAddress address);
void SendBulletPacket(PacketType type, float xPos, float yPos, int direction, string playerID, sf::IpAddress address);
bool running = true;
bool connected = false;
string message, name;
bool replied = false;
int messagesCount = 0;
string ipFromPacket, nameFromPacket;
unsigned short portFromPacket;
Clients clients = Clients();
void TestSendPacket(PacketType type, string &IP);
void SendChatPacket(string msg, PacketType type);
void SendNewClientDetails(string name, string ip, bool ready, PacketType type, sf::IpAddress address);
void SendReadyChanged(string ip, bool ready, PacketType type, sf::IpAddress address);
void SendChestUpdate(PacketType type, sf::IpAddress address);
void SendChestOpenUpdate(PacketType type, int index, sf::IpAddress address);
void SendChestItemTakenUpdate(PacketType type, int index, sf::IpAddress address);
void SendGameOver(PacketType type, sf::IpAddress address);
//New Stuff

bool gameStarted = false;
sf::Clock gameTime;
int gameRunTime = 120;
bool forceQuit = false;

void StartClock()
{
	while (!forceQuit)
	{

		if (gameStarted == true)
		{
			int timer = gameTime.getElapsedTime().asSeconds();
			if (timer > 0)
			{
				gameRunTime--;

				cout << "Count Down = " << gameRunTime << endl;

				if (gameRunTime <= 0)
				{
					//GAME OVER
					vector<sf::IpAddress> clientsIps = clients.BroadcastToAllClients();
					for (int i = 0; i < clientsIps.size(); i++)
					{
						SendGameOver(GAME_OVER_MESSAGE, clientsIps.at(i));
					}

					forceQuit = true;
				}

				gameTime.restart();
			}
		}
	}
}

//Chest stuff
int m_amountOfRandoms = 18;
int minX = 200; int maxX = 1000;
int minY = 150; int maxY = 600;

void GenerateRoom1Loot();
void GenerateRoom2Loot();
void GenerateRoom3Loot();
void GenerateRoom4Loot();
void GenerateRoom5Loot();
void GenerateRoom6Loot();
void GenerateRoom7Loot();
void GenerateRoom8Loot();
void GenerateRoom9Loot();
void GenerateAllRoomsLoot();


int main()
{

	sf::Thread clockThread(&StartClock);

	if (listener.bind(serverPort) == sf::Socket::Done)
	{
		GenerateAllRoomsLoot();
		cout << "Generated All Chests" << endl;
		cout << "Socket bound" << endl;
		connected = true;
	}

	clockThread.launch();


	while (running)
	{
		ReceivePacket();
	}

	return 0;
}

void ReceivePacket()
{

	sf::Packet packet;
	//cout << "Before Receive IP = " << ipAddress << endl;
	sf::Socket::Status status = listener.receive(packet, ipAddress, serverPort);
	//cout << "After Receive IP = " << ipAddress << endl;
	string clientName;
	string clientIP;
	string testName;
	string testMessage;
	unsigned short clientPort;

	switch (status)
	{
	case sf::Socket::Done:
		PacketType type;
		packet >> type;// >> clientIP >> clientName >> clientPort;

		//INITIAL CONNECTION RESPONSE 
		if (type == INITIAL_CONNECT_DATA)
		{
			string playerID;
			packet >> clientIP >> playerID;
			cout << "Client IP = " << clientIP << " PlayerID = " << playerID << endl;

			sf::IpAddress addy = clientIP;
			SendInitalConnectData(INITIAL_CONNECT_DATA, addy);

			//Send Chest Data
			SendChestUpdate(CHEST_MESSAGE, addy);

			//If the clients doesnt exist then add them..
			if (!clients.CheckIfClientExists(clientIP))
			{
				clients.AddNewClient(clientIP, playerID);		
				if (clients.ClientsSize() > 1)
				{
					vector<pair<sf::IpAddress, string>> result = clients.TellNewConnectAboutOtherPlayers(clientIP);
					for (int i = 0; i < result.size(); i++)
					{
						bool ready = clients.GetClients().at(clients.GetClientIndex(result.at(i).first.toString()))->GetReady();
						SendNewClientDetails(result.at(i).second, result.at(i).first.toString(), ready, NEW_PLAYER_CONNECTED, clients.FindWhoSentMessage(clientIP));
					}
				}


				TestSendPacket(type, clientIP);
								
				vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
				for (int i = 0; i < clientsIps.size(); i++)
				{
					bool ready = clients.GetClients().at(clients.GetClientIndex(clientsIps.at(i).toString()))->GetReady();
					SendNewClientDetails(playerID, clientIP,ready, NEW_PLAYER_CONNECTED, clientsIps.at(i));
					cout << "Sent other player info to " << clientsIps.at(i).toString() << "From new player = " << clientIP << endl;
				}
			}

			//Else the client already exists, so tell them, they reconnected.
			else
			{
				vector<pair<sf::IpAddress, string>> result = clients.TellNewConnectAboutOtherPlayers(clientIP);
				for (int i = 0; i < result.size(); i++)
				{
					bool ready = clients.GetClients().at(clients.GetClientIndex(result.at(i).first.toString()))->GetReady();
					SendNewClientDetails(result.at(i).second, result.at(i).first.toString(), ready, NEW_PLAYER_CONNECTED, clients.FindWhoSentMessage(clientIP));
				}

				TestSendPacket(type, clientIP);

			}

			packet.clear();
			cout << "PACKET CLEARED" << endl;
			break;
		}

		else if (type == GENERAL_MSG)
		{
			cout << "Got chat message" << endl;
			packet >> clientIP >> testMessage >> testName;
			std::cout << "IP = " << clientIP << endl << "Testname = " << testName << endl << "Message = " << testMessage << endl;
			historyMessages.push_back(testName + ": " + testMessage);
			SendChatPacket(testMessage, type);			
			++messagesCount;
			cout << "MESSAGE COUNT = " << messagesCount << endl;
			break;			
		}

		else if (type == GAME_STARTED_MESSAGE)
		{
			gameStarted = true;
			break;
		}

		else if (type == PLAYER_POSITION_UPDATE)
		{
			float xPos, yPos;
			int direction;
			packet >>  clientIP >> xPos >> yPos >> direction;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendPositionPacket(PLAYER_POSITION_UPDATE, xPos, yPos, direction, clientIP, clientsIps.at(i));
			}

		}

		else if (type == PLAYER_READY_CHANGED)
		{
			string IP;
			bool ready;

			packet >> IP >> ready;

			for (int i = 0; i < clients.GetClients().size(); i++)
			{
				if (clients.GetClients().at(i)->GetIPAddress() == clients.FindWhoSentMessage(IP))
				{
					clients.GetClients().at(i)->SetReady(ready);
				}
			}
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(IP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendReadyChanged(IP, ready, PLAYER_READY_CHANGED, clientsIps.at(i));
				cout << "Sent other player info to " << clientsIps.at(i).toString() << "From READY NEW player = " << clientIP << endl;
			}

		}

		else if (type == BULLET_MESSAGE)
		{
			float xPos, yPos;
			int gun;
			packet >> clientIP >> xPos >> yPos >> gun;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendBulletPacket(BULLET_MESSAGE, xPos, yPos, gun, clientIP, clientsIps.at(i));
			}

		}

		else if (type == RESPAWN_MESSAGE)
		{
			float xPos, yPos;
			int room;
			packet >> clientIP >> xPos >> yPos >> room;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendRespawnPacket(RESPAWN_MESSAGE, xPos, yPos, room, clientIP, clientsIps.at(i));
			}

		}

		else if (type == CHEST_OPEN_MESSAGE)
		{
			int index;
			packet >> clientIP >> index;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendChestOpenUpdate(CHEST_OPEN_MESSAGE, index, clientsIps.at(i));
			}

		}

		else if (type == CHEST_ITEM_TAKEN_MESSAGE)
		{
			int index;
			packet >> clientIP >> index;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendChestItemTakenUpdate(CHEST_ITEM_TAKEN_MESSAGE, index, clientsIps.at(i));
			}

		}

		else if (type == ROOM_UPDATE)
		{
			int room;
			packet >> clientIP >> room;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToEveryoneExceptClient(clientIP);
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendRoomUpdatePacket(ROOM_UPDATE, room, clientIP, clientsIps.at(i));
			}

		}

		else if (type == KILL_CONFIRMED)
		{
			string playerWhoKilledWho;
			packet >> playerWhoKilledWho;
			vector<sf::IpAddress> clientsIps = clients.BroadcastToAllClients();
			for (int i = 0; i < clientsIps.size(); i++)
			{
				SendKillConfirmedPacket(KILL_CONFIRMED, playerWhoKilledWho, clientsIps.at(i));
			}

		}

		break;


	case sf::Socket::Disconnected:
		std::cout << " has been disconnected\n";
		break;

	default:
		replied = false;
		;
	}

}

void SendPositionPacket(PacketType type, float xPos, float yPos, int direction, string playerID, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == PLAYER_POSITION_UPDATE)
	{
		//Put all the information into the packet..
		packet << type << playerID << xPos << yPos << direction;
		
		//cout << "Before Sending - Direction = " << direction << endl;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			//cout << "Sent Position Data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void SendRespawnPacket(PacketType type, float xPos, float yPos, int room, string playerID, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == RESPAWN_MESSAGE)
	{
		//Put all the information into the packet..
		packet << type << playerID << xPos << yPos << room;

		cout << "Before Sending - room = " << room << endl;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Sent Position Data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void SendKillConfirmedPacket(PacketType type, string playerID, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == KILL_CONFIRMED)
	{
		//Put all the information into the packet..
		packet << type << playerID;

		cout << "KILL CONFIRMED = " << playerID << endl;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Sent KILL CONFIRMED data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void SendInitalConnectData(PacketType type, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == INITIAL_CONNECT_DATA)
	{
		//Put all the information into the packet..
		packet << type;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Sent INITIAL CONNECT data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void SendRoomUpdatePacket(PacketType type, int room, string playerID, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == ROOM_UPDATE)
	{
		//Put all the information into the packet..
		packet << type << playerID << room;

		cout << "Before Sending - room = " << room << endl;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Sent Room Update Data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void SendBulletPacket(PacketType type, float xPos, float yPos, int gun, string playerID, sf::IpAddress address)
{
	sf::Packet packet;
	//*****************************************************************************************************************
	if (type == BULLET_MESSAGE)
	{
		//Put all the information into the packet..
		packet << type << playerID << xPos << yPos << gun;

		cout << "Before Sending - Gun = " << gun << endl;
		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Sent Bullet Data!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
}

void TestSendPacket(PacketType type, string &IP)
{
	sf::Packet packet;
	//*****************************************************************************************************************

	//If it a joining connect data
	if (type == INITIAL_CONNECT_DATA)
	{
		string name;
		//sf::Packet packet;

		if (packet.getDataSize() > 0)
		{
			packet.clear();
		}
		//Put server reply msg in the packet
		packet << SERVER_REPLY_MSG;


		sf::Socket::Status status = listener.send(packet, clients.FindWhoSentMessage(IP), sendingPort);
		cout << "Sending Connect message to " << clients.FindWhoSentMessage(IP).toString() <<  endl;
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "Connect message sent successfully!" << endl << endl;
			replied = true;
		}
		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}

	//*****************************************************************************************************************
	else if (type == GENERAL_MSG)
	{
		//Sending a list through a packet.
		//Send the size and then each item through as a single string... Might cause problems when there is more than 10 messages etc...
		if (messagesCount >= 11)
		{
			//Dont let the messages overflow..
			historyMessages.pop_front();
		}

		cout << endl << endl << "Inside the send chat message" << endl;
		cout << "Packing up the packet.. historyMessages.size() = " << historyMessages.size() << endl;
		if (packet.getDataSize() > 0)
		{
			cout << "Clearing Packet" << endl;
			packet.clear();
		}

		packet << UPDATE_MSG << static_cast<sf::Uint32>(historyMessages.size());

		for (std::list<string>::const_iterator it = historyMessages.begin(); it != historyMessages.end(); ++it)
		{
			packet << *it;
		}


		vector<sf::IpAddress> clientsIps = clients.BroadcastToAllClients();
		for (int i = 0; i < clientsIps.size(); i++)
		{
			cout << "Inside clientIP for loop.." << endl;
			cout << "IP = " << clientsIps.at(i).toString() << endl << "Packet Size = " << packet.getDataSize() << endl;
			sf::Socket::Status status = listener.send(packet, clientsIps.at(i), serverPort);
			switch (status)
			{
			case sf::Socket::Done:
			{
				cout << "Packet " << i << endl;
				cout << "Sent to -- " << clientsIps.at(i).toString() << endl;
			}

			break;
			case sf::Socket::Disconnected:
				cout << "Player has been disconnected inside the sending chat message" << endl;
				std::cout << " has been disconnected\n";
				break;

			default:
				;
			}
		}
			++messagesCount;
			cout << "MESSAGE COUNT = " << messagesCount << endl;
	}
}

void SendPacket(string msg, PacketType type, string IP)
{
	bool testReply = false;
	if (!replied)
	{
		sf::Packet packet;
		//*****************************************************************************************************************
		if (type == INITIAL_CONNECT_DATA)
		{
			string name;
			sf::Packet packet;
			packet << SERVER_REPLY_MSG;


			sf::Socket::Status status = listener.send(packet, ipAddress, sendingPort);
			switch (status)
			{
			case sf::Socket::Done:
			{
				cout << "Client connected!" << endl << endl;
				replied = true;
			}
			break;
			case sf::Socket::Disconnected:
				std::cout << " has been disconnected\n";
				break;

			default:
				;
			}
		}

		//*****************************************************************************************************************
		else if (type == GENERAL_MSG)
		{
			//Sending a list through a packet.
			//Send the size and then each item through as a single string... Might cause problems when there is more than 10 messages etc...
			if (messagesCount >= 11)
			{
				//Dont let the messages overflow..
				historyMessages.pop_front();
			}

			cout << endl << endl << "Inside the send chat message" << endl;
			cout << "Packing up the packet.. historyMessages.size() = " << historyMessages.size() << endl;
			packet << UPDATE_MSG << static_cast<sf::Uint32>(historyMessages.size());

			for (std::list<string>::const_iterator it = historyMessages.begin(); it != historyMessages.end(); ++it)
			{
				packet << *it;
			}
			

			vector<sf::IpAddress> clientsIps = clients.BroadcastToAllClients();
			for (int i = 0; i < clientsIps.size(); i++)
			{								
				cout << "Inside clientIP for loop.." << endl;
				cout << "IP = " << clientsIps.at(i).toString() << endl << "Packet Size = " << packet.getDataSize() << endl;
				sf::Socket::Status status = listener.send(packet, clientsIps.at(i), sendingPort);
				switch (status)
				{
				case sf::Socket::Done:
				{	
					cout << "Packet " << i << endl;
					cout << "Sent to -- " << clientsIps.at(i) << endl;
					testReply = true;
				}

				break;
				case sf::Socket::Disconnected:
					cout << "Player has been disconnected inside the sending chat message" << endl;
					std::cout << " has been disconnected\n";
					break;

				default:
					;
				}
			}


			if (testReply == true)
			{
				replied = true;
				testReply = false;
			}

			if (replied)
			{
				++messagesCount;
				cout << "MESSAGE COUNT = " << messagesCount << endl;
			}
		}
	}
}

void SendChatPacket(string msg, PacketType type)
{
	sf::Packet packet;

	if (type == GENERAL_MSG)
	{
		//Sending a list through a packet.
		//Send the size and then each item through as a single string... Might cause problems when there is more than 10 messages etc...
		if (messagesCount >= 11)
		{
			//Dont let the messages overflow..
			historyMessages.pop_front();
		}

		cout << endl << endl << "Inside the send chat message" << endl;
		cout << "Packing up the packet.. historyMessages.size() = " << historyMessages.size() << endl;
		packet << UPDATE_MSG << static_cast<sf::Uint32>(historyMessages.size());

		for (std::list<string>::const_iterator it = historyMessages.begin(); it != historyMessages.end(); ++it)
		{
			packet << *it;
		}


		vector<sf::IpAddress> clientsIps = clients.BroadcastToAllClients();
		for (int i = 0; i < clientsIps.size(); i++)
		{
			cout << "Inside clientIP for loop.." << endl;
			cout << "IP = " << clientsIps.at(i).toString() << endl << "Packet Size = " << packet.getDataSize() << endl;
			sf::Socket::Status status = listener.send(packet, clientsIps.at(i), sendingPort);
			switch (status)
			{
			case sf::Socket::Done:
			{
				cout << "DONE" << endl;
				cout << "Sent to -- " << clientsIps.at(i).toString() << endl;
				//packet.clear();
				//cout << "Cleared Packet after sending packet!" << endl;
			}

			break;
			case sf::Socket::Disconnected:
				cout << "Player has been disconnected inside the sending chat message" << endl;
				std::cout << " has been disconnected\n";
				break;

			default:
				;
			}
		}
	}
}

void SendNewClientDetails(string name, string ip, bool ready, PacketType type, sf::IpAddress address)
{
	sf::Packet packet;

	if (type == NEW_PLAYER_CONNECTED)
	{
		//Put the header, and then the message which will contain the players IP and Name
		packet << NEW_PLAYER_CONNECTED << name << ip << ready;

		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "DONE" << endl;
		}

		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}
	
}

void SendReadyChanged(string ip, bool ready, PacketType type, sf::IpAddress address)
{
	sf::Packet packet;

	if (type == PLAYER_READY_CHANGED)
	{
		//Put the header, and then the message which will contain the players IP and Name
		packet << PLAYER_READY_CHANGED << ip << ready;

		sf::Socket::Status status = listener.send(packet, address, sendingPort);
		switch (status)
		{
		case sf::Socket::Done:
		{
			cout << "SENT READY MESSAGE" << endl;
		}

		break;
		case sf::Socket::Disconnected:
			std::cout << " has been disconnected\n";
			break;

		default:
			;
		}
	}

}

void SendChestUpdate(PacketType type, sf::IpAddress address)
{
	sf::Packet packet;

	//Put the header and then the size of the vector
	packet << CHEST_MESSAGE << static_cast<sf::Uint32>(xPositions.size());

	for (vector<float>::const_iterator it = xPositions.begin(); it != xPositions.end(); ++it)
	{
		packet << *it;
	}

	//Put the size of the next vector
	packet << static_cast<sf::Uint32>(yPositions.size());

	for (vector<float>::const_iterator it = yPositions.begin(); it != yPositions.end(); ++it)
	{
		packet << *it;
	}

	//Put the size of the next vector
	packet << static_cast<sf::Uint32>(lootTypes.size());

	for (vector<float>::const_iterator it = lootTypes.begin(); it != lootTypes.end(); ++it)
	{
		packet << *it;
	}

	sf::Socket::Status status = listener.send(packet, address, sendingPort);
	switch (status)
	{
	case sf::Socket::Done:
	{
		cout << "SENT CHEST_MESSAGE " << endl;
	}

	break;
	case sf::Socket::Disconnected:
		std::cout << " has been disconnected\n";
		break;

	default:
		;
	}


}

void SendChestOpenUpdate(PacketType type, int index, sf::IpAddress address)
{
	sf::Packet packet;

	//Put the header, and then the message which will contain the players IP and Name
	packet << CHEST_OPEN_MESSAGE << index;

	sf::Socket::Status status = listener.send(packet, address, sendingPort);
	switch (status)
	{
	case sf::Socket::Done:
	{
		cout << "SENT CHEST_OPEN_MESSAGE MESSAGE" << endl;
	}

	break;
	case sf::Socket::Disconnected:
		std::cout << " has been disconnected\n";
		break;

	default:
		;
	}

}

void SendGameOver(PacketType type, sf::IpAddress address)
{
	sf::Packet packet;

	//Put the header, and then the message which will contain the players IP and Name
	packet << GAME_OVER_MESSAGE;

	sf::Socket::Status status = listener.send(packet, address, sendingPort);
	switch (status)
	{
	case sf::Socket::Done:
	{
		cout << "SENT GAME_OVER_MESSAGE" << endl;
	}

	break;
	case sf::Socket::Disconnected:
		std::cout << " has been disconnected\n";
		break;

	default:
		;
	}

}

void SendChestItemTakenUpdate(PacketType type, int index, sf::IpAddress address)
{
	sf::Packet packet;

	//Put the header, and then the message which will contain the players IP and Name
	packet << CHEST_ITEM_TAKEN_MESSAGE << index;

	sf::Socket::Status status = listener.send(packet, address, sendingPort);
	switch (status)
	{
	case sf::Socket::Done:
	{
		cout << "SENT CHEST_ITEM_TAKEN_MESSAGE " << endl;
	}

	break;
	case sf::Socket::Disconnected:
		std::cout << " has been disconnected\n";
		break;

	default:
		;
	}

}

void GenerateAllRoomsLoot()
{
	srand(time(NULL));
	GenerateRoom1Loot();
	GenerateRoom2Loot();
	GenerateRoom3Loot();
	GenerateRoom4Loot();
	GenerateRoom5Loot();
	GenerateRoom6Loot();
	GenerateRoom7Loot();
	GenerateRoom8Loot();
	GenerateRoom9Loot();
}
void GenerateRoom1Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;

		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom2Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom3Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom4Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom5Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom6Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom7Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom8Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}
void GenerateRoom9Loot()
{
	for (int i = 0; i < (m_amountOfRandoms / 9); i++)
	{
		int xPos = rand() % (maxX - minX) + minX;
		int yPos = rand() % (maxY - minY) + minY;
		int loot = rand() % 10 + 1;
		xPositions.push_back(xPos);
		yPositions.push_back(yPos);
		lootTypes.push_back(loot);
	}
}


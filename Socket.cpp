
// Socket.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include <afxsock.h>
#include "Socket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MAX 1000
#define SERVER_PORT 8080

// The one and only application object

CWinApp theapp;

using namespace std;

struct RequestLine
{
	string method, filename, versionHTTP;
};

struct RequestBody {
	string uname, psw, temp;
};

struct Request
{
	RequestLine headerLine;  //chua method, filename, version HTTP
	RequestBody data; // chua username va password;
	char request[MAX];// = new char[MAX];
};


struct Chunk
{
	unsigned int size;
	unsigned char identifier[4];
	char* contentData;
	unsigned int CRC;

};

struct PNG
{
	unsigned char header[8];
	Chunk chunk[3];

};

//using RestSharp;
//using RestSharp.Authenticators;

void ParseRequest(Request& buffer)
{
	cout << "Data nhan duoc tu client: " << endl << buffer.request << endl;
	stringstream s(buffer.request);
	string lineInfo;
	getline(s, lineInfo, '\r');
	stringstream ss(lineInfo);
	getline(ss, buffer.headerLine.method, ' ');  //GET hoac POST
	if (buffer.headerLine.method == "GET") {
		getline(ss, buffer.headerLine.filename, '/');// bo dau '/'
		getline(ss, buffer.headerLine.filename, ' '); //Lay ten file
		getline(ss, buffer.headerLine.versionHTTP);  // HTTP/1.1
	}
	else if (buffer.headerLine.method == "POST") {
		string header;
		getline(s, header, '='); // bo phan request header

		string body;
		getline(s, body);
		stringstream sss(body);
		getline(sss, buffer.data.uname, '&'); //username
		getline(sss, buffer.data.temp, '=');
		getline(sss, buffer.data.psw); // password
	}
}

int GettextData(Request buffer, char*& data)
{
	int length = 0;
	//Lấy dữ liệu từ filename để response lại cho browser
	if (buffer.headerLine.filename == "")
	{
		buffer.headerLine.filename = "images.html";
	}
	//Lấy dữ liệu từ file images.html để response lại cho browser
	ifstream fin(buffer.headerLine.filename);
	if (!fin)
		cout << "\nCan not open this file to read";
	else
	{
		fin.seekg(0, ios::end);
		length = fin.tellg(); //Lay kich thuoc file 
		fin.seekg(0, ios::beg);
		data = new char[length + 1];
		data[length] = '\0';
		fin.read(data, length);
		fin.close();
	}
	return length;
}

void check(Request& buffer, char*& data, CSocket& Connector) {
	int length = 0;
	if (buffer.data.uname == "admin" && buffer.data.psw == "123456") {
		GettextData(buffer, data);
	}
	else {
		char headerLine[] = "HTTP / 1.1 401 Unauthorized\r\nContent - Type: text / html\r\nConnection: close\r\n\r\n<!DOCTYPE html><h1>401 Unauthorized< / h1><p>This is a private area.< / p>";
		Connector.Send(&headerLine, sizeof(headerLine), 0);
		buffer.headerLine.filename = "404.html";
		length = GettextData(buffer, data);
		Connector.Send(data, length + 1, 0);
	}
}


int GetTextData(Request buffer, char*& data)
{
	int length = 0;
	//Lấy dữ liệu từ filename để response lại cho browser
	if (buffer.headerLine.filename == "")
	{
		buffer.headerLine.filename = "index.html";
	}
	//Lấy dữ liệu từ file index.html để response lại cho browser
	ifstream fin(buffer.headerLine.filename);
	if (!fin)
		cout << "\nNot open this file to read";
	else
	{
		fin.seekg(0, ios::end);
		length = fin.tellg(); //Lay kich thuoc file 
		fin.seekg(0, ios::beg);
		data = new char[length + 1];
		data[length] = '\0';
		fin.read(data, length);
		fin.close();
	}
	return length;
}

int GetImageData(Request buffer, char*& data)
{
	int length = 0;
	//Lấy dữ liệu từ filename để response lại cho browser
	ifstream fin(buffer.headerLine.filename, ios::binary);
	if (!fin)
		cout << "\nNot open this file to read";
	else
	{
		fin.seekg(0, ios::end);
		length = fin.tellg(); //Lay kich thuoc file 
		fin.seekg(0, ios::beg);
		cout << length << endl;
		data = new char[length + 1];
		data[length] = '\0';
		fin.read(data, length); //read the header
		/*for (int i = 0; i < 3; i++)
		{
			fin.read((char*)&data.chunk[i].size, sizeof(unsigned int));
			unsigned int sz = data.chunk[i].size;
			data.chunk[i].contentData = new char[sz];
			fin.read((char*)&data.chunk[i].identifier, 4);
			fin.read((char*)data.chunk[i].contentData, sz);
			fin.read((char*)&data.chunk[i].CRC, 4);

		}*/

		//cin.get();
		//fin.read(data, length);
		fin.close();
	}
	return length;
}


void SendResponse(CSocket& Connector, Request& buffer)
{
	//Body Content
	if (buffer.headerLine.filename == "" || buffer.headerLine.filename == "favicon.ico")
	{
		buffer.headerLine.filename = "index.html";
	}

	//Lấy dữ liệu từ filename để response lại cho browser
	/*ifstream fin(buffer.headerLine.filename);
	if (!fin)
		cout << "\nNot open this file to read";
	else
	{
		fin.seekg(0, ios::end);
		int length = fin.tellg(); //Lay kich thuoc file
		fin.seekg(0, ios::beg);
		char* data = new char[length + 1];
		data[length] = '\0';
		fin.read(data, length);
		fin.close();
		*/
	char* data = NULL;
	int length = 0;
	//Send headerContent
	ifstream fin(buffer.headerLine.filename);
	if (!fin) {
		char headerLine[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: length + 1\r\n\r\n";
		Connector.Send(&headerLine, sizeof(headerLine), 0);
		buffer.headerLine.filename = "404.html";
		length = GetTextData(buffer, data);
		Connector.Send(data, length + 1, 0);
	}
	else {
		if (strstr(buffer.headerLine.filename.c_str(), "html"))
		{
			char headerLine[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: length + 1\r\n\r\n";
			Connector.Send(&headerLine, sizeof(headerLine), 0);
			length = GetTextData(buffer, data);
			Connector.Send(data, length + 1, 0);
		}
		else
		{
			if (strstr(buffer.headerLine.filename.c_str(), "css"))
			{
				cout << buffer.headerLine.filename << endl;
				char headerLine[] = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nConnection: close\r\nContent-Length: length + 1\r\n\r\n";
				Connector.Send(&headerLine, sizeof(headerLine), 0);
				length = GetTextData(buffer, data);
				Connector.Send(data, length + 1, 0);
			}
			if (strstr(buffer.headerLine.filename.c_str(), "png"))
			{
				//PNG dat;
				char headerLine[] = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nConnection: close\r\nContent-Length: length + 1\r\n\r\n";
				Connector.Send(&headerLine, sizeof(headerLine), 0);
				length = GetImageData(buffer, data);
				Connector.Send(data, length + 1, 0);

			}
		}
	}
	// Send noi dung bodyContent: Lấy dữ liệu từ filename để response lại cho browser
	//Connector.Send(data, length + 1, 0); 
	Connector.Close();
	fin.close();
	cout << "Data dc gui di: " << endl << data << endl << endl;

	// can filename
}

DWORD WINAPI function_cal(LPVOID arg)
{
	SOCKET* hConnected = (SOCKET*)arg;
	CSocket mysock;
	//Chuyen ve lai CSocket
	mysock.Attach(*hConnected);


	//	do {
			//Thao tac o day

			//vector<Request> ServerMsg;
	Request buffer;
	memset(buffer.request, 0, MAX);
	char* data;
	if (mysock.Receive(buffer.request, sizeof(buffer.request), 0))
	{
		ParseRequest(buffer);
		if (buffer.headerLine.method == "GET") {
			SendResponse(mysock, buffer);
		}
		else if (buffer.headerLine.method == "POST") {
			check(buffer, data, mysock);
		}
		//ServerMsg.push_back(buffer);
	}
	//int numberRequest = ServerMsg.size();
	//cout << endl << "Slg request: " << numberRequest << endl << endl;				
	//else break;

//	} while (1);
	delete hConnected;
	return 0;
	//return 0;
}
int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: code your application's behavior here.
			wprintf(L"Fatal Error: MFC initialization failed\n");
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			// Khoi tao thu vien Socket
			if (AfxSocketInit() == FALSE)
			{
				cout << "Khong the khoi tao Socket Libraray";
				return FALSE;
			}

			CSocket ServerSocket; //cha  // Socket cha chỉ có làm create socket, listen, accept và close socket, sau đó sẽ tạo socket con để duy trì việc giao tiếp với 1 client
			// Tao socket cho server, dang ky port la 8080, giao thuc TCP
			if (ServerSocket.Create(SERVER_PORT, SOCK_STREAM, NULL) == 0) //SOCK_STREAM or SOCK_DGRAM.
			{
				cout << "Khoi tao that bai !!!" << endl;
				cout << ServerSocket.GetLastError();
				return FALSE;
			}
			else
			{
				cout << "Server khoi tao thanh cong !!!" << endl;

				if (ServerSocket.Listen(0) == FALSE)
				{
					cout << "Khong the lang nghe tren port nay !!!" << endl;
					ServerSocket.Close();
					return FALSE;
				}
			}
			CSocket Connector;
			DWORD threadID;
			HANDLE threadStatus;
			do
			{
				cout << "Cho ket noi tu Client" << endl;

				//Khoi tao mot socket de duy tri viec ket noi va trao doi du lieu
				if (ServerSocket.Accept(Connector))
				{
					cout << "Da co Client ket noi !!!" << endl << endl;
					SOCKET* hConnected = new SOCKET();

					//Chuyển đỏi CSocket thanh Socket
					*hConnected = Connector.Detach();

					//Khoi tao thread tuong ung voi moi client Connect vao server.
				  //Nhu vay moi client se doc lap nhau, khong phai cho doi tung client xu ly rieng
					threadStatus = CreateThread(NULL, 0, function_cal, hConnected, 0, &threadID);



				}

			} while (1);
			ServerSocket.Close();
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}

	return nRetCode;
}
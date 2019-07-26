#ifndef _FTPCLASS_H
#define _FTPCLASS_H

#include <string>
#ifdef LINUX
#include <sys/socket.h>
#else
#include <WinSock2.h>
#endif

using namespace std;

class CFTPclient
{
public:
	CFTPclient();
	~CFTPclient();

	bool MoveFile(string remotefile, string localfile,bool pasv,bool get);
	void LogOffServer();
	bool LogOnToServer(string hostname,int hostport,string username, string password);

	bool FTPcommand(string command);

	bool UploadFile(string remotefile, string localfile, bool pasv);
	bool DownloadFile(string remotefile, string localfile, bool pasv);

	//同步时间
	bool SyncTime(string command);

	string m_retmsg;

private:
	/*对应的文件描述符是否可读，返回0可读，返回<0不可读，同时设置errno*/
	int CanRead(int fd, int timeoutsec, int timeoutusec = 0);
	/*对应的文件描述符是否可写，返回0可写，返回<0不可写，同时设置errno*/
	int CanWrite(int fd, int timeoutsec, int timeoutusec = 0);

	int 	m_sockControlSock;
	bool 	m_bIsLogin;
};

#endif

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

	//ͬ��ʱ��
	bool SyncTime(string command);

	string m_retmsg;

private:
	/*��Ӧ���ļ��������Ƿ�ɶ�������0�ɶ�������<0���ɶ���ͬʱ����errno*/
	int CanRead(int fd, int timeoutsec, int timeoutusec = 0);
	/*��Ӧ���ļ��������Ƿ��д������0��д������<0����д��ͬʱ����errno*/
	int CanWrite(int fd, int timeoutsec, int timeoutusec = 0);

	int 	m_sockControlSock;
	bool 	m_bIsLogin;
};

#endif

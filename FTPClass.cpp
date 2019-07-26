#include "FTPClass.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ScFileUtils.h"
#include "IVE_SDK_Export.h"
#ifdef LINUX
#include <arpa/inet.h>
#include <sys/select.h>
#else
#pragma comment(lib, "ws2_32.lib")
#endif

#define READ_LEN 1024
#define SEND_LEN 1024
#define FILE_LEN 4096

#ifdef WIN32
	#define CLOSE closesocket
#else
	#define CLOSE close
#endif

CFTPclient::CFTPclient()
{
	m_bIsLogin = false;
	m_sockControlSock = -1;
#ifdef WIN32
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(sockVersion, &wsaData);
#endif
}
CFTPclient::~CFTPclient()
{
#ifdef WIN32
	WSACleanup();
#endif
}

bool CFTPclient::LogOnToServer(string hostname,int hostport,string username, string password)
{
	if(m_bIsLogin)
	{
		LogOffServer();
		m_bIsLogin = false;
	}

	m_sockControlSock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockControlSock == -1)
	{
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������socketʧ��: %s", strerror(errno));
		return false;
	}

	char szSendBuf[SEND_LEN] = {0};
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(hostport);
	server.sin_addr.s_addr = inet_addr(hostname.c_str());

	//���ӵ�������
	int ret = connect(m_sockControlSock, (struct sockaddr*)&server, sizeof(server));
	if(ret != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "���ӵ�������ʧ�ܣ� IP: %s, �˿�: %d", hostname.c_str(), hostport);
		return false;
	}
	if(CanRead(m_sockControlSock, 3) == 0)
	{
		memset(szSendBuf, 0, sizeof(szSendBuf));
		recv(m_sockControlSock, szSendBuf, sizeof(szSendBuf), 0);
		m_retmsg = szSendBuf;
		printf(szSendBuf);
	}
	else
	{
		m_retmsg = "";
		printf("FTP����ʱ\n");
		return false;
	}

	//�����û����������
	snprintf(szSendBuf, sizeof(szSendBuf), "USER %s", username.c_str());
	bool commandRet = FTPcommand(szSendBuf);
	if(!commandRet)
	{
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�������������ʧ�� command: %s", szSendBuf);
		return false;
	}
	if(m_retmsg.compare(0, 3, "331") != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�����û�����������, ����ʧ��: %s, command: %s", m_retmsg.c_str(), szSendBuf);
		return false;
	}

	//�������뵽�����
	snprintf(szSendBuf, sizeof(szSendBuf), "PASS %s", password.c_str());
	commandRet = FTPcommand(szSendBuf);
	if(!commandRet)
	{
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�������������ʧ�� command: %s", szSendBuf);
		return false;
	}
	if(m_retmsg.compare(0, 3, "230") != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�������뵽������, ����ʧ��: %s, command: %s", m_retmsg.c_str(), szSendBuf);
		return false;
	}
	m_bIsLogin = true;

	return true;
}

void CFTPclient::LogOffServer()
{
	if(m_bIsLogin)
	{
		//�Ͽ������˵�����
		string strSend = "QUIT";
		bool commandRet = FTPcommand(strSend);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ͻ���ʧ�� command: %s", strSend.c_str());
			return;
		}
		if(m_retmsg.compare(0, 3, "221") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�Ͽ������˵�����ʧ��: %s", m_retmsg.c_str());
			return;
		}
	}
	CLOSE(m_sockControlSock);
	m_bIsLogin = false;

	return;
}

bool CFTPclient::SyncTime(string command)
{
	printf("command: %s\n", command.c_str());
	command += "\r\n";
	char 	szReadBuf[READ_LEN];
	char 	szSendBuf[SEND_LEN];

	if(CanWrite(m_sockControlSock, 3) == 0)
	{
		snprintf(szSendBuf, sizeof(szSendBuf), "%s", command.c_str());
		send(m_sockControlSock, szSendBuf, strlen(szSendBuf), 0);
	}
	else
	{
		m_retmsg = "";
		printf("FTPд��ʱ command: %s\n", command.c_str());
		return false;
	}

	fd_set rSet;
	FD_ZERO(&rSet);
	FD_SET(m_sockControlSock, &rSet);

	struct timeval sTimeout;
	memset(&sTimeout, 0, sizeof(sTimeout));
	sTimeout.tv_sec = 0;
	sTimeout.tv_usec = 100*1000;

	int ret;
	while(1)
	{
		ret = select(m_sockControlSock + 1, &rSet, NULL, NULL, &sTimeout);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			return false;
		}
		else if(ret == 0)
		{
			errno = ETIMEDOUT;
			return false;
		}
		else if(ret == 1)
			break;
	}
	memset(szReadBuf, 0, sizeof(szReadBuf));
	recv(m_sockControlSock, szReadBuf, sizeof(szReadBuf), 0);
	printf("szReadBuf: %s\n", szReadBuf);
	m_retmsg = szReadBuf;

	return true;
}

//��������
bool CFTPclient::FTPcommand(string command)
{
	m_retmsg = "";
	command += "\r\n";

	char 	szReadBuf[READ_LEN];
	char 	szSendBuf[SEND_LEN];

	// ���m_sockControlSock�Ľ���������
	if (CanRead(m_sockControlSock, 0, 1) == 0)
	{
		memset(szReadBuf, 0, sizeof(szReadBuf));
		recv(m_sockControlSock, szReadBuf, sizeof(szReadBuf), 0);
		printf("��ȡ���Ķ�����Ϣ: %s", szReadBuf);
	}

	if(CanWrite(m_sockControlSock, 3) == 0)
	{
		snprintf(szSendBuf, sizeof(szSendBuf), "%s", command.c_str());
		send(m_sockControlSock, szSendBuf, strlen(szSendBuf), 0);
	}
	else
	{
		m_retmsg = "";
		printf("FTPд��ʱ command: %s\n", command.c_str());
		return false;
	}

	if(CanRead(m_sockControlSock, 3) == 0)
	{
		memset(szReadBuf, 0, sizeof(szReadBuf));
		recv(m_sockControlSock, szReadBuf, sizeof(szReadBuf), 0);
	}
	else
	{
		m_retmsg = "";
		printf("FTP����ʱ command: %s\n", command.c_str());
		return false;
	}

	m_retmsg = szReadBuf;
	printf("���͵�����: %s\n�յ��ķ���: %s\n", command.c_str(), szReadBuf);

	return true;
}

bool CFTPclient::MoveFile(string remotefile, string localfile,bool pasv,bool get)
{
	if(get)
	{
		return DownloadFile(remotefile, localfile, pasv);
	}
	else
	{
		return UploadFile(remotefile, localfile, pasv);
	}
}

bool CFTPclient::UploadFile(string remotefile, string localfile, bool pasv)
{
	if(pasv)
	{
		char szSendBuf[SEND_LEN] = {0};

		//��������ƴ���
		sprintf(szSendBuf, "TYPE I");
		bool commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ͻ���ʧ�� command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "200") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ƴ���ɹ�: %s", m_retmsg.c_str());
			return false;
		}

		//���߷�����ñ���ģʽ
		sprintf(szSendBuf, "PASV");
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ͻ���ʧ�� command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "227") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ñ���ģʽʧ��: %s", m_retmsg.c_str());
			return false;
		}

		/*����������������Ϣת��ΪIP���˿�*/
		int h1,h2,h3,h4,p1,p2;
		sscanf(m_retmsg.c_str(), "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
		char szAddr[16];
		sprintf(szAddr, "%d.%d.%d.%d", h1, h2, h3, h4);
		int port = p1 * 256 + p2;
		printf("IP: %s, PORT: %d\n", szAddr, port);

		//���ӷ�����¿��Ķ˿�
		SOCKET sockDataSock;
		struct sockaddr_in sDataServer;
		sDataServer.sin_family = AF_INET;
		sDataServer.sin_port = htons(port);
		sDataServer.sin_addr.s_addr = inet_addr(szAddr);

		//��ʼ����������socket
		sockDataSock = socket(AF_INET, SOCK_STREAM, 0);
		if(sockDataSock == -1)
		{
			m_retmsg = "";
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������socketʧ��: %s", strerror(errno));
			return false;
		}

		//���ӷ������¿������ݶ˿�
		int ret = connect(sockDataSock, (struct sockaddr *)&sDataServer, sizeof(sDataServer));
		if(ret != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "���ӵ����ݷ���˿�ʧ��: %s IP: %s, �˿�: %d", szSendBuf, szAddr, port);
			CLOSE(sockDataSock);
			return false;
		}

		/*
		//��ȡ�ļ��б�
		sprintf(szSendBuf, "LIST");
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			printf("��������ͻ���ʧ�� command: %s\n", szSendBuf);
			m_pLoger->WriteLog("��������ͻ���ʧ�� command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "150") != 0)
		{
			printf("��ȡ�ļ��б�ʧ��: %s\n", m_retmsg.c_str());
			m_pLoger->WriteLog("��ȡ�ļ��б�ʧ��: %s", m_retmsg.c_str());
			return false;
		}

		char tmpBuf[4096];
		read(sockDataSock, tmpBuf, 4096);
		printf("%s\n", tmpBuf);
		*/


		//�ͻ��˷�������ı�Ŀ¼
		//int pos = remotefile.rfind("/");
		//string strFilePath(remotefile, 0, pos);
//		snprintf(szSendBuf, sizeof(szSendBuf), "CWD /home/testftp/data");
//		commandRet = FTPcommand(szSendBuf);
//		if(!commandRet)
//		{
//			printf("��������ͻ���ʧ�� command: %s\n", szSendBuf);
//			//m_pLoger->WriteLog("��������ͻ���ʧ�� command: %s", szSendBuf);
//			close(sockDataSock);
//			return false;
//		}
//		if(m_retmsg.compare(0, 3, "250") != 0)
//		{
//			printf("�ı�Ŀ¼ʧ��: %s, command: %s\n", m_retmsg.c_str(), szSendBuf);
//			//m_pLoger->WriteLog("�ı�Ŀ¼ʧ��: %s, command: %s", m_retmsg.c_str(), szSendBuf);
//			close(sockDataSock);
//			return false;
//		}

		//�ͻ��˷����ϴ��ļ����� �ϴ���������FTPֻ��Ҫ�ϴ��ļ���
		int pos = remotefile.rfind("/");
		string strFileName(remotefile, pos + 1);
		snprintf(szSendBuf, sizeof(szSendBuf), "STOR %s", strFileName.c_str());
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������ͻ���ʧ�� command: %s", szSendBuf);
			CLOSE(sockDataSock);
			return false;
		}
		if(m_retmsg.compare(0, 3, "150") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�����ϴ��ļ�����ʧ��: %s, command: %s", m_retmsg.c_str(), szSendBuf);
			CLOSE(sockDataSock);
			return false;
		}

		//�ϴ��ļ�
		FILE *fp = fopen(localfile.c_str(), "rb");
		if(fp == NULL)
		{
			m_retmsg = "";
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "���ϴ��ļ�ʧ��: %s�� filename: %s", strerror(errno), localfile.c_str());
			CLOSE(sockDataSock);
			return false;
		}

		char szFileBuf[FILE_LEN];
		while(1)
		{
			int readRet = fread(szFileBuf, 1, sizeof(szFileBuf), fp);
			if(readRet == -1)
			{
				m_retmsg = "";
				IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��ȡ�ļ�ʧ��: %s�� filename: %s", strerror(errno), localfile.c_str());
				fclose(fp);
				CLOSE(sockDataSock);
				return false;
			}
			if(CanWrite(sockDataSock, 3) == 0)
			{
				send(sockDataSock, szFileBuf, readRet, 0);
			}
			else
			{
				m_retmsg = "";
				IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "��������������д: %s�� filename: %s", strerror(errno), localfile.c_str());
				fclose(fp);
				CLOSE(sockDataSock);
				return false;
			}
			if(readRet == 0)
				break;
		}
		fclose(fp);

		//�ͻ��˹ر���������
		CLOSE(sockDataSock);
		memset(szSendBuf, 0, sizeof(szSendBuf));
		if(CanRead(m_sockControlSock, 3) == 0)
		{
			recv(m_sockControlSock, szSendBuf, sizeof(szSendBuf), 0);
		}
		else
		{
			printf("�ر��������Ӻ������������ʧ��\n");
			m_retmsg = "";
			return false;
		}
		printf("%s\n", szSendBuf);

		m_retmsg = szSendBuf;
		if(m_retmsg.compare(0, 3, "226") != 0)
		{
			printf("�ر��������Ӻ����˷�������: %s\n", m_retmsg.c_str());
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "�ر��������Ӻ����˷�������: %s", m_retmsg.c_str());
			return false;
		}
	}

	return true;
}
bool CFTPclient::DownloadFile(string remotefile, string localfile, bool pasv)
{
	return true;
}

int CFTPclient::CanRead(int fd, int timeoutsec, int timeoutusec)
{
	fd_set rSet;
	FD_ZERO(&rSet);
	FD_SET(fd, &rSet);
		
	struct timeval sTimeout;
	memset(&sTimeout, 0, sizeof(sTimeout));
	sTimeout.tv_sec = timeoutsec;
	sTimeout.tv_usec = timeoutusec;
		
	int ret;
	while(1)
	{
		ret = select(fd + 1, &rSet, NULL, NULL, &sTimeout);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			return ret;
		}
		else if(ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
		else if(ret == 1)
			break;
	}
	
	return 0;
}
int CFTPclient::CanWrite(int fd, int timeoutsec, int timeoutusec)
{
	fd_set wSet;
	FD_ZERO(&wSet);
	FD_SET(fd, &wSet);
		
	struct timeval sTimeout;
	memset(&sTimeout, 0, sizeof(sTimeout));
	sTimeout.tv_sec = timeoutsec;
	sTimeout.tv_usec = timeoutusec;
		
	int ret;
	while(1)
	{
		ret = select(fd + 1, NULL, &wSet, NULL, &sTimeout);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			return ret;
		}
		else if(ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
		else if(ret == 1)
			break;
	}
	
	return 0;
}

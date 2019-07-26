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
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "创建控制socket失败: %s", strerror(errno));
		return false;
	}

	char szSendBuf[SEND_LEN] = {0};
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(hostport);
	server.sin_addr.s_addr = inet_addr(hostname.c_str());

	//连接到服务器
	int ret = connect(m_sockControlSock, (struct sockaddr*)&server, sizeof(server));
	if(ret != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "连接到服务器失败， IP: %s, 端口: %d", hostname.c_str(), hostport);
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
		printf("FTP读超时\n");
		return false;
	}

	//发送用户名到服务端
	snprintf(szSendBuf, sizeof(szSendBuf), "USER %s", username.c_str());
	bool commandRet = FTPcommand(szSendBuf);
	if(!commandRet)
	{
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到服务器失败 command: %s", szSendBuf);
		return false;
	}
	if(m_retmsg.compare(0, 3, "331") != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送用户名到服务器, 反馈失败: %s, command: %s", m_retmsg.c_str(), szSendBuf);
		return false;
	}

	//发送密码到服务端
	snprintf(szSendBuf, sizeof(szSendBuf), "PASS %s", password.c_str());
	commandRet = FTPcommand(szSendBuf);
	if(!commandRet)
	{
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到服务器失败 command: %s", szSendBuf);
		return false;
	}
	if(m_retmsg.compare(0, 3, "230") != 0)
	{
		CLOSE(m_sockControlSock);
		IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送密码到服务器, 反馈失败: %s, command: %s", m_retmsg.c_str(), szSendBuf);
		return false;
	}
	m_bIsLogin = true;

	return true;
}

void CFTPclient::LogOffServer()
{
	if(m_bIsLogin)
	{
		//断开与服务端的连接
		string strSend = "QUIT";
		bool commandRet = FTPcommand(strSend);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到客户端失败 command: %s", strSend.c_str());
			return;
		}
		if(m_retmsg.compare(0, 3, "221") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "断开与服务端的连接失败: %s", m_retmsg.c_str());
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
		printf("FTP写超时 command: %s\n", command.c_str());
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

//发送命令
bool CFTPclient::FTPcommand(string command)
{
	m_retmsg = "";
	command += "\r\n";

	char 	szReadBuf[READ_LEN];
	char 	szSendBuf[SEND_LEN];

	// 清空m_sockControlSock的接收区缓存
	if (CanRead(m_sockControlSock, 0, 1) == 0)
	{
		memset(szReadBuf, 0, sizeof(szReadBuf));
		recv(m_sockControlSock, szReadBuf, sizeof(szReadBuf), 0);
		printf("读取到的额外消息: %s", szReadBuf);
	}

	if(CanWrite(m_sockControlSock, 3) == 0)
	{
		snprintf(szSendBuf, sizeof(szSendBuf), "%s", command.c_str());
		send(m_sockControlSock, szSendBuf, strlen(szSendBuf), 0);
	}
	else
	{
		m_retmsg = "";
		printf("FTP写超时 command: %s\n", command.c_str());
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
		printf("FTP读超时 command: %s\n", command.c_str());
		return false;
	}

	m_retmsg = szReadBuf;
	printf("发送的命令: %s\n收到的反馈: %s\n", command.c_str(), szReadBuf);

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

		//请求二进制传输
		sprintf(szSendBuf, "TYPE I");
		bool commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到客户端失败 command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "200") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "请求二进制传输成功: %s", m_retmsg.c_str());
			return false;
		}

		//告诉服务端用被动模式
		sprintf(szSendBuf, "PASV");
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到客户端失败 command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "227") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "服务端启用被动模式失败: %s", m_retmsg.c_str());
			return false;
		}

		/*将服务器反馈的信息转换为IP，端口*/
		int h1,h2,h3,h4,p1,p2;
		sscanf(m_retmsg.c_str(), "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
		char szAddr[16];
		sprintf(szAddr, "%d.%d.%d.%d", h1, h2, h3, h4);
		int port = p1 * 256 + p2;
		printf("IP: %s, PORT: %d\n", szAddr, port);

		//连接服务端新开的端口
		SOCKET sockDataSock;
		struct sockaddr_in sDataServer;
		sDataServer.sin_family = AF_INET;
		sDataServer.sin_port = htons(port);
		sDataServer.sin_addr.s_addr = inet_addr(szAddr);

		//初始化数据连接socket
		sockDataSock = socket(AF_INET, SOCK_STREAM, 0);
		if(sockDataSock == -1)
		{
			m_retmsg = "";
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "创建数据socket失败: %s", strerror(errno));
			return false;
		}

		//连接服务器新开的数据端口
		int ret = connect(sockDataSock, (struct sockaddr *)&sDataServer, sizeof(sDataServer));
		if(ret != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "连接到数据服务端口失败: %s IP: %s, 端口: %d", szSendBuf, szAddr, port);
			CLOSE(sockDataSock);
			return false;
		}

		/*
		//获取文件列表
		sprintf(szSendBuf, "LIST");
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			printf("发送命令到客户端失败 command: %s\n", szSendBuf);
			m_pLoger->WriteLog("发送命令到客户端失败 command: %s", szSendBuf);
			return false;
		}
		if(m_retmsg.compare(0, 3, "150") != 0)
		{
			printf("获取文件列表失败: %s\n", m_retmsg.c_str());
			m_pLoger->WriteLog("获取文件列表失败: %s", m_retmsg.c_str());
			return false;
		}

		char tmpBuf[4096];
		read(sockDataSock, tmpBuf, 4096);
		printf("%s\n", tmpBuf);
		*/


		//客户端发送命令改变目录
		//int pos = remotefile.rfind("/");
		//string strFilePath(remotefile, 0, pos);
//		snprintf(szSendBuf, sizeof(szSendBuf), "CWD /home/testftp/data");
//		commandRet = FTPcommand(szSendBuf);
//		if(!commandRet)
//		{
//			printf("发送命令到客户端失败 command: %s\n", szSendBuf);
//			//m_pLoger->WriteLog("发送命令到客户端失败 command: %s", szSendBuf);
//			close(sockDataSock);
//			return false;
//		}
//		if(m_retmsg.compare(0, 3, "250") != 0)
//		{
//			printf("改变目录失败: %s, command: %s\n", m_retmsg.c_str(), szSendBuf);
//			//m_pLoger->WriteLog("改变目录失败: %s, command: %s", m_retmsg.c_str(), szSendBuf);
//			close(sockDataSock);
//			return false;
//		}

		//客户端发送上传文件命令 上传到北京的FTP只需要上传文件名
		int pos = remotefile.rfind("/");
		string strFileName(remotefile, pos + 1);
		snprintf(szSendBuf, sizeof(szSendBuf), "STOR %s", strFileName.c_str());
		commandRet = FTPcommand(szSendBuf);
		if(!commandRet)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送命令到客户端失败 command: %s", szSendBuf);
			CLOSE(sockDataSock);
			return false;
		}
		if(m_retmsg.compare(0, 3, "150") != 0)
		{
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "发送上传文件命令失败: %s, command: %s", m_retmsg.c_str(), szSendBuf);
			CLOSE(sockDataSock);
			return false;
		}

		//上传文件
		FILE *fp = fopen(localfile.c_str(), "rb");
		if(fp == NULL)
		{
			m_retmsg = "";
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "打开上传文件失败: %s， filename: %s", strerror(errno), localfile.c_str());
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
				IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "读取文件失败: %s， filename: %s", strerror(errno), localfile.c_str());
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
				IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "数据描述符不可写: %s， filename: %s", strerror(errno), localfile.c_str());
				fclose(fp);
				CLOSE(sockDataSock);
				return false;
			}
			if(readRet == 0)
				break;
		}
		fclose(fp);

		//客户端关闭数据连接
		CLOSE(sockDataSock);
		memset(szSendBuf, 0, sizeof(szSendBuf));
		if(CanRead(m_sockControlSock, 3) == 0)
		{
			recv(m_sockControlSock, szSendBuf, sizeof(szSendBuf), 0);
		}
		else
		{
			printf("关闭数据连接后读控制描述符失败\n");
			m_retmsg = "";
			return false;
		}
		printf("%s\n", szSendBuf);

		m_retmsg = szSendBuf;
		if(m_retmsg.compare(0, 3, "226") != 0)
		{
			printf("关闭数据连接后服务端反馈错误: %s\n", m_retmsg.c_str());
			IVE_Logger_AddLog(LOGLEVEL_ERROR, "FTPError.txt", "FTP", "关闭数据连接后服务端反馈错误: %s", m_retmsg.c_str());
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

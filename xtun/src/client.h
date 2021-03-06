#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <netinet/in.h>
#include <vector>
#include <unordered_map>
#include "reactor.h"
#include "msgdata.h"
#include "logger.h"

const size_t PW_MAX_LEN = 32; // len of md5
const size_t MAX_BUF_SIZE = 1024;
const int HEARTBEAT_INTERVAL_MS = 1000; // 每次心跳的间隔时间
const long DEFAULT_SERVER_TIMEOUT_MS = 5000; // 默认5秒没收到服务端的心跳表示服务端不在线
extern const char HEARTBEAT_CLIENT_MSG[];
extern const char HEARTBEAT_SERVER_MSG[];

struct ProxyInfo
{
  unsigned short remotePort;
  unsigned short localPort;
  char localIp[INET_ADDRSTRLEN];
};

enum AUTH_STATUS
{
  AUTH_ERR = -1,  // 程序出错
  AUTH_OK = 0,    // 密码正确
  AUTH_WRONG = 1, // 密码错误
  AUTH_UNKNOW     // 服务器返回了未知数据
};

struct NetData
{
  size_t recvNum;
  size_t recvSize;
  MsgData msgData;
  char buf[MAX_BUF_SIZE];
  NetData() : recvNum(0), recvSize(sizeof(msgData)) {}
};

struct ProxyConnInfo
{
  /* data */
  int localFd;

  int sendSize;
  char sendBuf[MAX_BUF_SIZE];
  ProxyConnInfo() : sendSize(0) {}
};
using ProxyConnInfoMap = std::unordered_map<int, ProxyConnInfo>;

struct LocalConnInfo
{
  /* data */
  int proxyFd;

  int sendSize;
  char sendBuf[MAX_BUF_SIZE];
  LocalConnInfo() : sendSize(0) {}
};
using LocalConnInfoMap = std::unordered_map<int, LocalConnInfo>;

class Client
{
private:
  std::vector<ProxyInfo> m_configProxy;
  unsigned short m_serverPort;
  unsigned short m_serverProxyPort;
  char m_serverIp[INET_ADDRSTRLEN];
  int m_clientSocketFd;
  char m_password[PW_MAX_LEN];
  Logger *m_pLogger;

  long long m_heartTimerId;
  long m_maxServerTimeout;   // 多少毫秒没收到服务端的心跳表示断开了连接
  long long m_lastServerHeartbeatMs; // 时间戳，上次收到服务端心跳的时间

  Reactor m_reactor;
  NetData m_clientData;

  ProxyConnInfoMap m_mapProxyConn;
  LocalConnInfoMap m_mapLocalConn;

  void clientReadProc(int fd, int mask);
  int sendPorts();
  void porcessMsgBuf();
  void makeNewProxy(NewProxyMsg newProxy);
  int connectLocalApp(unsigned short remotePort);
  int connectServerProxy();
  void replyNewProxy(int userId, bool isSuccess);
  int sendProxyInfo(int porxyFd, int userId);
  int sendHeartbeatTimerProc(long long id);
  void processHeartbeat();  // 收到服务端的心跳做的处理
  int checkHeartbeatTimerProc(long long id);

  void localReadDataProc(int fd, int mask);
  void localWriteDataProc(int fd, int mask);
  void proxyReadDataProc(int fd, int mask);
  void proxyWriteDataProc(int fd, int mask);

  void deleteProxyConn(int fd);
  void deleteLocalConn(int fd);

  int connectServer();
  int authServer();

public:
  Client(const char *sip, unsigned short sport);
  ~Client();

  void setProxyConfig(const std::vector<ProxyInfo> &pcs);
  void setProxyPort(unsigned short proxyPort);
  void setPassword(const char *password);
  void setLogger(Logger* logger);
  
  void runClient();
};

#endif // __CLIENT_H__
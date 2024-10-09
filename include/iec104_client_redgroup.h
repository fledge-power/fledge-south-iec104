#ifndef IEC104_CLIENT_REDGROUP_H
#define IEC104_CLIENT_REDGROUP_H

#include <string>
#include <vector>
#include <memory>

class RedGroupCon
{
public:

    RedGroupCon(const std::string& serverIp, int tcpPort, bool conn, bool start, const std::string& clientIp);

    ~RedGroupCon();

    const std::string& ServerIP() const {return m_serverIp;};
    const std::string& ClientIP() const {return m_clientIp;};
    int TcpPort() const {return m_tcpPort;};
    bool Conn() const {return m_conn;};
    bool Start() const {return m_start;};
    long ConnId() const {return m_connId;};
    void SetConnId(long connId) {m_connId = connId;};

private:
    
    /* configuration properties */
    long m_connId = -1;
    std::string m_serverIp;
    std::string m_clientIp;
    int m_tcpPort = 2404;
    bool m_conn = true;
    bool m_start = true;
};

class IEC104ClientRedGroup
{
public:

    IEC104ClientRedGroup(const std::string& name, int index): m_name(name), m_index(index) {};
    ~IEC104ClientRedGroup();

    const std::string& Name() const {return m_name;};
    int Index() const {return m_index;};
    bool UseTLS() const {return m_useTls;};

    std::vector<std::shared_ptr<RedGroupCon>>& Connections() {return m_connections;};

    int K() const {return m_k;};
    int W() const {return m_w;};
    int T0() const {return m_t0;};
    int T1() const {return m_t1;};
    int T2() const {return m_t2;};
    int T3() const {return m_t3;};

    void K(int k) {m_k = k;};
    void W(int w) {m_w = w;};
    void T0(int t0) {m_t0 = t0;};
    void T1(int t1) {m_t1 = t1;};
    void T2(int t2) {m_t2 = t2;};
    void T3(int t3) {m_t3 = t3;};

    void UseTLS(bool useTls) {m_useTls = useTls;};

    void AddConnection(std::shared_ptr<RedGroupCon> con);

private:

    std::vector<std::shared_ptr<RedGroupCon>> m_connections;

    std::string m_name;
    int m_index;
    bool m_useTls = false;
    
    int m_k = 12;
    int m_w = 8;
    int m_t0 = 30;
    int m_t1 = 15;
    int m_t2 = 10;
    int m_t3 = 20;
};


#endif /* IEC104_CLIENT_REDGROUP_H */
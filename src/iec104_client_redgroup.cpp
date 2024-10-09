#include "iec104_client_redgroup.h"

using namespace std;

RedGroupCon::RedGroupCon(const string& serverIp, int tcpPort, bool conn, bool start, const string& clientIp)
{
    m_serverIp = serverIp;
    m_tcpPort = tcpPort;
    m_start = start;
    m_conn = conn;
    m_clientIp = clientIp;
}

RedGroupCon::~RedGroupCon()
{}

void IEC104ClientRedGroup::AddConnection(std::shared_ptr<RedGroupCon> con)
{
    con->SetConnId(m_connections.size());
    m_connections.push_back(con);
}

IEC104ClientRedGroup::~IEC104ClientRedGroup()
{}
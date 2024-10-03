#ifndef INCLUDE_IEC104_H_
#define INCLUDE_IEC104_H_

/*
 * Fledge IEC 104 south plugin.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Michael Zillgith
 *
 */
#include <memory>

#include <plugin_api.h>
#include <reading.h>

#include <lib60870/cs104_connection.h>

class IEC104Client;
class IEC104ClientConfig;

class IEC104
{
public:
    typedef void (*INGEST_CB)(void*, Reading);

    IEC104();
    ~IEC104();

    void setAssetName(const std::string& asset) { m_asset = asset; }
    void setJsonConfig(const std::string& stack_configuration,
                              const std::string& msg_configuration,
                              const std::string& tls_configuration);

    void start();
    void stop();

    // void ingest(Reading& reading);
    void ingest(std::string assetName, std::vector<Datapoint*>& points);
    void registerIngest(void* data, void (*cb)(void*, Reading));
    bool operation(const std::string& operation, int count,
                   PLUGIN_PARAMETER** params);

    inline const std::string& getServiceName() const { return m_service_name; }
    inline void setServiceName(const std::string& serviceName) { m_service_name = serviceName; }

    inline std::shared_ptr<IEC104Client> getClient() const { return m_client; }

private:

    bool m_singleCommandOperation(int count, PLUGIN_PARAMETER** params, bool withTime);
    bool m_doubleCommandOperation(int count, PLUGIN_PARAMETER** params, bool withTime);
    bool m_stepCommandOperation(int count, PLUGIN_PARAMETER** params, bool withTime);
    bool m_setpointNormalized(int count, PLUGIN_PARAMETER** params, bool withTime);
    bool m_setpointScaled(int count, PLUGIN_PARAMETER** params, bool withTime);
    bool m_setpointShort(int count, PLUGIN_PARAMETER** params, bool withTime);

    std::shared_ptr<IEC104ClientConfig> m_config;

    std::string m_asset;

protected:
    std::vector<CS104_Connection> m_connections;

private:
    INGEST_CB m_ingest = nullptr;  // Callback function used to send data to south service
    void* m_data = nullptr;        // Ingest function data
    std::shared_ptr<IEC104Client> m_client;
    std::string m_service_name;    // Service name used to generate audits
};

#endif  // INCLUDE_IEC104_H_

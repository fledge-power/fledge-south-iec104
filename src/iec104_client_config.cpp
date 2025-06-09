#include <algorithm>

#include <rapidjson/error/en.h>

#include <arpa/inet.h>
#include <lib60870/cs104_connection.h>

#include "iec104_client_config.h"
#include "iec104_client_redgroup.h"
#include "iec104_utility.h"



#define JSON_EXCHANGED_DATA "exchanged_data"
#define JSON_DATAPOINTS "datapoints"
#define JSON_PROTOCOLS "protocols"
#define JSON_LABEL "label"
#define JSON_PIVOT_SUBTYPES "pivot_subtypes"

#define PROTOCOL_IEC104 "iec104"
#define JSON_PROT_NAME "name"
#define JSON_PROT_ADDR "address"
#define JSON_PROT_TYPEID "typeid"
#define JSON_PROT_GI_GROUPS "gi_groups"
#define JSON_TRIGGER_SOUTH_GI_PIVOT_SUBTYPE "trigger_south_gi"

using namespace rapidjson;
using namespace std;

// Map of all existing ASDU types
static std::map<std::string, int> mapAsduTypeId = {
    {"M_SP_TA_1", M_SP_TA_1},
    {"M_SP_NA_1", M_SP_NA_1},
    {"M_DP_NA_1", M_DP_NA_1},
    {"M_DP_TA_1", M_DP_TA_1},
    {"M_ST_NA_1", M_ST_NA_1},
    {"M_ST_TA_1", M_ST_TA_1},
    {"M_BO_NA_1", M_BO_NA_1},
    {"M_BO_TA_1", M_BO_TA_1},
    {"M_ME_NA_1", M_ME_NA_1},
    {"M_ME_TA_1", M_ME_TA_1},
    {"M_ME_NB_1", M_ME_NB_1},
    {"M_ME_TB_1", M_ME_TB_1},
    {"M_ME_NC_1", M_ME_NC_1},
    {"M_ME_TC_1", M_ME_TC_1},
    {"M_IT_NA_1", M_IT_NA_1},
    {"M_IT_TA_1", M_IT_TA_1},
    {"M_EP_TA_1", M_EP_TA_1},
    {"M_EP_TB_1", M_EP_TB_1},
    {"M_EP_TC_1", M_EP_TC_1},
    {"M_PS_NA_1", M_PS_NA_1},
    {"M_ME_ND_1", M_ME_ND_1},
    {"M_SP_TB_1", M_SP_TB_1},
    {"M_DP_TB_1", M_DP_TB_1},
    {"M_ST_TB_1", M_ST_TB_1},
    {"M_BO_TB_1", M_BO_TB_1},
    {"M_ME_TD_1", M_ME_TD_1},
    {"M_ME_TE_1", M_ME_TE_1},
    {"M_ME_TF_1", M_ME_TF_1},
    {"M_IT_TB_1", M_IT_TB_1},
    {"M_EP_TD_1", M_EP_TD_1},
    {"M_EP_TE_1", M_EP_TE_1},
    {"M_EP_TF_1", M_EP_TF_1},
    {"S_IT_TC_1", S_IT_TC_1},
    {"C_SC_NA_1", C_SC_NA_1},
    {"C_DC_NA_1", C_DC_NA_1},
    {"C_RC_NA_1", C_RC_NA_1},
    {"C_SE_NA_1", C_SE_NA_1},
    {"C_SE_NB_1", C_SE_NB_1},
    {"C_SE_NC_1", C_SE_NC_1},
    {"C_BO_NA_1", C_BO_NA_1},
    {"C_SC_TA_1", C_SC_TA_1},
    {"C_DC_TA_1", C_DC_TA_1},
    {"C_RC_TA_1", C_RC_TA_1},
    {"C_SE_TA_1", C_SE_TA_1},
    {"C_SE_TB_1", C_SE_TB_1},
    {"C_SE_TC_1", C_SE_TC_1},
    {"C_BO_TA_1", C_BO_TA_1},
    {"M_EI_NA_1", M_EI_NA_1},
    {"S_CH_NA_1", S_CH_NA_1},
    {"S_RP_NA_1", S_RP_NA_1},
    {"S_AR_NA_1", S_AR_NA_1},
    {"S_KR_NA_1", S_KR_NA_1},
    {"S_KS_NA_1", S_KS_NA_1},
    {"S_KC_NA_1", S_KC_NA_1},
    {"S_ER_NA_1", S_ER_NA_1},
    {"S_US_NA_1", S_US_NA_1},
    {"S_UQ_NA_1", S_UQ_NA_1},
    {"S_UR_NA_1", S_UR_NA_1},
    {"S_UK_NA_1", S_UK_NA_1},
    {"S_UA_NA_1", S_UA_NA_1},
    {"S_UC_NA_1", S_UC_NA_1},
    {"C_IC_NA_1", C_IC_NA_1},
    {"C_CI_NA_1", C_CI_NA_1},
    {"C_RD_NA_1", C_RD_NA_1},
    {"C_CS_NA_1", C_CS_NA_1},
    {"C_TS_NA_1", C_TS_NA_1},
    {"C_RP_NA_1", C_RP_NA_1},
    {"C_CD_NA_1", C_CD_NA_1},
    {"C_TS_TA_1", C_TS_TA_1},
    {"P_ME_NA_1", P_ME_NA_1},
    {"P_ME_NB_1", P_ME_NB_1},
    {"P_ME_NC_1", P_ME_NC_1},
    {"P_AC_NA_1", P_AC_NA_1},
    {"F_FR_NA_1", F_FR_NA_1},
    {"F_SR_NA_1", F_SR_NA_1},
    {"F_SC_NA_1", F_SC_NA_1},
    {"F_LS_NA_1", F_LS_NA_1},
    {"F_AF_NA_1", F_AF_NA_1},
    {"F_SG_NA_1", F_SG_NA_1},
    {"F_DR_TA_1", F_DR_TA_1},
    {"F_SC_NB_1", F_SC_NB_1}
};

// Map is automatically initialized from mapAsduTypeId at first getStringFromTypeID() call
static std::map<int, std::string> mapAsduTypeIdStr = {};

int
IEC104ClientConfig::getTypeIdFromString(const string& name)
{
    return mapAsduTypeId[name];
}

std::string
IEC104ClientConfig::getStringFromTypeID(int typeId)
{
    // Build reverse mapping if not yet initialized
    if (mapAsduTypeIdStr.empty()) {
        for(const auto& kvp : mapAsduTypeId) {
            mapAsduTypeIdStr[kvp.second]=kvp.first;
        }
    }
    
    return mapAsduTypeIdStr[typeId];
}

bool
IEC104ClientConfig::isMessageTypeMatching(int expectedType, int rcvdType)
{
    if (expectedType == rcvdType) {
        return true; /* direct match */
    }

    switch (expectedType) {

        case M_SP_NA_1:
            if ((rcvdType == M_SP_TA_1) || (rcvdType == M_SP_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_SP_TA_1:
            if ((rcvdType == M_SP_NA_1) || (rcvdType == M_SP_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_SP_TB_1:
            if ((rcvdType == M_SP_NA_1) || (rcvdType == M_SP_TA_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_DP_NA_1:
            if ((rcvdType == M_DP_TA_1) || (rcvdType == M_DP_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_DP_TA_1:
            if ((rcvdType == M_DP_NA_1) || (rcvdType == M_DP_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_DP_TB_1:
            if ((rcvdType == M_DP_NA_1) || (rcvdType == M_DP_TA_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_NA_1:
            if ((rcvdType == M_ME_TA_1) || (rcvdType == M_ME_TD_1) || (rcvdType == M_ME_ND_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TA_1:
            if ((rcvdType == M_ME_NA_1) || (rcvdType == M_ME_TD_1) || (rcvdType == M_ME_ND_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TD_1:
            if ((rcvdType == M_ME_TA_1) || (rcvdType == M_ME_NA_1) || (rcvdType == M_ME_ND_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_ND_1:
            if ((rcvdType == M_ME_TA_1) || (rcvdType == M_ME_TD_1) || (rcvdType == M_ME_NA_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_NB_1:
            if ((rcvdType == M_ME_TB_1) || (rcvdType == M_ME_TE_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TB_1:
            if ((rcvdType == M_ME_NB_1) || (rcvdType == M_ME_TE_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TE_1:
            if ((rcvdType == M_ME_TB_1) || (rcvdType == M_ME_NB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_NC_1:
            if ((rcvdType == M_ME_TC_1) || (rcvdType == M_ME_TF_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TC_1:
            if ((rcvdType == M_ME_NC_1) || (rcvdType == M_ME_TF_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ME_TF_1:
            if ((rcvdType == M_ME_TC_1) || (rcvdType == M_ME_NC_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE


        case M_ST_NA_1:
            if ((rcvdType == M_ST_TA_1) || (rcvdType == M_ST_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ST_TA_1:
            if ((rcvdType == M_ST_NA_1) || (rcvdType == M_ST_TB_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case M_ST_TB_1:
            if ((rcvdType == M_ST_TA_1) || (rcvdType == M_ST_NA_1)) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SC_NA_1:
            if (rcvdType == C_SC_TA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SC_TA_1:
            if (rcvdType == C_SC_NA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_DC_NA_1:
            if (rcvdType == C_DC_TA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_DC_TA_1:
            if (rcvdType == C_DC_NA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_RC_NA_1:
            if (rcvdType == C_RC_TA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_RC_TA_1:
            if (rcvdType == C_RC_NA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SE_NA_1:
            if (rcvdType == C_SE_TA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SE_TA_1:
            if (rcvdType == C_SE_NA_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SE_NB_1:
            if (rcvdType == C_SE_TB_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SE_TB_1:
            if (rcvdType == C_SE_NB_1) {
                return true;
            }

        case C_SE_NC_1:
            if (rcvdType == C_SE_TC_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        case C_SE_TC_1:
            if (rcvdType == C_SE_NC_1) {
                return true;
            }

            break; //LCOV_EXCL_LINE

        default:
            //Type not supported
            break; //LCOV_EXCL_LINE
    }   

    return false;
}

std::string*
IEC104ClientConfig::checkExchangeDataLayer(int typeId, int ca, int ioa)
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::checkExchangeDataLayer -"; //LCOV_EXCL_LINE
    const auto& def = ExchangeDefinition()[ca][ioa];

    if (def != nullptr) {
        // check if message type is matching the exchange definition
        if (isMessageTypeMatching(def->typeId, typeId)) {
            return &(def->label);
        }
        else {
            Iec104Utility::log_warn("%s data point %i:%i found but type %s (%i) not matching", beforeLog.c_str(), ca, ioa, //LCOV_EXCL_LINE
                                    IEC104ClientConfig::getStringFromTypeID(def->typeId).c_str(), def->typeId);
        }
    }
    else {
        Iec104Utility::log_warn("%s data point %i:%i not found", beforeLog.c_str(), ca, ioa); //LCOV_EXCL_LINE
    }

    return nullptr;
}

bool
IEC104ClientConfig::isValidIPAddress(const string& addrStr)
{
    // see https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

    return (result == 1);
}

void IEC104ClientConfig::importProtocolConfig(const string& protocolConfig)
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::importProtocolConfig -"; //LCOV_EXCL_LINE
    m_protocolConfigComplete = false;

    Document document;

    if (document.Parse(const_cast<char*>(protocolConfig.c_str())).HasParseError()) {
        Iec104Utility::log_fatal("%s Parsing error in protocol_stack json, offset %u: %s", beforeLog.c_str(), //LCOV_EXCL_LINE
                                static_cast<unsigned>(document.GetErrorOffset()), GetParseError_En(document.GetParseError()));
        return;
    }

    if (!document.IsObject()) {
        Iec104Utility::log_fatal("%s Root is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    if (!document.HasMember("protocol_stack") || !document["protocol_stack"].IsObject()) {
        Iec104Utility::log_fatal("%s protocol_stack does not exist or is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    const Value& protocolStack = document["protocol_stack"];

    if (!protocolStack.HasMember("transport_layer") || !protocolStack["transport_layer"].IsObject()) {
        Iec104Utility::log_fatal("%s transport layer does not exist or is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    if (!protocolStack.HasMember("application_layer") || !protocolStack["application_layer"].IsObject()) {
        Iec104Utility::log_fatal("%s appplication layer does not exist or is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    if (protocolStack.HasMember("south_monitoring")) 
    {
        const Value& southMonitoring = protocolStack["south_monitoring"];

        if (southMonitoring.IsObject()) {
            if (southMonitoring.HasMember("asset")) {
                if (southMonitoring["asset"].IsString()) {
                    m_connxStatus = southMonitoring["asset"].GetString();
                }
                else {
                    Iec104Utility::log_error("%s south_monitoring \"asset\" element is not a string", beforeLog.c_str()); //LCOV_EXCL_LINE
                }
            }
            else {
                Iec104Utility::log_error("%s south_monitoring is missing \"asset\" element", beforeLog.c_str()); //LCOV_EXCL_LINE
            }
        }
        else {
            Iec104Utility::log_error("%s south_monitoring is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        }
    }

    const Value& transportLayer = protocolStack["transport_layer"];
    const Value& applicationLayer = protocolStack["application_layer"];

    if (transportLayer.HasMember("redundancy_groups")) {

        if (transportLayer["redundancy_groups"].IsArray()) {

            const Value& redundancyGroups = transportLayer["redundancy_groups"];

            for (const Value& redGroup : redundancyGroups.GetArray()) {
                importRedGroup(redGroup);
            }
        }
        else {
            Iec104Utility::log_fatal("%s redundancy_groups is not an array -> ignore redundancy groups", beforeLog.c_str()); //LCOV_EXCL_LINE
        }
    }

    /* Application layer parameters */

    if (applicationLayer.HasMember("orig_addr")) {
        if (applicationLayer["orig_addr"].IsInt()) {
            int origAddr = applicationLayer["orig_addr"].GetInt();

            if (origAddr >= 0 && origAddr <= 255) {
                m_origAddr = origAddr;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.orig_addr value out of range [1..255]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), origAddr, m_origAddr);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.orig_addr is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_origAddr);
        }
    }

    if (applicationLayer.HasMember("ca_asdu_size")) {
        if (applicationLayer["ca_asdu_size"].IsInt()) {
            int caSize = applicationLayer["ca_asdu_size"].GetInt();

            if (caSize > 0 && caSize < 3) {
                m_caSize = caSize;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.ca_asdu_size value out of range [1..2]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), caSize, m_caSize);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.ca_asdu_size is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_caSize);
        }
    }

    if (applicationLayer.HasMember("ioaddr_size")) {
        if (applicationLayer["ioaddr_size"].IsInt()) {
            int ioaSize = applicationLayer["ioaddr_size"].GetInt();

            if (ioaSize > 0 && ioaSize < 4) {
                m_ioaSize = ioaSize;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.ioaddr_size value out of range [1..3]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), ioaSize, m_ioaSize);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.ioaddr_size is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_ioaSize);
        }
    }

    if (applicationLayer.HasMember("asdu_size")) {
        if (applicationLayer["asdu_size"].IsInt()) {
            int asduSize = applicationLayer["asdu_size"].GetInt();

            if (asduSize == 0 || (asduSize > 10 && asduSize < 254)) {
                m_asduSize = asduSize;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.asdu_size value out of range [0,11..253]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), asduSize, m_asduSize);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.asdu_size is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_asduSize);
        }
    }

    if (applicationLayer.HasMember("time_sync")) {
        if (applicationLayer["time_sync"].IsInt()) {
            int timeSyncValue = applicationLayer["time_sync"].GetInt();

            if (timeSyncValue < 0) {
                Iec104Utility::log_warn("%s application_layer.time_sync value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), timeSyncValue, m_timeSyncPeriod);
            }
            else {
                m_timeSyncPeriod = timeSyncValue;
            }

        }
        else {
            Iec104Utility::log_warn("%s application_layer.time_sync is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_timeSyncPeriod);
        }
    }

    if (applicationLayer.HasMember("gi_all_ca")) {
        if (applicationLayer["gi_all_ca"].IsBool()) {
            m_giAllCa = applicationLayer["gi_all_ca"].GetBool();
        }
        else {
            Iec104Utility::log_warn("%s applicationLayer.gi_all_ca is not a bool -> using default value (%s)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    (m_giAllCa?"true":"false"));
        }
    }

    if (applicationLayer.HasMember("gi_time")) {
        if (applicationLayer["gi_time"].IsInt()) {
            int giTime = applicationLayer["gi_time"].GetInt();

            if (giTime >= 0) {
                m_giTime = giTime;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.gi_time value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), giTime, m_giTime);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.gi_time is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_giTime);
        }
    }

    if (applicationLayer.HasMember("gi_enabled")) {
        if (applicationLayer["gi_enabled"].IsBool()) {
            m_giEnabled = applicationLayer["gi_enabled"].GetBool();
        }
        else {
            Iec104Utility::log_warn("%s applicationLayer.gi_enabled is not a bool -> using default value (%s)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    (m_giEnabled?"true":"false"));
        }
    }

    if (applicationLayer.HasMember("gi_cycle")) {
        if (applicationLayer["gi_cycle"].IsInt()) {
            int giCycle = applicationLayer["gi_cycle"].GetInt();

            if (giCycle >= 0) {
                m_giCycle = giCycle;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.gi_cycle value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), giCycle, m_giCycle);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.gi_cycle is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_giCycle);
        }
    }

    if (applicationLayer.HasMember("gi_repeat_count")) {
        if (applicationLayer["gi_repeat_count"].IsInt()) {
            int giRepeatCount = applicationLayer["gi_repeat_count"].GetInt();

            if (giRepeatCount >= 0) {
                m_giRepeatCount = giRepeatCount;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.gi_repeat_count value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), giRepeatCount, m_giRepeatCount);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.gi_repeat_count is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_giRepeatCount);
        }
    }

    if (applicationLayer.HasMember("cmd_parallel")) {              
        if (applicationLayer["cmd_parallel"].IsInt()) {
            int cmdParallel = applicationLayer["cmd_parallel"].GetInt();

            if (cmdParallel >= 0) {
                m_cmdParallel = cmdParallel;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.cmd_parallel value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), cmdParallel, m_cmdParallel);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.cmd_parallel is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_cmdParallel);
        }
    }

    if (applicationLayer.HasMember("cmd_exec_timeout")) {              
        if (applicationLayer["cmd_exec_timeout"].IsInt()) {
            int cmdExecTimeout = applicationLayer["cmd_exec_timeout"].GetInt();

            if (cmdExecTimeout >= 0) {
                m_cmdExecTimeout = cmdExecTimeout;
            }
            else {
                Iec104Utility::log_warn("%s application_layer.cmd_exec_timeout value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), cmdExecTimeout, m_cmdExecTimeout);
            }
        }
        else {
            Iec104Utility::log_warn("%s application_layer.cmd_exec_timeout is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    m_cmdExecTimeout);
        }
    }

    m_protocolConfigComplete = true;
}

void IEC104ClientConfig::importRedGroup(const Value& redGroup)
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::importRedGroup -"; //LCOV_EXCL_LINE
    
    if (!redGroup.IsObject()) {
        Iec104Utility::log_error("%s redundancy_groups element is not an object -> ignore", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }
    
    std::string redGroupName;
    bool hasName = false;
    if (redGroup.HasMember("rg_name")) {
        if (redGroup["rg_name"].IsString()) {
            redGroupName = redGroup["rg_name"].GetString();
            hasName = true;
        }
    }
    if (!hasName) {
        Iec104Utility::log_error("%s rg_name does not exist or is not a string -> ignore", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    auto redundancyGroup = std::make_shared<IEC104ClientRedGroup>(redGroupName, static_cast<int>(m_redundancyGroups.size()));

    Iec104Utility::log_debug("%s Adding red group with name: %s", beforeLog.c_str(), redGroupName.c_str()); //LCOV_EXCL_LINE

    if (redGroup.HasMember("connections") && redGroup["connections"].IsArray()) {
        for (const Value& con : redGroup["connections"].GetArray()) {
            importRedGroupCon(con, redundancyGroup);
        }
    }
    else {
        Iec104Utility::log_debug("%s  connections does not exist or is not an array -> adding fallback group", //LCOV_EXCL_LINE
                                beforeLog.c_str());
    }

    if (redGroup.HasMember("k_value")) {
        if (redGroup["k_value"].IsInt()) {
            int kValue = redGroup["k_value"].GetInt();

            if (kValue > 0 && kValue < 32768) {
                redundancyGroup->K(kValue);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.k_value value out of range [1..32767]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), kValue, redundancyGroup->K());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.k_value is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->K());
        }
    }

    if (redGroup.HasMember("w_value")) {
        if (redGroup["w_value"].IsInt()) {
            int wValue = redGroup["w_value"].GetInt();

            if (wValue > 0 && wValue < 32768) {
                redundancyGroup->W(wValue);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.w_value value out of range [1..32767]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), wValue, redundancyGroup->W());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.w_value is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->W());
        }
    }

    if (redGroup.HasMember("t0_timeout")) {
        if (redGroup["t0_timeout"].IsInt()) {
            int t0Timeout = redGroup["t0_timeout"].GetInt();

            if (t0Timeout > 0 && t0Timeout < 256) {
                redundancyGroup->T0(t0Timeout);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.t0_timeout value out of range [1..255]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), t0Timeout, redundancyGroup->T0());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.t0_timeout is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->T0());
        }
    }

    if (redGroup.HasMember("t1_timeout")) {
        if (redGroup["t1_timeout"].IsInt()) {
            int t1Timeout = redGroup["t1_timeout"].GetInt();

            if (t1Timeout > 0 && t1Timeout < 256) {
                redundancyGroup->T1(t1Timeout);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.t1_timeout value out of range [1..255]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), t1Timeout, redundancyGroup->T1());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.t1_timeout is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->T1());
        }
    }

    if (redGroup.HasMember("t2_timeout")) {
        if (redGroup["t2_timeout"].IsInt()) {
            int t2Timeout = redGroup["t2_timeout"].GetInt();

            if (t2Timeout > 0 && t2Timeout < 256) {
                redundancyGroup->T2(t2Timeout);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.t2_timeout value out of range [1..255]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), t2Timeout, redundancyGroup->T2());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.t2_timeout is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->T2());
        }
    }

    if (redGroup.HasMember("t3_timeout")) {
        if (redGroup["t3_timeout"].IsInt()) {
            int t3Timeout = redGroup["t3_timeout"].GetInt();

            if (t3Timeout > -1) {
                redundancyGroup->T3(t3Timeout);
            }
            else {
                Iec104Utility::log_warn("%s redGroup.t3_timeout value out of range [0..+Inf]: %d -> using default value (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), t3Timeout, redundancyGroup->T3());
            }
        }
        else {
            Iec104Utility::log_warn("%s redGroup.t3_timeout is not an integer -> using default value (%d)", beforeLog.c_str(), //LCOV_EXCL_LINE
                                    redundancyGroup->T3());
        }
    }

    if (redGroup.HasMember("tls")) {
        if (redGroup["tls"].IsBool()) {
            redundancyGroup->UseTLS(redGroup["tls"].GetBool());
        }
        else {
            Iec104Utility::log_warn("%s redGroup.tls is not a bool -> not using TLS", beforeLog.c_str()); //LCOV_EXCL_LINE
        }
    }

    m_redundancyGroups.push_back(redundancyGroup);
}

void IEC104ClientConfig::importRedGroupCon(const Value& con, std::shared_ptr<IEC104ClientRedGroup> redundancyGroup) const
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::importRedGroupCon -"; //LCOV_EXCL_LINE

    if(!con.IsObject()) {
        Iec104Utility::log_error("%s  connections element is not an object -> ignore", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    if (!con.HasMember("srv_ip") || !con["srv_ip"].IsString()) {
        Iec104Utility::log_error("%s  srv_ip does not exist or is not a string -> ignore", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }
    string srvIp = con["srv_ip"].GetString();

    if (!isValidIPAddress(srvIp)) {
        Iec104Utility::log_error("%s  srv_ip %s is not a valid IP address -> ignore", beforeLog.c_str(), srvIp.c_str()); //LCOV_EXCL_LINE
        return;
    }

    Iec104Utility::log_debug("%s  add to group: %s", beforeLog.c_str(), srvIp.c_str()); //LCOV_EXCL_LINE

    int tcpPort = 2404;
    bool start = true;
    bool conn = true;
    
    string clientIp;

    if (con.HasMember("clt_ip")) {
        if (con["clt_ip"].IsString()) {
            string cltIpStr = con["clt_ip"].GetString();

            if (isValidIPAddress(cltIpStr)) {
                clientIp = cltIpStr;
            }
            else {
                Iec104Utility::log_error("%s  clt_ip %s is not a valid IP address -> ignore client", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), cltIpStr.c_str());
            }
        }
        else {
            Iec104Utility::log_warn("%s  clt_ip is not a string -> ignore client", //LCOV_EXCL_LINE
                                    beforeLog.c_str());
        }
    }
    
    if (con.HasMember("port")) {
        if (con["port"].IsInt()) {
            int tcpPortVal = con["port"].GetInt();

            if (tcpPortVal > 0 && tcpPortVal < 65636) {
                tcpPort = tcpPortVal;
            }
            else {
                Iec104Utility::log_error("%s  port value out of range [1..65635]: %d -> using default port (%d)", //LCOV_EXCL_LINE
                                        beforeLog.c_str(), tcpPortVal, tcpPort);
            }
        }
        else {
            Iec104Utility::log_warn("%s  port is not an integer -> using default port (%d)", //LCOV_EXCL_LINE
                                    beforeLog.c_str(), tcpPort);
        }
    }

    if (con.HasMember("conn")) {
        if (con["conn"].IsBool()) {
            conn = con["conn"].GetBool();
        }
        else {
            Iec104Utility::log_warn("%s  conn is not a bool -> using default conn (%s)", //LCOV_EXCL_LINE
                                    beforeLog.c_str(), conn?"true":"false");
        }
    }

    if (con.HasMember("start")) {
        if (con["start"].IsBool()) {
            start = con["start"].GetBool();
        }
        else {
            Iec104Utility::log_warn("%s  start is not a bool -> using default start (%s)", //LCOV_EXCL_LINE
                                    beforeLog.c_str(), start?"true":"false");
        }
    }

    auto connection = std::make_shared<RedGroupCon>(srvIp, tcpPort, conn, start, clientIp);

    redundancyGroup->AddConnection(connection);
}

void IEC104ClientConfig::deleteExchangeDefinitions()
{
    m_exchangeDefinitions.clear();
}

void
IEC104ClientConfig::importTlsConfig(const string& tlsConfig)
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::importTlsConfig -"; //LCOV_EXCL_LINE
    Document document;
    m_tlsConfigComplete = false;

    if (document.Parse(const_cast<char*>(tlsConfig.c_str())).HasParseError()) {
        Iec104Utility::log_fatal("%s Parsing error in tls_conf json, offset %u: %s", beforeLog.c_str(), //LCOV_EXCL_LINE
                                static_cast<unsigned>(document.GetErrorOffset()), GetParseError_En(document.GetParseError()));
        return;
    }
       
    if (!document.IsObject()) {
        Iec104Utility::log_fatal("%s Root is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }
    
    if (!document.HasMember("tls_conf") || !document["tls_conf"].IsObject()) {
        Iec104Utility::log_debug("%s tls_conf does not exist or is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    const Value& tlsConf = document["tls_conf"];

    if (tlsConf.HasMember("private_key") && tlsConf["private_key"].IsString()) {
        m_privateKey = tlsConf["private_key"].GetString();
    }
    else {
        Iec104Utility::log_warn("%s private_key does not exist or is not a string", beforeLog.c_str()); //LCOV_EXCL_LINE
    }

    if (tlsConf.HasMember("own_cert") && tlsConf["own_cert"].IsString()) {
        m_ownCertificate = tlsConf["own_cert"].GetString();
    }
    else {
        Iec104Utility::log_warn("%s own_cert does not exist or is not a string", beforeLog.c_str()); //LCOV_EXCL_LINE
    }

    if (tlsConf.HasMember("ca_certs") && tlsConf["ca_certs"].IsArray()) {

        const Value& caCerts = tlsConf["ca_certs"];

        for (const Value& caCert : caCerts.GetArray()) {
            if (!caCert.IsObject()) {
                Iec104Utility::log_warn("%s ca_certs element is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
                continue;
            }

            if (caCert.HasMember("cert_file") && caCert["cert_file"].IsString()) {
                string certFileName = caCert["cert_file"].GetString();
                m_caCertificates.push_back(certFileName);
            }
            else {
                Iec104Utility::log_warn("%s ca_certs.cert_file does not exist or is not a string", beforeLog.c_str()); //LCOV_EXCL_LINE
            }
        }
    }
    else {
        Iec104Utility::log_warn("%s ca_certs does not exist or is not an array", beforeLog.c_str()); //LCOV_EXCL_LINE
    }

    if (tlsConf.HasMember("remote_certs") && tlsConf["remote_certs"].IsArray()) {

        const Value& remoteCerts = tlsConf["remote_certs"];

        for (const Value& remoteCert : remoteCerts.GetArray()) {
            if (!remoteCert.IsObject()) {
                Iec104Utility::log_warn("%s remote_certs element is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
                continue;
            }

            if (remoteCert.HasMember("cert_file") && remoteCert["cert_file"].IsString()) {
                string certFileName = remoteCert["cert_file"].GetString();
                m_remoteCertificates.push_back(certFileName);
            }
            else {
                Iec104Utility::log_warn("%s remote_certs.cert_file does not exist or is not a string", beforeLog.c_str()); //LCOV_EXCL_LINE
            }
        }
    }
    else {
        Iec104Utility::log_warn("%s remote_certs does not exist or is not an array", beforeLog.c_str()); //LCOV_EXCL_LINE
    }

    m_tlsConfigComplete = true;
}

static vector<string> tokenizeString(string& str, string delimiter)
{
    vector<string> tokens;

    size_t pos = 0;
    std::string token;

    str = str + delimiter;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);

        tokens.push_back(token);

        str.erase(0, pos + delimiter.length());
    }

    return tokens;
}

std::shared_ptr<DataExchangeDefinition>
IEC104ClientConfig::getExchangeDefinitionByLabel(std::string& label)
{
    for (auto const& exchangeDefintions : ExchangeDefinition()) {
        for (auto const& dpPair : exchangeDefintions.second) {
            std::shared_ptr<DataExchangeDefinition> dp = dpPair.second;

            if (dp) {
                if (dp->label == label) {
                    return dp;
                }
            }
        }
    }

    return nullptr;
}

void IEC104ClientConfig::importExchangeConfig(const string& exchangeConfig)
{
    std::string beforeLog = Iec104Utility::PluginName + " - IEC104ClientConfig::importExchangeConfig -"; //LCOV_EXCL_LINE
    m_exchangeConfigComplete = false;

    deleteExchangeDefinitions();

    Document document;

    if (document.Parse(const_cast<char*>(exchangeConfig.c_str())).HasParseError()) {
        Iec104Utility::log_fatal("%s Parsing error in exchanged_data json, offset %u: %s", beforeLog.c_str(), //LCOV_EXCL_LINE
                                static_cast<unsigned>(document.GetErrorOffset()), GetParseError_En(document.GetParseError()));
        return;
    }

    if (!document.IsObject()) {
        Iec104Utility::log_fatal("%s Root is not an object", beforeLog.c_str()); //LCOV_EXCL_LINE
        return;
    }

    if (!document.HasMember(JSON_EXCHANGED_DATA) || !document[JSON_EXCHANGED_DATA].IsObject()) {
        Iec104Utility::log_fatal("%s %s does not exist or is not an object", beforeLog.c_str(), JSON_EXCHANGED_DATA); //LCOV_EXCL_LINE
        return;
    }

    const Value& exchangeData = document[JSON_EXCHANGED_DATA];

    if (!exchangeData.HasMember(JSON_DATAPOINTS) || !exchangeData[JSON_DATAPOINTS].IsArray()) {
        Iec104Utility::log_fatal("%s %s does not exist or is not an array", beforeLog.c_str(), JSON_DATAPOINTS); //LCOV_EXCL_LINE
        return;
    }

    const Value& datapoints = exchangeData[JSON_DATAPOINTS];

    for (const Value& datapoint : datapoints.GetArray()) {

        if (!datapoint.IsObject()) {
            Iec104Utility::log_error("%s %s element is not an object", beforeLog.c_str(), JSON_DATAPOINTS); //LCOV_EXCL_LINE
            return;
        } 

        if (!datapoint.HasMember(JSON_LABEL) || !datapoint[JSON_LABEL].IsString()) {
            Iec104Utility::log_error("%s %s does not exist or is not a string", beforeLog.c_str(), JSON_LABEL); //LCOV_EXCL_LINE
            return;
        }

        string label = datapoint[JSON_LABEL].GetString();


        bool isGiTriggeringTs = false;
        if (datapoint.HasMember(JSON_PIVOT_SUBTYPES) && datapoint[JSON_PIVOT_SUBTYPES].IsArray()) {
            for (const Value &subtype : datapoint[JSON_PIVOT_SUBTYPES].GetArray()) {
                if (subtype.IsString() && subtype.GetString() == string(JSON_TRIGGER_SOUTH_GI_PIVOT_SUBTYPE)) {
                    isGiTriggeringTs = true;
                    break; //LCOV_EXCL_LINE
                }
            }
        }

        if (!datapoint.HasMember(JSON_PROTOCOLS) || !datapoint[JSON_PROTOCOLS].IsArray()) {
            Iec104Utility::log_error("%s %s does not exist or is not an array", beforeLog.c_str(), JSON_PROTOCOLS); //LCOV_EXCL_LINE
            return;
        }

        for (const Value& protocol : datapoint[JSON_PROTOCOLS].GetArray()) {
            
            if (!protocol.IsObject()) {
                Iec104Utility::log_error("%s %s element is not an object", beforeLog.c_str(), JSON_PROTOCOLS); //LCOV_EXCL_LINE
                return;
            } 
            
            if (!protocol.HasMember(JSON_PROT_NAME) || !protocol[JSON_PROT_NAME].IsString()) {
                Iec104Utility::log_error("%s %s does not exist or is not a string", beforeLog.c_str(), JSON_PROT_NAME); //LCOV_EXCL_LINE
                return;
            }
            
            string protocolName = protocol[JSON_PROT_NAME].GetString();

            if (protocolName == PROTOCOL_IEC104) {
                int giGroups = 0;

                if (!protocol.HasMember(JSON_PROT_ADDR) || !protocol[JSON_PROT_ADDR].IsString()) {
                    Iec104Utility::log_error("%s %s does not exist or is not a string", beforeLog.c_str(), JSON_PROT_ADDR); //LCOV_EXCL_LINE
                    return;
                }
                if (!protocol.HasMember(JSON_PROT_TYPEID) || !protocol[JSON_PROT_TYPEID].IsString()) {
                    Iec104Utility::log_error("%s %s does not exist or is not a string", beforeLog.c_str(), JSON_PROT_TYPEID); //LCOV_EXCL_LINE
                    return;
                }

                string address = protocol[JSON_PROT_ADDR].GetString();
                string typeIdStr = protocol[JSON_PROT_TYPEID].GetString();

                if (protocol.HasMember(JSON_PROT_GI_GROUPS)) {
                    if(protocol[JSON_PROT_GI_GROUPS].IsString()) {
                        string giGroupsStr = protocol["gi_groups"].GetString();

                        vector<string> tokens = tokenizeString(giGroupsStr, " ");

                        for (string token : tokens) {
                            if (token == "station") {
                                giGroups = 1;
                            }
                        }
                    } else {
                        Iec104Utility::log_warn("%s %s value is not a string", beforeLog.c_str(), //LCOV_EXCL_LINE
                                                JSON_PROT_GI_GROUPS);
                    }
                }

                Iec104Utility::log_debug("%s GI GROUPS = %i", beforeLog.c_str(), giGroups); //LCOV_EXCL_LINE

                size_t sepPos = address.find("-");

                if (sepPos != std::string::npos) {
                    std::string caStr = address.substr(0, sepPos);
                    std::string ioaStr = address.substr(sepPos + 1);

                    int ca = 0;
                    int ioa = 0;
                    try {
                        ca = std::stoi(caStr);
                        ioa = std::stoi(ioaStr);
                    } catch (const std::invalid_argument &e) {
                        Iec104Utility::log_error("%s  Cannot convert ca '%s' or ioa '%s' to integer: %s", //LCOV_EXCL_LINE
                                                beforeLog.c_str(), caStr.c_str(), ioaStr.c_str(), e.what());
                        return;
                    } catch (const std::out_of_range &e) {
                        Iec104Utility::log_error("%s  Cannot convert ca '%s' or ioa '%s' to integer: %s", //LCOV_EXCL_LINE
                                                beforeLog.c_str(), caStr.c_str(), ioaStr.c_str(), e.what());
                        return;
                    }

                    auto def = std::make_shared<DataExchangeDefinition>();

                    if (def) {
                        def->ca = ca;
                        def->ioa = ioa;
                        def->label = label;
                        def->typeId = IEC104ClientConfig::getTypeIdFromString(typeIdStr);
                        def->giGroups = giGroups;

                        Iec104Utility::log_debug("%s  Added exchange data %i:%i type: %i (%s)", beforeLog.c_str(), ca, ioa, def->typeId, //LCOV_EXCL_LINE
                                                typeIdStr.c_str());
                        ExchangeDefinition()[ca][ioa] = def;

                        if (isGiTriggeringTs) {
                            Iec104Utility::log_debug("Adding TS %s with ca %d ioa %d to GI triggering one", def->label, def->ca, def->ioa); //LCOV_EXCL_LINE
                            m_cgTriggeringTsAdresses.insert(make_pair(ca, ioa));
                        }
                    }
                } else {
                    Iec104Utility::log_error("%s  %s value does not follow format 'XXX-YYY': %s", beforeLog.c_str(), JSON_PROT_ADDR, //LCOV_EXCL_LINE
                                            address.c_str());
                    return;
                }
            }
        }
    }

    for (auto& element : ExchangeDefinition()) {
        int ca = element.first;

        if (std::find(m_listOfCAs.begin(), m_listOfCAs.end(), ca) == m_listOfCAs.end()) {
            m_listOfCAs.push_back(ca);
        }
    }

    m_exchangeConfigComplete = true;
}

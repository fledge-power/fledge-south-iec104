/*
 * FledgePower IEC104 <-> pivot filter utility functions.
 *
 * Copyright (c) 2022, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Michael Zillgith (michael.zillgith at mz-automation.de)
 * 
 */

#ifndef _IEC104_UTILITY_H
#define _IEC104_UTILITY_H

#include <string>
#include <logger.h>
#include <audit_logger.h>

#define PLUGIN_NAME "iec104"

namespace Iec104Utility {

    static const std::string PluginName = PLUGIN_NAME;

    /*
     * Log helper function that will log both in the Fledge syslog file and in stdout for unit tests
     */
    template<class... Args>
    void log_debug(const std::string& format, Args&&... args) {  
        #ifdef UNIT_TEST
        printf(std::string(format).append("\n").c_str(), std::forward<Args>(args)...);
        fflush(stdout);
        #endif
        Logger::getLogger()->debug(format.c_str(), std::forward<Args>(args)...);
    }

    template<class... Args>
    void log_info(const std::string& format, Args&&... args) {    
        #ifdef UNIT_TEST
        printf(std::string(format).append("\n").c_str(), std::forward<Args>(args)...);
        fflush(stdout);
        #endif
        Logger::getLogger()->info(format.c_str(), std::forward<Args>(args)...);
    }

    template<class... Args>
    void log_warn(const std::string& format, Args&&... args) { 
        #ifdef UNIT_TEST  
        printf(std::string(format).append("\n").c_str(), std::forward<Args>(args)...);
        fflush(stdout);
        #endif
        Logger::getLogger()->warn(format.c_str(), std::forward<Args>(args)...);
    }

    template<class... Args>
    void log_error(const std::string& format, Args&&... args) {   
        #ifdef UNIT_TEST
        printf(std::string(format).append("\n").c_str(), std::forward<Args>(args)...);
        fflush(stdout);
        #endif
        Logger::getLogger()->error(format.c_str(), std::forward<Args>(args)...);
    }

    template<class... Args>
    void log_fatal(const std::string& format, Args&&... args) {  
        #ifdef UNIT_TEST
        printf(std::string(format).append("\n").c_str(), std::forward<Args>(args)...);
        fflush(stdout);
        #endif
        Logger::getLogger()->fatal(format.c_str(), std::forward<Args>(args)...);
    }

    void audit_fail(const std::string& code, const std::string& data) {  
        AuditLogger::getLogger()->auditLog(code.c_str(), "FAILURE", format.c_str());
    }

    void audit_success(const std::string& code, const std::string& data) {  
        AuditLogger::getLogger()->auditLog(code.c_str(), "SUCCESS", format.c_str());
    }

    void audit_warn(const std::string& code, const std::string& data) {  
        AuditLogger::getLogger()->auditLog(code.c_str(), "WARNING", format.c_str());
    }

    void audit_info(const std::string& code, const std::string& data) {  
        AuditLogger::getLogger()->auditLog(code.c_str(), "INFORMATION", format.c_str());
    }
}

#endif /* _IEC104_UTILITY_H */

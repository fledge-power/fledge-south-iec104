#include <gtest/gtest.h>

#include "iec104_utility.h"

TEST(PivotIEC104PluginUtility, Logs)
{
    std::string text("This message is at level %s");
    ASSERT_NO_THROW(Iec104Utility::log_debug(text.c_str(), "debug"));
    ASSERT_NO_THROW(Iec104Utility::log_info(text.c_str(), "info"));
    ASSERT_NO_THROW(Iec104Utility::log_warn(text.c_str(), "warning"));
    ASSERT_NO_THROW(Iec104Utility::log_error(text.c_str(), "error"));
    ASSERT_NO_THROW(Iec104Utility::log_fatal(text.c_str(), "fatal"));
}

TEST(PivotIEC104PluginUtility, Audit)
{
    std::string text{"This audit is of type "};
    std::string jsonAudit{"{}"};
    ASSERT_NO_THROW(Iec104Utility::audit_success("SRVFL", text + "success"));
    ASSERT_NO_THROW(Iec104Utility::audit_info("SRVFL", text + "info"));
    ASSERT_NO_THROW(Iec104Utility::audit_warn("SRVFL", text + "warn"));
    ASSERT_NO_THROW(Iec104Utility::audit_fail("SRVFL", text + "fail"));
    ASSERT_NO_THROW(Iec104Utility::audit_success("SRVFL", jsonAudit, false));
    ASSERT_NO_THROW(Iec104Utility::audit_info("SRVFL", jsonAudit, false));
    ASSERT_NO_THROW(Iec104Utility::audit_warn("SRVFL", jsonAudit, false));
    ASSERT_NO_THROW(Iec104Utility::audit_fail("SRVFL", jsonAudit, false));
}
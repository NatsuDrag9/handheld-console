/*
 * error_utils.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */
#include "Utils/comm_utils.h"


/**
 * Translates ESP32 AT command error codes into human-readable messages
 * @param error_response The AT command error response string
 * @return A human-readable error message
 */
const char* translate_at_error(const char* error_response) {
    // WiFi connection errors (AT+CWJAP)
    if (strstr(error_response, "+CWJAP:1"))
        return "WiFi connection timeout - AP not responding";
    if (strstr(error_response, "+CWJAP:2"))
        return "WiFi connection failed - Wrong password";
    if (strstr(error_response, "+CWJAP:3"))
        return "WiFi connection failed - AP not found";
    if (strstr(error_response, "+CWJAP:4"))
        return "WiFi connection failed - Connection error (incorrect credentials)";
    if (strstr(error_response, "+CWJAP:5"))
        return "WiFi connection failed - Connection refused by AP";
    if (strstr(error_response, "+CWJAP:6"))
        return "WiFi connection failed - Unknown/unhandled error";

    // TCP connection errors (AT+CIPSTART)
    if (strstr(error_response, "ALREADY CONNECTED"))
        return "TCP connection error - Connection already exists";
    if (strstr(error_response, "CONNECT FAIL"))
        return "TCP connection error - Connection failed";
    if (strstr(error_response, "IP NEGATIVE"))
        return "TCP connection error - Invalid IP address";
    if (strstr(error_response, "CLOSED"))
        return "TCP connection was closed";

    // IP errors (AT+CIPSTATUS)
    if (strstr(error_response, "+CIPSTATUS:0"))
        return "IP status - ESP32 station not initialized";
    if (strstr(error_response, "+CIPSTATUS:1"))
        return "IP status - ESP32 station initialized but not connected to AP";
    if (strstr(error_response, "+CIPSTATUS:2"))
        return "IP status - ESP32 station connected to AP but no TCP connection";
    if (strstr(error_response, "+CIPSTATUS:4"))
        return "IP status - ESP32 station disconnected from AP";
    if (strstr(error_response, "+CIPSTATUS:5"))
        return "IP status - ESP32 station not connected to AP";

    // Sending data errors
    if (strstr(error_response, "SEND FAIL"))
        return "Data transmission failed";

    // Generic AT errors
    if (strstr(error_response, "ERROR"))
        return "Generic AT command error - Command not recognized or invalid parameters";
    if (strstr(error_response, "busy"))
        return "AT system busy - Previous command still processing";

    // Default case - unknown error
    return "Unknown AT error response";
}

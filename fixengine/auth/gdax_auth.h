#ifndef LIBTRADING_AUTH_H
#define LIBTRADING_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

void LogonMsgGen(const char *time, const char *sender_id, const char *target_id, const char *password, const char *api_secret, char *msg);

#ifdef __cplusplus
}
#endif

#endif

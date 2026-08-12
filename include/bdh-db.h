#ifndef BDH_DB_H
#define BDH_DB_H

#include <libpq-fe.h>

// டேட்டாபேஸ் கனெக்ஷனைத் துவக்க
PGconn* db_connect(const char *conninfo);

// SQL குவரியை (Query) ரன் செய்து ரிசல்ட்டைப் பெற
void db_execute_query(PGconn *conn, const char *query);

// டேட்டாபேஸ் கனெக்ஷனைப் பாதுகாப்பாக மூட
void db_close(PGconn *conn);

#endif // BDH_DB_H

#include <stdio.h>
#include <stdlib.h>
#include "../include/bdh-db.h"

// 1. டேட்டாபேஸ் கனெக்ஷனை உருவாக்கும் ஃபங்ஷன்
PGconn* db_connect(const char *conninfo) {
    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "\033[1;31m[DB Error] Connection failed: %s\033[0m\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    
    return conn;
}

// 2. SQL குவரியை ரன் செய்து அவுட்புட்டைக் காட்டும் ஃபங்ஷன்
void db_execute_query(PGconn *conn, const char *query) {
    if (conn == NULL) {
        printf("\033[1;31m[DB Error] No active database connection.\033[0m\n");
        return;
    }

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "\033[1;31m[DB Error] Query failed: %s\033[0m\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    // ரிசல்ட்டில் உள்ள ரோ (Rows) மற்றும் காலம் (Columns) எண்ணிக்கையைக் கண்டுபிடித்தல்
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    printf("\033[1;34m--- Query Results ---\033[0m\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%s\t", PQgetvalue(res, i, j));
        }
        printf("\n");
    }
    printf("\033[1;34m---------------------\033[0m\n");

    PQclear(res);
}

// 3. கனெக்ஷனைத் துண்டிக்கும் ஃபங்ஷன்
void db_close(PGconn *conn) {
    if (conn != NULL) {
        PQfinish(conn);
        printf("\033[1;32m[DB] Connection closed cleanly.\033[0m\n");
    }
}

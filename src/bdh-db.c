#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h> // PostgreSQL-க்கான அதிகாரப்பூர்வ C ஹெடர் ஃபைல்

// ஏதேனும் எரர் வந்தால் மெமரியை க்ளீன் செய்துவிட்டு வெளியேற
void exit_nicely(PGconn *conn) {
    PQfinish(conn);
    exit(1);
}

int main() {
    printf("\033[1;36m[ BDH-DB Engine ] Initializing Native C Connection...\033[0m\n");

    // 1. Connection String: உங்கள் லோக்கல் டேட்டாபேஸ் பெயர் மற்றும் யூசர் (தேவைப்பட்டால் மாற்றிக்கொள்ளலாம்)
    const char *conninfo = "dbname=postgres user=postgres";

    // 2. டேட்டாபேஸுடன் TCP Socket வழியாக நேரடியாக கனெக்ட் செய்தல்
    PGconn *conn = PQconnectdb(conninfo);

    // 3. கனெக்ஷன் ஸ்டேட்டஸை செக் செய்தல்
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "\033[1;31m[Error] Connection to database failed: %s\033[0m\n", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    printf("\033[1;32m[Success] Connected to PostgreSQL!\033[0m\n");

    // 4. SQL கமாண்டை C-ப்ரோக்ராம் மூலமாக சர்வெருக்கு அனுப்புதல்
    // இங்கே நாம் சர்வரின் வெர்ஷனை (Version) கேட்கிறோம்
    PGresult *res = PQexec(conn, "SELECT version();");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "\033[1;31m[Error] SELECT command failed: %s\033[0m\n", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    // 5. டேட்டாபேஸ் அனுப்பிய ரிசல்ட்டைப் படித்து ஸ்கிரீனில் காட்டுதல்
    // PQgetvalue(result, row_number, column_number)
    printf("\n\033[1;33m[Server Info] ->\033[0m %s\n\n", PQgetvalue(res, 0, 0));

    // 6. மெமரியை க்ளீன் செய்து கனெக்ஷனைத் துண்டித்தல்
    PQclear(res);
    PQfinish(conn);

    return 0;
}

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int testMemoryDatabasePerformance(int nameLength, int descriptionLength, int inMemory) {
    int rc;
    sqlite3 *db;

    if (inMemory == 0) {
        rc = sqlite3_open("test.db", &db);
    } else {
        rc = sqlite3_open(":memory:", &db);
    }

    if (rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    char *createSQL = "CREATE TABLE Rec (id integer PRIMARY KEY, name varchar(8), desc varchar(15));";
    sqlite3_stmt *createStatement;
    rc = sqlite3_prepare_v2(db, createSQL, strlen(createSQL), &createStatement, NULL);
    sqlite3_bind_int(createStatement, 0, nameLength);
    sqlite3_bind_int(createStatement, 1, descriptionLength);

    rc = sqlite3_step(createStatement);
    if (rc != SQLITE_DONE) {
            fprintf(stderr, "Database create table error: %s\n", sqlite3_errmsg(db));
            return 1;
    }
    sqlite3_reset(createStatement);

    sqlite3_stmt *stmt;
    char *sql = "INSERT INTO Rec(name, desc) VALUES(?1, ?2)";
    rc = sqlite3_prepare_v2(db,sql,strlen(sql), &stmt, NULL);

    int i;
    int MAX = 100;

    clock_t start = clock();

    for (i = 0; i < MAX; i++) {
        sqlite3_bind_int(stmt, 0, i);
        sqlite3_bind_text(stmt, 1, "testName", strlen("testName") + 1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, "testDescription", strlen("testDescription") + 1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Database insert error \n");
        }
        sqlite3_reset(stmt);
    }

    printf("czas wypełniania danymi[s]: %f\n", ((float)clock()-(float)start)/CLOCKS_PER_SEC);

    start = clock();

    rc = sqlite3_exec(db, "SELECT * FROM Rec WHERE id = 999999", callback, NULL, NULL);
    if (rc){
        fprintf(stderr, "Database select error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    printf("czas wyszukania rekordu[s]: %f\n", ((float)clock()-(float)start)/CLOCKS_PER_SEC);

    sqlite3_finalize(stmt);

    if (inMemory == 0) {
        sqlite3_exec(db, "DROP TABLE Rec", NULL, NULL, NULL);
    }

    sqlite3_close(db);
    return 0;
}

int main() {

    printf("\n (8,15) database in memory \n ======================= \n");
    testMemoryDatabasePerformance(8, 15, 1);
    printf("\n (18, 80) database in memory \n ======================= \n");
    testMemoryDatabasePerformance(18, 80, 1);
    printf("\n (8, 15) database in file \n ======================= \n");
    testMemoryDatabasePerformance(8, 15, 0);
    printf("\n (18, 80) database in file \n ======================= \n");
    testMemoryDatabasePerformance(18, 80, 0);

    return 0;
}



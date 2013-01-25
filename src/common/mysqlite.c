#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "mysqlite.h"


// variable globale
sqlite3 *database;
sqlite3_stmt *stmt;	


/** connection la base de donnees **/
int connect_server_database(const char *fileDb) {
	
	// creation de la base de donnée si existe pas sinon ouverture de la base
	int open = sqlite3_open(fileDb, &database);
	if (open != SQLITE_OK) {
		perror("Database connection failed\n");
		sqlite3_close(database);
		return -1;
	}
	
	// creation de la table users
	char create_table[QUERY_SIZE] = "CREATE TABLE IF NOT EXISTS users (login VARCHAR(20))";
	int sql = sqlite3_exec(database, create_table, 0, 0, 0);
	if (sql != SQLITE_OK) {
		perror("Cant create table users\n");
		return -1;
    }
    printf("connect_server: table users cree: test2\n");//--------------------
	
	return 1;
}


/** fermer la base de données */	
void close_server_database() {
	sqlite3_close(database);
}


/** ajoute un user dans la base **/
int add_user(char *login) {
	
	// creation de la requete insertion
	char insert[QUERY_SIZE] = "INSERT INTO users values ('";
	strcat(insert, login);
	strcat(insert, "')");

	int sql = sqlite3_exec(database, insert, 0, 0, 0);
	printf("add_user: sql_insertion: %d\nadd_user: test4\n", sql);//--------------------
	if (sql != SQLITE_OK) {
		perror("Cant insert in server database");
		return -1;
    } else {
		printf("adding user succesfull\n");
	}
    printf("add_user: test5\n");//--------------------
    return 1;
}


/** supprime un user de la base **/
int delete_user (char *login) { 
    
	char delete[QUERY_SIZE] = "DELETE FROM users WHERE login = '";
	strcat(delete, login);
	strcat(delete, "'");
	
	int sql = sqlite3_exec(database, delete, 0, 0, 0);
	if (sql != SQLITE_OK) {
		perror("Cant check in server database\n");
		return -1;
    } else {
		printf("user delete succesfull\n");
	}
    
    return 1;
}




int check_user(char *login) {
	
	char select[QUERY_SIZE] = "SELECT * FROM users WHERE login = '";
	strcat(select, login);
	strcat(select, "'");
	
	int sql = sqlite3_prepare_v2(database, select, -1, &stmt, 0);
	int cols=sqlite3_column_count(stmt);
	printf("colonne avant : %d et statement %s\n", cols, stmt);
	if (sql) {
		perror("Cant select in server database\n");
		return -1;
    } else {
		
		printf("colonne : %d\n", cols);
		if (cols > 1){
			printf("select succesfull\n");
		return 1;}
		else{
		printf("colonne pas superieur");	
		return -1;}
		
		
		
	}
	
    
}



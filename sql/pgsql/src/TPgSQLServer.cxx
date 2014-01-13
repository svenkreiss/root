// @(#)root/pgsql:$Id$
// Author: g.p.ciceri <gp.ciceri@acm.org> 01/06/2001

/*************************************************************************
 * Copyright (C) 1995-2001, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TPgSQLServer.h"
#include "TPgSQLResult.h"
#include "TPgSQLStatement.h"

#include "TSQLColumnInfo.h"
#include "TSQLTableInfo.h"
#include "TSQLRow.h"
#include "TUrl.h"
#include "TList.h"
#include "TObjString.h"
#include "TObjArray.h"


ClassImp(TPgSQLServer)

//______________________________________________________________________________
TPgSQLServer::TPgSQLServer(const char *db, const char *uid, const char *pw)
{
   // Open a connection to a PgSQL DB server. The db arguments should be
   // of the form "pgsql://<host>[:<port>][/<database>]", e.g.:
   // "pgsql://pcroot.cern.ch:3456/test". The uid is the username and pw
   // the password that should be used for the connection.

   fPgSQL = 0;
   fSrvInfo="";

   TUrl url(db);

   if (!url.IsValid()) {
      Error("TPgSQLServer", "malformed db argument %s", db);
      MakeZombie();
      return;
   }

   if (strncmp(url.GetProtocol(), "pgsql", 5)) {
      Error("TPgSQLServer", "protocol in db argument should be pgsql it is %s",
            url.GetProtocol());
      MakeZombie();
      return;
   }

   const char *dbase = url.GetFile();

   if (url.GetPort()) {
      TString port;
      port += url.GetPort();
      fPgSQL = PQsetdbLogin(url.GetHost(), port, 0, 0, dbase, uid, pw);
   } else {
      fPgSQL = PQsetdbLogin(url.GetHost(), 0, 0, 0, dbase, uid, pw);
   }

   if (PQstatus(fPgSQL) != CONNECTION_BAD) {
      fType = "PgSQL";
      fHost = url.GetHost();
      fDB   = dbase;
      fPort = url.GetPort();

      // Populate server-info
      fSrvInfo = "postgres ";
      static const char *sql = "select setting from pg_settings where name='server_version'";
      PGresult *res = PQexec(fPgSQL, sql);
      int stat = PQresultStatus(res);
      if (stat == PGRES_TUPLES_OK && PQntuples(res)) {
         char *vers = PQgetvalue(res,0,0);
         fSrvInfo += vers;
         PQclear(res);
      } else {
         fSrvInfo += "unknown version number";
      }
   } else {
      Error("TPgSQLServer", "connection to %s failed", url.GetHost());
      MakeZombie();
   }
}

//______________________________________________________________________________
TPgSQLServer::~TPgSQLServer()
{
   // Close connection to PgSQL DB server.

   if (IsConnected())
      Close();
}

//______________________________________________________________________________
void TPgSQLServer::Close(Option_t *)
{
   // Close connection to PgSQL DB server.

   if (!fPgSQL)
      return;

   PQfinish(fPgSQL);
   fPort = -1;
}

//______________________________________________________________________________
TSQLResult *TPgSQLServer::Query(const char *sql)
{
   // Execute SQL command. Result object must be deleted by the user.
   // Returns a pointer to a TSQLResult object if successful, 0 otherwise.
   // The result object must be deleted by the user.

   if (!IsConnected()) {
      Error("Query", "not connected");
      return 0;
   }

   PGresult *res = PQexec(fPgSQL, sql);

   if ((PQresultStatus(res) != PGRES_COMMAND_OK) &&
       (PQresultStatus(res) != PGRES_TUPLES_OK)) {
      Error("Query", "%s",PQresultErrorMessage(res));
      PQclear(res);
      return 0;
   }

   return new TPgSQLResult(res);
}

//______________________________________________________________________________
Int_t TPgSQLServer::SelectDataBase(const char *dbname)
{
   // Select a database. Returns 0 if successful, non-zero otherwise.

   TString usr;
   TString pwd;
   TString port;
   TString opts;

   if (!IsConnected()) {
      Error("SelectDataBase", "not connected");
      return -1;
   }

   if (dbname == fDB) {
      return 0;
   } else {
      usr = PQuser(fPgSQL);
      pwd = PQpass(fPgSQL);
      port = PQport(fPgSQL);
      opts = PQoptions(fPgSQL);

      Close();
      fPgSQL = PQsetdbLogin(fHost.Data(), port.Data(),
                            opts.Data(), 0, dbname,
                            usr.Data(), pwd.Data());

      if (PQstatus(fPgSQL) == CONNECTION_OK) {
         fDB=dbname;
         fPort=port.Atoi();
      } else {
         Error("SelectDataBase", "%s",PQerrorMessage(fPgSQL));
         return -1;
      }
   }
   return 0;
}

//______________________________________________________________________________
TSQLResult *TPgSQLServer::GetDataBases(const char *wild)
{
   // List all available databases. Wild is for wildcarding "t%" list all
   // databases starting with "t".
   // Returns a pointer to a TSQLResult object if successful, 0 otherwise.
   // The result object must be deleted by the user.

   if (!IsConnected()) {
      Error("GetDataBases", "not connected");
      return 0;
   }

   TString sql = "SELECT pg_database.datname FROM pg_database";
   if (wild)
      sql += Form(" WHERE pg_database.datname LIKE '%s'", wild);

   return Query(sql);
}

//______________________________________________________________________________
TSQLResult *TPgSQLServer::GetTables(const char *dbname, const char *wild)
{
   // List all tables in the specified database. Wild is for wildcarding
   // "t%" list all tables starting with "t".
   // Returns a pointer to a TSQLResult object if successful, 0 otherwise.
   // The result object must be deleted by the user.

   if (!IsConnected()) {
      Error("GetTables", "not connected");
      return 0;
   }

   if (SelectDataBase(dbname) != 0) {
      Error("GetTables", "no such database %s", dbname);
      return 0;
   }

   TString sql = "SELECT relname FROM pg_class where relkind='r'";
   if (wild)
      sql += Form(" AND relname LIKE '%s'", wild);

   return Query(sql);
}

//______________________________________________________________________________
TSQLResult *TPgSQLServer::GetColumns(const char *dbname, const char *table,
                                     const char *wild)
{
   // List all columns in specified table in the specified database.
   // Wild is for wildcarding "t%" list all columns starting with "t".
   // Returns a pointer to a TSQLResult object if successful, 0 otherwise.
   // The result object must be deleted by the user.

   if (!IsConnected()) {
      Error("GetColumns", "not connected");
      return 0;
   }

   if (SelectDataBase(dbname) != 0) {
      Error("GetColumns", "no such database %s", dbname);
      return 0;
   }

   char *sql;
   if (wild)
      sql = Form("select a.attname,t.typname,a.attnotnull \
                  from pg_attribute a, pg_class c, pg_type t \
                  where c.oid=a.attrelid and c.relname='%s' and \
                  a.atttypid=t.oid and a.attnum>0 \
                  and a.attname like '%s' order by a.attnum ", table,wild);
   else
      sql = Form("select a.attname,t.typname,a.attnotnull \
                  from pg_attribute a, pg_class c, pg_type t \
                  where c.oid=a.attrelid and c.relname='%s' and \
                  a.atttypid=t.oid and a.attnum>0 order by a.attnum",table);

   return Query(sql);
}

//______________________________________________________________________________
Int_t TPgSQLServer::CreateDataBase(const char *dbname)
{
   // Create a database. Returns 0 if successful, non-zero otherwise.

   if (!IsConnected()) {
      Error("CreateDataBase", "not connected");
      return -1;
   }
   char *sql;
   sql = Form("CREATE DATABASE %s", dbname);
   PGresult *res = PQexec(fPgSQL, sql);
   PQclear(res);
   return 0;
}

//______________________________________________________________________________
Int_t TPgSQLServer::DropDataBase(const char *dbname)
{
   // Drop (i.e. delete) a database. Returns 0 if successful, non-zero
   // otherwise.

   if (!IsConnected()) {
      Error("DropDataBase", "not connected");
      return -1;
   }
   char *sql;
   sql = Form("DROP DATABASE %s", dbname);
   PGresult *res = PQexec(fPgSQL, sql);
   PQclear(res);
   return 0;
}

//______________________________________________________________________________
Int_t TPgSQLServer::Reload()
{
   // Reload permission tables. Returns 0 if successful, non-zero
   // otherwise. User must have reload permissions.

   if (!IsConnected()) {
      Error("Reload", "not connected");
      return -1;
   }

   Error("Reload", "not implemented");
   return 0;
}

//______________________________________________________________________________
Int_t TPgSQLServer::Shutdown()
{
   // Shutdown the database server. Returns 0 if successful, non-zero
   // otherwise. User must have shutdown permissions.

   if (!IsConnected()) {
      Error("Shutdown", "not connected");
      return -1;
   }

   Error("Shutdown", "not implemented");
   return 0;
}

//______________________________________________________________________________
const char *TPgSQLServer::ServerInfo()
{
   // Return server info.

   if (!IsConnected()) {
      Error("ServerInfo", "not connected");
      return 0;
   }

   return fSrvInfo.Data();
}

//______________________________________________________________________________
Bool_t TPgSQLServer::HasStatement() const
{
   // PG_VERSION_NUM conveniently only started being #defined at 8.2.3
   // which is the first version of libpq which explicitly supports prepared
   // statements

#ifdef PG_VERSION_NUM
   return kTRUE;
#else
   return kFALSE;
#endif
}

//______________________________________________________________________________
#ifdef PG_VERSION_NUM
TSQLStatement* TPgSQLServer::Statement(const char *sql, Int_t)
#else
TSQLStatement* TPgSQLServer::Statement(const char *, Int_t)
#endif
{
  // Produce TPgSQLStatement.

#ifdef PG_VERSION_NUM
   if (!sql || !*sql) {
      SetError(-1, "no query string specified","Statement");
      return 0;
   }

   PgSQL_Stmt_t *stmt = new PgSQL_Stmt_t;
   if (!stmt){
      SetError(-1, "cannot allocate PgSQL_Stmt_t", "Statement");
      return 0;
   }
   stmt->fConn = fPgSQL;
   stmt->fRes  = PQprepare(fPgSQL, "preparedstmt", sql, 0, (const Oid*)0);

   ExecStatusType stat = PQresultStatus(stmt->fRes);
   if (pgsql_success(stat)) {
      fErrorOut = stat;
      return new TPgSQLStatement(stmt, fErrorOut);
   } else {
      SetError(stat, PQresultErrorMessage(stmt->fRes), "Statement");
      stmt->fConn = 0;
      delete stmt;
      return 0;
   }
#else
   Error("Statement", "not implemented for pgsql < 8.2");
#endif
   return 0;
}

#if 0
//______________________________________________________________________________
TSQLTableInfo *TPgSQLServer::GetTableInfo(const char *tablename)
{
   if (!IsConnected()) {
      Error("GetColumns", "not connected");
      return 0;
   }

   // Check table name
   if ((tablename == 0) || (*tablename == 0)) return 0;

   // Query first row ( works same way as MySQL)
   TString sql;
   sql.Form("SELECT * FROM %s LIMIT 1;", tablename);
   PGresult *res = PQexec(fPgSQL, sql);

   if ((PQresultStatus(res) != PGRES_COMMAND_OK) &&
       (PQresultStatus(res) != PGRES_TUPLES_OK)) {
      Error("Query", "%s", PQresultErrorMessage(res));
      PQclear(res);
      return 0;
   }

   TList *lst = 0;
   Int_t sqltype = kSQL_NONE;
   Int_t data_size = -1;    // size in bytes
   Int_t data_length = -1;  // declaration like VARCHAR(n) or NUMERIC(n)
   Int_t data_scale = -1;   // second argument in declaration
   Int_t data_sign = -1;    // signed type or not
   Bool_t nullable = 0;

   Int_t nfields  = PQnfields(res);
   Int_t ibin     = PQbinaryTuples(res);

   for (Int_t col = 0; col < nfields; col++) {

      const char *column_name = PQfname(res, col);
      const char *type_name;
      int   imod     = PQfmod(res, col);
      int   isize    = PQfsize(res, col);

      switch (PQftype(res, col)) {
         case INT2OID:
            sqltype = kSQL_INTEGER;
            type_name = "INT";
            data_size = 2;
            break;
         case INT4OID:
            sqltype = kSQL_INTEGER;
            type_name = "INT";
            data_size = 4;
            break;
         case INT8OID:
            sqltype = kSQL_INTEGER;
            type_name = "INT";
            data_size = 8;
            break;
         case FLOAT4OID:
            sqltype = kSQL_FLOAT;
            type_name = "FLOAT";
            data_size = 4;
            break;
         case FLOAT8OID:
            sqltype = kSQL_DOUBLE;
            type_name = "DOUBLE";
            data_size = 8;
            break;
         case BOOLOID:
            sqltype = kSQL_INTEGER;
            type_name = "BOOL";
            data_size = 4;
            break;
         case CHAROID:
            sqltype = kSQL_CHAR;
            type_name = "CHAR";
            data_size = 1;
            break;
         case NAMEOID:
            sqltype = kSQL_VARCHAR;
            type_name = "VARCHAR";
            data_size = imod;
            break;
         case TEXTOID:
            sqltype = kSQL_VARCHAR;
            type_name = "VARCHAR";
            data_size = imod;
            break;
         case VARCHAROID:
            sqltype = kSQL_VARCHAR;
            type_name = "VARCHAR";
            data_size = imod;
            break;
         case DATEOID:
            sqltype = kSQL_TIMESTAMP;
            type_name = "TIMESTAMP";
            data_size = 4;
            break;
         case TIMEOID:
            sqltype = kSQL_TIMESTAMP;
            type_name = "TIMESTAMP";
            data_size = 8;
            break;
         case TIMETZOID:
            sqltype = kSQL_TIMESTAMP;
            type_name = "TIMESTAMP";
            data_size = 8;
            break;
         case TIMESTAMPOID:
            sqltype = kSQL_TIMESTAMP;
            type_name = "TIMESTAMP";
            data_size = 8;
            break;
         case TIMESTAMPTZOID:
            sqltype = kSQL_TIMESTAMP;
            type_name = "TIMESTAMP";
            data_size = 8;
            break;
         case BYTEAOID:
            sqltype = kSQL_BINARY;
            type_name = "BINARY";
            break;
         default:
            sqltype = kSQL_NUMERIC;
            type_name = "NUMERIC";
            break;
      }

      if (!lst)
         lst = new TList;
      lst->Add(new TSQLColumnInfo(column_name,
                                  type_name,
                                  nullable,
                                  sqltype,
                                  data_size,
                                  data_length,
                                  data_scale,
                                  data_sign));
   } //! ( cols)

   PQclear(res);
   return (new TSQLTableInfo(tablename, lst));
}
#endif

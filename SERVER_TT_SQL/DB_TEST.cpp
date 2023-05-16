// SQLBindCol_ref.cpp  
// compile with: odbc32.lib  
#include <windows.h>  
#include <iostream>
#include <format>

#define UNICODE  
#include <sqlext.h>  

#define NAME_LEN 50  
#define PHONE_LEN 60

using namespace std;

void show_error() {
    printf("error\n");
}

int main() {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0;
    SQLRETURN retcode;
    SQLINTEGER user_id, user_exp;
    SQLWCHAR szName[NAME_LEN];
    SQLLEN cbuser_id = 0, cbszName = 0, cbuser_exp = 0;

    wcout.imbue(locale("korean"));
    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLWCHAR*)L"test", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
                
                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                    cout << "1 : SUCCESS" << endl;
                    //retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT user_id, user_name, exp FROM user_data", SQL_NTS);
                    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC over_exp 10000", SQL_NTS);

                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                        cout << "2 : SUCCESS" << endl;
                        // Bind columns 1, 2, and 3  
                        retcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &user_id, 12, &cbuser_id);
                        retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szName, 20, &cbszName);
                        retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &user_exp, 12, &cbuser_exp);

                        // Fetch and print each row of data. On an error, display a message and exit.  
                        for (int i = 0; ; i++) {
                            retcode = SQLFetch(hstmt);
                            if (retcode == SQL_ERROR)
                                show_error();
                            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
                            {
                                wcout << i + 1 << " :: " << user_id << " " << szName << " " << user_exp << endl;
                            }
                            else
                                break;
                        }
                    }
                    else {
                        cout << "2 : FAIL" << endl;
                    }

                    // Process data  
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                        SQLCancel(hstmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                    }

                    SQLDisconnect(hdbc);
                }
                else {
                    cout << "1 : FAIL" << endl;
                }

                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            }
        }
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}
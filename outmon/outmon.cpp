// outmon.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <output_monitor.h>


#define  ERROROUT(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)
#define  INFOOUT(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)


#define  FILE_CREATE    1
#define  FILE_APPEND    2

static char* st_pFile=NULL;
static int st_FileMode=FILE_CREATE;
static int st_Running=1;

BOOL WINAPI HandlerConsoleRoutine(DWORD dwCtrlType)
{
    BOOL bret=TRUE;
    switch(dwCtrlType)
    {
    case CTRL_C_EVENT:
        fprintf(stderr,"CTRL_C_EVENT\n");
        st_Running = 0;
        break;
    case CTRL_BREAK_EVENT:
        fprintf(stderr,"CTRL_BREAK_EVENT\n");
        st_Running = 0;
        break;
    case CTRL_CLOSE_EVENT:
        fprintf(stderr,"CTRL_CLOSE_EVENT\n");
        st_Running = 0;
        break;
    case CTRL_LOGOFF_EVENT:
        fprintf(stderr,"CTRL_LOGOFF_EVENT\n");
        st_Running = 0;
        break;
    case CTRL_SHUTDOWN_EVENT:
        fprintf(stderr,"CTRL_SHUTDOWN_EVENT\n");
        st_Running = 0;
        break;
    default:
        fprintf(stderr,"ctrltype %d\n",dwCtrlType);
        bret = FALSE;
        break;
    }

    return bret;
}

void Usage(int ec,const char* fmt,...)
{
    FILE* fp=stderr;
    va_list ap;
    if(ec == 0)
    {
        fp = stdout;
    }

    if(fmt)
    {
        va_start(ap,fmt);
        vfprintf(fp,fmt,ap);
    }

	fprintf(fp,"outmon [OPTIONS]\n");
	fprintf(fp,"\t-h|--help              to display this message\n");
	fprintf(fp,"\t-a|--append filename   to specify the file of output and append it\n");
	fprintf(fp,"\t-c|--create filename   to specify the file of output and create it\n");
	fprintf(fp,"default output is stdout\n");

    exit(ec);
}


int ParseParam(int argc,char* argv[])
{
    int i;
    int ret=0;

    for(i=1; i<argc; i++)
    {

    }
}



int main(int argc, char* argv[])
{
    BOOL bret;
    int ret;

    bret = SetConsoleCtrlHandler(HandlerConsoleRoutine,TRUE);
    if(!bret)
    {
        ret = GETERRNO();
        ERROROUT("SetControlCtrlHandler Error(%d)",ret);
        goto out;
    }


    ret = 0;

out:
    SETERRNO(ret);
    return -ret;
}


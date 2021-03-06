#include "ini.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <tchar.h>

#define BUFFER_SIZE_FOR_VARIABLE 80

using namespace ini;

File::File(std::string FileName)
{
    TCHAR szFileName[MAX_PATH], szPath[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    ExtractFilePath(szFileName, szPath);
    Dir = std::string(szPath);
    Name = FileName;
    Path = Dir + Name;
}
VOID File::Clear()
{
    DeleteFile(Path.c_str());
}
VOID File::Rename(std::string NewName)
{
    Name = NewName;
    Path = Dir + NewName;
}

VOID File::Redir(std::string NewDir)
{
    Dir = NewDir;
    Path = Dir + Name;;
}
/* ������� ������ */
BOOL File::ReadString(std::string section, std::string key, std::string *value, std::string default_value)
{
    char buff[BUFFER_SIZE_FOR_VARIABLE];
    BOOL status = GetPrivateProfileString(section.c_str(), key.c_str(), default_value.c_str(), buff, sizeof(buff), Path.c_str());
    *value = buff;
    return status;
}
BOOL File::ReadDouble(std::string section, std::string key, double *value, std::string default_value)
{
    char buff[BUFFER_SIZE_FOR_VARIABLE];
    BOOL status = GetPrivateProfileString(section.c_str(), key.c_str(), default_value.c_str(), buff, sizeof(buff), Path.c_str());
    *value = atof(buff);
    return status;
}
BOOL File::ReadInt(std::string section, std::string key, int *value, std::string default_value)
{
    char buff[BUFFER_SIZE_FOR_VARIABLE];
    BOOL status = GetPrivateProfileString(section.c_str(), key.c_str(), default_value.c_str(), buff, sizeof(buff), Path.c_str());
    *value = atoi(buff);
    return status;
}
BOOL File::ReadBool(std::string section, std::string key, bool *value, std::string default_value)
{
    char buff[BUFFER_SIZE_FOR_VARIABLE];
    BOOL status = GetPrivateProfileString(section.c_str(), key.c_str(), default_value.c_str(), buff, sizeof(buff), Path.c_str());
    *value = (atoi(buff) == 0? false : true);
    return status;
}
/* ������� ������ */
BOOL File::WriteString(std::string section, std::string key, std::string value)
{
    return WritePrivateProfileString(section.c_str(), key.c_str(), value.c_str(), Path.c_str());
}
BOOL File::WriteInt(std::string section, std::string key, int value)
{
    std::stringstream buff;
    buff << value;
    return WritePrivateProfileString(section.c_str(), key.c_str(), buff.str().c_str(), Path.c_str());
}
BOOL File::WriteBool(std::string section, std::string key, bool value)
{
    std::stringstream buff;
    buff << (value == true? 1 : 0);
    return WritePrivateProfileString(section.c_str(), key.c_str(), buff.str().c_str(), Path.c_str());
}
BOOL File::WriteDoubleSc(std::string section, std::string key, double value, int prec)
{
    std::stringstream buff;
    buff << std::scientific << std::setprecision(prec);
    return WriteDouble(section, key, value, &buff);
}
BOOL File::WriteDoubleFix(std::string section, std::string key, double value, int prec)
{
    std::stringstream buff;
    buff << std::fixed << std::setprecision(prec);
    return WriteDouble(section, key, value, &buff);
}
BOOL File::WriteDouble(std::string section, std::string key, double value, std::stringstream *buff)
{
    (*buff) << value;
    return WritePrivateProfileString(section.c_str(), key.c_str(), buff->str().c_str(), Path.c_str());
}

LPTSTR File::ExtractFilePath(LPCTSTR FileName, LPTSTR buf)
{
    int i, len = lstrlen(FileName);
    for(i=len-1; i>=0; i--)
    {
        if(FileName[i] == _T('\\'))
            break;
    }
    lstrcpyn(buf, FileName, i+2);
    return buf;
}

//压缩类，用于提供相应方法
#ifndef ZIP_H
#define ZIP_H
#include <fstream>
#include <QProgressDialog>

class ZIP
{
public:
    static void encode(const char* srcPath, const char* dstPath,QProgressDialog* progress);
    static void decode(const char* zipPath, const char *dstPath, QProgressDialog* progress);
    static char* getFileName(const char* path);
    static char* getZipFileName(const char* path);
    static bool checkZip(FILE* f);
    static bool UTF8ToUnicode(const char* UTF8, wchar_t* strUnicode);
private:
    ZIP();
};

#endif // ZIP_H

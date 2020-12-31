#include "zip.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <huffmanTree.h>
#include <string.h>
#include <map>
#include <deque>
#include <QDebug>
#include <windows.h>
#include <QProgressDialog>
#include <QCoreApplication>
#include <ctime>
#include <bits/stdc++.h>
//加个万能头以防万一嘻嘻

ZIP::ZIP()//构造函数
{

}

//char* srcPath: 被压缩的文件路径
//char* dstPath: 压缩到的文件路径
void ZIP::encode(const char* srcPath, const char* dstPath, QProgressDialog* progress)
{
    wchar_t wSrcPath[2048],wdstPath[2048];
    if(!UTF8ToUnicode(srcPath, wSrcPath) || !UTF8ToUnicode(dstPath, wdstPath))
        throw runtime_error("路径转换字符集失败！");
    FILE *fin = _wfopen(wSrcPath, L"rb");//被压缩的文件
    FILE *fout = _wfopen(wdstPath, L"wb");//压缩后的文件

    if(!fin)
    {
        qDebug()<<"压缩失败！无法打开被压缩文件！"<<endl;
        throw runtime_error("无法打开被压缩文件！\n请检查文件路径！");
    }
    else if(!fout)
    {
        qDebug()<<"压缩失败！无法打开目标文件！"<<endl;
        throw runtime_error("无法打开目标文件！\n请检查压缩到路径！");
    }

    //进度条
    fpos_t fileLen;
    _fseeki64(fin, 0, SEEK_END);
    fgetpos(fin, &fileLen);
    _fseeki64(fin,0,SEEK_SET);
    int steps = 0;//进度条计数器
    long long each = fileLen/50 + 1;
    register long long circleTimes = 0;
    progress->setRange(0, 100);//设置进度条

    char* fileName = getFileName(srcPath);//获取文件名
    int fileNameLen = strlen(fileName);//文件名长度
    if(fileNameLen >= 256)
    {
        qDebug()<<"压缩失败！文件名【"<<fileName<<"】过长！"<<endl;
        throw runtime_error("源文件文件名过长！");
    }

    fputc(fileNameLen,fout);//将文件名长度写入
    fputs(fileName,fout);//将文件名写入

    //建立哈夫曼树
    int c,weights[256];
    memset(weights,0,sizeof(weights));
    while (true) {
        c = fgetc(fin);
        if(feof(fin)) break;

        //更新进度条
        if(circleTimes==each)
        {
            QCoreApplication::processEvents();
            circleTimes = 0;
            progress->setValue(++steps);
        }
        weights[c]++;
    }
    HuffmanTree tree(weights);

    circleTimes = 0;
    fwrite(&weights, sizeof(weights[0]), 256, fout);

    //压缩文件数据
    {
        map<int,string> codeTable = tree.getCodeTable();
        _fseeki64(fin, 0, SEEK_SET);
        string codes = "";
        register int len = 0;
        while (true)
        {
            c = fgetc(fin);
            if(feof(fin))
            {
                break;
            }
            codes += codeTable[c];
            len = codes.length();
            while(len >= 8)
            {
                register int temp = 0;
                for(int i=0; i<8; i++)
                {
                    temp <<= 1;
                    temp |= (codes[i] - '0');
                }
                codes = codes.substr(8,len-8);//缩短字符串
                len -= 8;
                fputc(temp,fout);
            }

            //防止假死，更新进度条
            if((++circleTimes & 65535) == 65535)
                QCoreApplication::processEvents();
            if(circleTimes == each)
            {
                QCoreApplication::processEvents();
                circleTimes = 0;
                progress->setValue(++steps);
            }
        }
        //用0补齐最后不满8位的数据
        int temp = 0;
        for(int i=0; i<len; i++)
        {
            temp <<= 1;
            temp |= (codes[i] - '0');
        }
        for(int i=0; i<8-len; i++)
            temp <<= 1;
        fputc(temp, fout);
        fputc(len, fout);//剩余的位数
    }

    fclose(fin);
    fclose(fout);

    for(; steps<=100; steps++)
    {
        progress->setValue(steps);
    }//进度条补全

    delete[] fileName;//释放内存
    return;
}


//char* zipPath: 压缩文件路径
//char* desPath: 解压的目标文件夹
//char* fileName: 文件名

void ZIP::decode(const char* zipPath, const char* dstPath, QProgressDialog* progress)
{

    const int buffByteSize = 20480;

    wchar_t wZipPath[2048],wdstPath[2048];
    if(!UTF8ToUnicode(zipPath, wZipPath) || !UTF8ToUnicode(dstPath, wdstPath))
        throw runtime_error("路径转换为字符集失败！");
    FILE *fin = _wfopen(wZipPath, L"rb");//被压文件
    FILE *fout = _wfopen(wdstPath, L"wb");//压缩后的文件

    if(!fin)
    {
        qDebug()<<"解压失败！无法打开压缩文件！"<<endl;
        throw runtime_error("无法打开压缩文件！\n请检查压缩文件路径！");
    }
    else if(!fout)
    {
        qDebug()<<"解压失败！无法打开目标解压文件！"<<endl;
        fclose(fin);
        throw runtime_error("无法打开目标解压文件！\n请检查命名以及解压到路径！");
    }
    else if(!checkZip(fin)){
        qDebug()<<"解压失败！压缩文件有误！"<<endl;
        throw runtime_error("压缩文件有误！\n可能压缩文件有损坏！");
    }

    fpos_t fileLen;//文件大小
    _fseeki64(fin, 0, SEEK_END);
    fgetpos(fin, &fileLen);
    _fseeki64(fin, 0, SEEK_SET);
    register int steps = 0;//进度条计数器
    int circleTimes = (fileLen * 8 / buffByteSize) + 1;
    progress->setRange(0, circleTimes);

    int fileNameLen = fgetc(fin);
    _fseeki64(fin,fileNameLen,SEEK_CUR);//跳过文件名存储部分

    //重建哈夫曼树
    int weights[256];
    fread(weights, sizeof(weights[0]), 256, fin);
    HuffmanTree tree(weights);

    //解码
    deque<char> codeQueue;
    register int c1,c2=fgetc(fin);//临时储存压缩文件数据
    char c;//临时存储序列数据
    HuffmanTreeNode *root,*work;//work为工作地址
    root = tree.getRoot();
    work = tree.getRoot();
    while(true)
    {
        while(codeQueue.size() != 0)
        {
            c = codeQueue.front();//出队
            codeQueue.pop_front();
            if(c=='0')
            {
                work=work->lchild;
            }
            else if(c=='1')
            {
                work=work->rchild;
            }

            if(work==nullptr)
            {
                throw runtime_error("解码失败！");
            }

            if(work->data != -1)
            {
                fputc(work->data, fout);
                work = root;
            }
        }
        while(!feof(fin) && codeQueue.size() <= buffByteSize)//获取数据，规定每次获取上限减少内存使用
        {
            c1 = c2;
            c2 = fgetc(fin);
            if(feof(fin))//清空多余的位数
            {
                for(int i=0; i < 8 - c1; i++)
                    codeQueue.pop_back();
                break;
            }
            for(int i=7; i>=0; i--)//重新转化为二进制，并加入到序列
            {
                if(c1 & (1 << i))
                {
                    codeQueue.push_back('1');
                }
                else
                {
                    codeQueue.push_back('0');
                }
            }
        }
        QCoreApplication::processEvents();//防止界面假死
        progress->setValue(++steps);//更新进度条

        if(feof(fin)&&codeQueue.size()==0) break;//处理结束
    }
    fclose(fin);
    fclose(fout);

    return;
}


//分离文件名
char* ZIP::getFileName(const char* path)
{
    int idx=-1;
    int len=strlen(path);
    for(int i=0; i<len; i++)//获取文件夹分隔符
    {
        if(path[i]=='/' || path[i]=='\\')
        {
            idx = i;
        }
    }
    char *fileName = new char[len-idx+1];
    if(idx != -1)//有分隔符
    {
        strcpy(fileName, path + idx + 1);
    }
    else
    {
        strcpy(fileName, path);
    }
    return fileName;
}

//获取源文件名
char *ZIP::getZipFileName(const char* path)
{
    wchar_t wPath[2048];
    UTF8ToUnicode(path, wPath);
    FILE *fin = _wfopen(wPath, L"rb");
    if(!checkZip(fin)) throw runtime_error("压缩文件可能损坏");
    int fileNameLen = fgetc(fin);
    char* fileName = new char[fileNameLen+2];//储存文件名
    fgets(fileName,fileNameLen+1,fin);
    if(ferror(fin)) throw runtime_error("文件读取失败");
    fclose(fin);
    return fileName;
}


//压缩有效性检验
bool ZIP::checkZip(FILE *f)
{
    _fseeki64(f, -1, SEEK_END);
    int t = fgetc(f);
    if(t<0 || t>8)
        return false;
    _fseeki64(f, 0, SEEK_SET);
    t = fgetc(f);
    if(t<=0||t>=256)
        return false;
    _fseeki64(f, 0, SEEK_SET);//文件指针重置
    return true;
}

//转换字符
bool ZIP::UTF8ToUnicode(const char *UTF8, wchar_t *strUnicode)
{
     DWORD dwUnicodeLen;//Unicode的长度
     TCHAR* pwText;//保存Unicode的指针
     dwUnicodeLen = MultiByteToWideChar(CP_UTF8, 0, UTF8, -1, NULL, 0);
     pwText = new TCHAR[dwUnicodeLen];
     if(!pwText)
        return false;
     //转为Unicode
     MultiByteToWideChar(CP_UTF8, 0, UTF8, -1, pwText, dwUnicodeLen);
     //转为CString
     wcscpy(strUnicode, pwText);
     delete[]pwText;
     return true;
}

#include <cstdlib>
#include <cstring>
#include <cstdio>

struct FileStruct
{
    char fileName[300];
    long long fileSize;
};

long long getFileSize(char *fileName)
{
    FILE *fp = fopen(fileName, "rb");
    long long fileSize = 0;

    char buf[1];
    while (fread(buf, 1, 1, fp))
        fileSize++;
    
    fclose(fp);
    return fileSize;
}

char *getFileName(char *pathName)
{
    char *tmp = strrchr(pathName, '\\');
    char *res = new char[strlen(tmp + 1) + 1];
    strcpy(res, tmp + 1);
    return res;
}

void packFile(char *srcFileName, FILE *destFile)
{
    long long fileSize = getFileSize(srcFileName);
    char *fileName = getFileName(srcFileName);  // delete needed

    FileStruct tmpfs;
    strcpy(tmpfs.fileName, fileName);
    tmpfs.fileSize = fileSize;

    char fileHead[sizeof(FileStruct)];
    memcpy(fileHead, &tmpfs, sizeof(FileStruct));
    fwrite(fileHead, sizeof(FileStruct), 1, destFile);

    FILE *fp = fopen(srcFileName, "rb");
    char buf[1];
    while (fread(buf, 1, 1, fp))
        fwrite(buf, 1, 1, destFile);
    fclose(fp);
    delete[] fileName;
}

int main()
{
    char srcFileName[300];
    char destFileName[300];

    printf("destFileName(path): ");
    scanf("%s", destFileName);
    FILE *fp = fopen(destFileName, "wb");

    int count = 1;
    while (true)
    {
        printf("#%d fileName(path): ", count++);
        scanf("%s", srcFileName);
        if (!strcmp(srcFileName, "exit"))
            break;
        packFile(srcFileName, fp);
    }

    fclose(fp);
    return 0;
}
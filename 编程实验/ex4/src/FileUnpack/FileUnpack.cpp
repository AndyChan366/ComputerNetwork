#include <cstdlib>
#include <cstring>
#include <cstdio>

struct FileStruct
{
    char fileName[300];
    long long fileSize;
};

void getUniqueName(char *newFileName, char *filePathName)
{
    char res[300];
    char *tmp = strrchr(newFileName, '.');

    strcpy(res, filePathName);
    strcpy(res + strlen(filePathName), "\\");
    strcpy(res + strlen(filePathName) + 1, newFileName);

    FILE *fp;
    if (!(fp = fopen(res, "rb")))
    {
        strcpy(newFileName, res);
        return;
    }

    fclose(fp);
    int count = 2;
    while (true)
    {
        strcpy(res, filePathName);
        strcpy(res + strlen(filePathName), "\\");
        strcpy(res + strlen(filePathName) + 1, newFileName);

        char num[100];
        itoa(count, num, 10);
        strcpy(res + strlen(res) - strlen(tmp), "(");
        strcpy(res + strlen(res), num);
        strcpy(res + strlen(res), ")");
        strcpy(res + strlen(res), tmp);

        if (!(fp = fopen(res, "rb")))
        {
            strcpy(newFileName, res);
            break;
        }

        fclose(fp);
        count++;
    }
}

void unpackFile(FILE *srcFile, char *path)
{
    FileStruct tmpfs;
    char buf[sizeof(FileStruct)];
    while (fread(buf, sizeof(FileStruct), 1, srcFile))
    {
        memcpy(&tmpfs, buf, sizeof(FileStruct));

        char newFileName[300];
        strcpy(newFileName, tmpfs.fileName);
        getUniqueName(newFileName, path);

        long long count = tmpfs.fileSize;
        FILE *fp = fopen(newFileName, "wb");
        char buf[1];
        while (count)
        {
            fread(buf, 1, 1, srcFile);
            fwrite(buf, 1, 1, fp);
            count--;
        }
        fclose(fp);
    }
}

int main()
{
    char srcFileName[300];
    char path[300];

    printf("dest folder(path): ");
    scanf("%s", path);
    printf("source file(path): ");
    scanf("%s", srcFileName);

    FILE *fp = fopen(srcFileName, "rb");
    unpackFile(fp, path);
    fclose(fp);

    return 0;
}
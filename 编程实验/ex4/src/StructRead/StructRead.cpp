#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#define USER_NAME_LEN 20
#define EMAIL_LEN 80
#define BUF_LEN 100
#define TIME_BUF_LEN 30

typedef unsigned long DWORD;

struct Person
{
    char username[USER_NAME_LEN];
    int level;
    char email[EMAIL_LEN];
    DWORD sendtime;
    time_t regtime;
};

int main()
{
    FILE* fp = fopen("Persons.stru", "rb");
    char buf[sizeof(Person)];

    while (fread(buf, sizeof(Person), 1, fp))
    {
        Person tmpPerson;
        memcpy(&tmpPerson, buf, sizeof(Person));
        char timeBuf1[TIME_BUF_LEN];
        char timeBuf2[TIME_BUF_LEN];
        ctime_s(timeBuf1, TIME_BUF_LEN, (time_t*)&tmpPerson.sendtime);
        ctime_s(timeBuf2, TIME_BUF_LEN, &tmpPerson.regtime);

        printf("username: %s\n", tmpPerson.username);
        printf("level: %d\n", tmpPerson.level);
        printf("email: %s\n", tmpPerson.email);
        printf("sendtime: %s", timeBuf1);
        printf("regtime: %s", timeBuf2);
        printf("\n");
    }

    fclose(fp);
    return 0;
}
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

int inputOnePerson(Person *personSent)
{
    char inputBuf[BUF_LEN];
    char timeBuf[TIME_BUF_LEN];
    time_t now;

    printf("name: ");
    scanf("%s", inputBuf);
    if (!strcmp(inputBuf, "exit"))
        return 0;
    else
        strcpy(personSent->username, inputBuf);

    printf("level: ");
    scanf("%d", &personSent->level);

    printf("email: ");
    scanf("%s", inputBuf);
    strcpy(personSent->email, inputBuf);

    time(&now);
    personSent->sendtime = (DWORD)now;
    personSent->regtime = now;

    printf("\n");
    return 1;
}

int main()
{
    FILE *fp = fopen("Persons.stru", "wb");
    Person tmpPerson;
    while (inputOnePerson(&tmpPerson))
    {
        char buf[sizeof(Person)];
        memcpy(buf, &tmpPerson, sizeof(Person));
        fwrite(buf, sizeof(Person), 1, fp);
    }
    fclose(fp);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    int id;
    char *first_name;
    char *last_name;
} person_t;

typedef struct
{
    size_t size;
    person_t *persons;
} dataframe_t;

int id = 1;


void insert(dataframe_t *df, char *args)
{
    char first_name[50], last_name[50];
    int read = sscanf(args, "%49s %49s", first_name, last_name);
    if (read < 2)
    {
        printf("Enter person's first and last name: ");
        char line[128];
        fgets(line, 128, stdin);

        read = sscanf(line, "%49s %49s", first_name, last_name);
    }

    if (read >= 2)
    {
        person_t *new_persons = (person_t *)realloc(df->persons, (df->size + 1) * sizeof(person_t));
        if (!new_persons)
        {
            perror("new_persons = realloc(df->persons)");
            return;
        }

        df->persons = new_persons;
        df->persons[df->size++] = (person_t){
            .id = id++,
            .first_name = strdup(first_name),
            .last_name = strdup(last_name),
        };
    }
}

void delete(dataframe_t *df, char *args)
{
    int did;
    int read = sscanf(args, "%d", &did);
    if (read < 1)
    {
        printf("Enter the person's id: ");
        char line[32];
        fgets(line, 32, stdin);

        read = sscanf(line, "%d", &did);
    }

    if (read >= 1)
    {
        size_t i;
        for (i = 0; i < df->size; ++i)
            if (df->persons[i].id >= did)
                break;

        if (i >= df->size || df->persons[i].id > did)
            return;

        if (df->size == 1)
        {
            free(df->persons);
            df->persons = NULL;
            df->size = 0;
            return;
        }

        person_t *new_persons = (person_t *)calloc(df->size - 1, sizeof(person_t));
        if (!new_persons)
        {
            perror("new_persons = calloc(df->size - 1, sizeof(person_t)");
            return;
        }

        memcpy(new_persons, df->persons, i * sizeof(person_t));
        memcpy(new_persons + i, &df->persons[i+1], (df->size - i - 1) * sizeof(person_t));

        free(df->persons[i].first_name);
        free(df->persons[i].last_name);
        free(df->persons);

        df->persons = new_persons;
        --df->size;
    }
}

dataframe_t *find(dataframe_t *df, char *args)
{
    dataframe_t *ret = calloc(1, sizeof(dataframe_t));
    char *p = args;
    int n;

    int fid;
    int sid = sscanf(p, " ID = %d%n", &fid, &n);
    sid = sid == 1;
    if (sid)
        p += n;

    char first_name[50];
    int sfn = sscanf(p, " FirstName = %s%n", first_name, &n);
    sfn = sfn == 1;
    if (sfn)
        p += n;

    char last_name[50];
    int sln = sscanf(p, " LastName = %s", last_name);
    sln = sln == 1;

    if (sid)
    {
        size_t i;
        for (i = 0; i < df->size; ++i)
            if (df->persons[i].id >= fid)
                break;

        if (i >= df->size || df->persons[i].id > fid)
            return NULL;

        ret->persons = (person_t *)calloc(1, sizeof(person_t));
        if (!ret->persons)
        {
            perror("ret->persons = calloc(1, sizeof(person_t))");
            free(ret);
            return NULL;
        }

        ret->size = 1;
        ret->persons[0] = df->persons[i];

        return ret;
    }

    if (sfn)
    {
        size_t size = 0;
        person_t *persons = NULL;
        for (size_t i = 0; i < df->size; ++i)
            if (!strcmp(first_name, df->persons[i].first_name))
            {
                persons = realloc(persons, (size + 1) * sizeof(person_t));
                persons[size++] = df->persons[i];
            }

        ret->persons = persons;
        ret->size = size;
    }

    if (sln)
    {
        size_t size = 0;
        person_t *persons = NULL;

        if (sfn)
        {
            for (size_t i = 0; i < ret->size; ++i)
            {
                if (!strcmp(last_name, ret->persons[i].last_name))
                {
                    persons = realloc(persons, (size + 1) * sizeof(person_t));
                    persons[size++] = ret->persons[i];
                }
            }

            free(ret->persons);
            ret->persons = persons;
            ret->size = size;
        } else
        {
            for (size_t i = 0; i < df->size; ++i)
            {
                if (!strcmp(last_name, df->persons[i].last_name))
                {
                    persons = realloc(persons, (size + 1) * sizeof(person_t));
                    persons[size++] = df->persons[i];
                }
            }

            ret->persons = persons;
            ret->size = size;
        }
    }

    if (ret->size)
        return ret;

    free(ret);
    return NULL;
}

void update(dataframe_t *df, char *args)
{
    int uid;
    char first_name[50], last_name[50];

    char *p = args;
    int n;
    int cid = sscanf(p, " %d%n", &uid, &n);
    cid = cid >= 1;

    if (cid)
        p += n;
    else
    {
        printf("Enter the person's id: ");
        char line[32];
        fgets(line, 32, stdin);

        cid = sscanf(line, "%d%n", &uid, &n);
    }

    int cfn = sscanf(p, " FirstName = %49s%n", first_name, &n);
    cfn = cfn >= 1;
    if (cfn)
        p += n;

    int cln = sscanf(p, " LastName = %49s", last_name);
    cln = cln >= 1;

    printf("%s\n", last_name);

    if (!cfn && !cln)
    {
        printf("[FirstName=%%s] [LastName=%%s] ");

        char line[128];
        fgets(line, 128, stdin);
        p = line;

        cfn = sscanf(p, " FirstName = %49s%n", first_name, &n);
        cfn = cfn >= 1;
        if (cfn)
            p += n;

        cln = sscanf(p, " LastName = %49s", last_name);
        cln = cln >= 1;
    }

    if (!cfn && !cln)
        return;

    if (cid)
    {
        size_t i;
        for (i = 0; i < df->size; ++i)
            if (df->persons[i].id >= uid)
                break;

        if (i >= df->size || df->persons[i].id > uid)
            return;

        if (cfn)
        {
            free(df->persons[i].first_name);
            df->persons[i].first_name = strdup(first_name);
        } if (cln)
        {
            free(df->persons[i].last_name);
            df->persons[i].last_name = strdup(last_name);
        }
    }
}

int main(int argc, char *argv[])
{
    dataframe_t df = {
        .size = 0,
        .persons = NULL,
    };

    char input[256];
    while (strncmp(input, "exit", strlen("exit")))
    {
        char command[32] = { 0 };
        char args[200] = { 0 };

        printf("[person-db]# ");
        fgets(input, 256, stdin);

        if (sscanf(input, "%31s %[^\n]", command, args) >= 1)
        {
            if (!strncmp(command, "insert", strlen("insert")))
                insert(&df, args);
            else if (!strncmp(command, "delete", strlen("delete")))
                delete(&df, args);
            else if (!strncmp(command, "find", strlen("find")))
            {
                dataframe_t *ret = find(&df, args);
                if (!ret)
                {
                    printf("Empty DataFrame\n");
                    continue;
                }

                dataframe_t fdf = *ret;
                printf("PERSON   %-5s %-50s %-50s\n", "ID", "FIRST NAME", "LAST NAME");
                for (size_t i = 0; i < fdf.size; ++i)
                {
                    person_t person = fdf.persons[i];
                    printf("         %-5d %-50s %-50s\n", person.id, person.first_name, person.last_name);
                }

                free(ret);
            } else if (!strncmp(command, "update", strlen("update")))
                update(&df, args);
            else if (!strncmp(command, "print", strlen("print")))
            {
                printf("PERSON   %-5s %-50s %-50s\n", "ID", "FIRST NAME", "LAST NAME");
                for (size_t i = 0; i < df.size; ++i)
                {
                    person_t person = df.persons[i];
                    printf("         %-5d %-50s %-50s\n", person.id, person.first_name, person.last_name);
                }
            } else if (!strncmp(command, "exit", strlen("exit")))
                break;
        }
    }

    if (df.persons)
        free(df.persons);

    return 0;
}


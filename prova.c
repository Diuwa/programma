#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <utmp.h>
#include <time.h>
#include <stdlib.h>

#define UT_NAMESIZE 32


/*

    contenuto di plan in /home/vboxuser/.plan

*/

struct user_info
{
    char *username;
};

struct user_list
{
    struct user_info *users;
    size_t count;
};


/*
    Opzione_M come il controllo normale, e aggiungi un case insesitive quando non e m, guarda su internet
    Opzione_P guarda su finger ma hai tutto il necessario
*/

int opzione_P(const char *username){
    struct passwd *pwd = getpwnam(username);
    
    char *login = pwd -> pw_name;
    printf("Login: %-15s\t\t\t", login);
    
    char *realName = pwd -> pw_gecos;
    char *del = strchr(realName, ',');
    *del = '\0';

    printf("Name: %s\n", realName);
    printf("Directory: %-20s\t\t", pwd -> pw_dir);
    printf("Shell: %s\n", pwd -> pw_shell);

}




int opzione_L(const char *username){

    struct passwd *pws = getpwnam(username);

    /*
        Basta chiamare opzione_p e aggiungerci anche il Plan
    */



    /*
        Operazioni sulle stringhe per il nome reale in quanto pw_gecos contiene altre informazioni oltre 
        il nome reale di conseguenza c'e la necessita di togliere 3 virgole finali
    */
    
    char *prova = pws -> pw_gecos;
    size_t len = strlen(prova);
    prova[len-3] = '\0';

    printf("Username: %s\n", prova);
    printf("Prova L\n");

    return 0;
}


int opzione_S(const char *username){

    struct passwd *pws = getpwnam(username);
    struct utmp *ute;
    time_t currentT = time(NULL);
    char time_buf[256];
    static int printed = 0;

    if (!printed)
    {
        printf("%-9s %-10s %-8s %-5s %-12s %-11s %-11s\n", "Login", "Name", "Tty", "Idle", "Login Time", "Office", "Office Phone");
        printed = 1;
    }
    
    char *sUserName = pws -> pw_name;
    char *sName = pws -> pw_gecos;
    
    size_t len = strlen(sName);
    sName[len-3] = '\0';
    
    char *sTty = "*";
    time_t lTime = 0;
    
    setutent();
    // Serve per prendere il terminale alla quale l'utente appartiene
    while ((ute = getutent()) != NULL) {
        if (strncmp(ute->ut_user, sUserName,sizeof(ute->ut_user)) == 0) {
            lTime = ute -> ut_tv.tv_sec;
            sTty = ute->ut_line;
            break;
        }
    }
    endutent();
    
    struct tm *tm_info = localtime(&lTime);

    strftime(time_buf, sizeof(time_buf), "%d-%m %H:%M", tm_info);
    time_buf[11] = '\0';

    long idle_seconds = currentT - lTime;
    int idle_hours = idle_seconds / 3600;
    int idle_minutes = (idle_seconds % 3600) / 60;


    char *gecos = pws -> pw_gecos;
    char *sOffice = strtok(gecos, ","); // Qui prende il nome reale
    sOffice = strtok(NULL, ","); // Scrivo NULL per poter far riprendere a strtok dal punto di prima
    char *sOfPhone = strtok(NULL, ",");\

    if (sOffice == NULL)
    {
        sOffice = sTty;
    }
    

    if (lTime != 0)
    {
        printf("%-9s %-10s %-8s %d:%-3d %-12s %-11s %s\n", sUserName, sName, sTty, idle_hours,idle_minutes,time_buf, sOffice,sOfPhone);

    }else{
        printf("%-9s %-10s %-8s %-4s %-12s %-11s %s\n", sUserName, sName, sTty, "*", "*", sOffice,sOfPhone);

    }
    
    
    return 0;
}

//funzione che stampa tutti gli utenti

int all_user(int tOpt, struct user_list *userList){
    struct utmp *ut;

    setutent();

    userList -> users = NULL;
    userList -> count = 0;

    while((ut = getutent())!= NULL){
        if (ut -> ut_type == USER_PROCESS){
            userList -> count++;
            userList -> users = realloc(userList -> users, userList -> count * sizeof(struct user_info));
            if (userList->users == NULL) {
                perror("realloc failed");
                endutent();
                return 1;
            }

            userList->users[userList->count - 1].username = strndup(ut->ut_user, UT_NAMESIZE);
            if (userList->users[userList->count - 1].username == NULL) {
                perror("strdup failed");
                endutent();
                return 1;
            }
        }
        
    }

    endutent();

    for (size_t i = 0; i < userList->count; i++)
    {
        if (tOpt == 0){
            opzione_L(userList->users[i].username);
            free(userList->users[i].username);
        } else if (tOpt == 1)
        {
            opzione_P(userList -> users[i].username);
            free(userList->users[i].username);
        } else if (tOpt == 2)
        {
            opzione_S(userList->users[i].username);
            free(userList->users[i].username); }
    }
    return 0;    
}

//main

int main (int args, char *argv[] ){


    int opt;
    int counter = 1;
    const char *username = NULL;
    struct user_list userList;


    bool l_flag = false;
    bool p_flag = false;
    bool s_flag = false;
    bool m_flag = false;

    while((opt = getopt(args, argv, "lpsm")) != -1){
        switch (opt)
        {
            case 'l':
                counter += 1;
                l_flag = true;
                break;
            case 'p':
                counter += 1;
                p_flag = true;
                break;
            case 's':
                counter += 1;
                s_flag = true;
                break;
            case 'm':
                counter += 1;
                m_flag = true;
                break;
            default:
                // qui non entra mai è solo per sicurezza
                if (counter > 2)
                {
                    printf("Situazione di default");
                }
                
                printf("Nulla");
                break;
        }

    }

    //quando no opzioni e no utenti o quando solo opzioni e no utenti
    if (args == 1 || args == counter)
    {
        int a;
        if (counter > 2 || l_flag || args == 1)
        {
            a = all_user(0, &userList);
            return a;
        }else if (p_flag)
        {
            a = all_user(1, &userList);
            return a;
        }else if (s_flag)
        {
            a = all_user(2, &userList);
            return a;
        }else if (m_flag)
        {
            a = all_user(3, &userList);
            return a;
        }
        
        return a;
    }
    

    // aggiungere controllo sull'esistenza dell'username e nel caso sia un Real Name
    for (int i = counter; i < args; i++)
    {
        username = argv[i];
        if (username != NULL)
        {
            if (getpwnam(username) != NULL){
               struct passwd *pw = getpwnam(username);
            } else{
                struct passwd *pw_real = NULL;
                setpwent();  // Rewind to the beginning of passwd file
                while ((pw_real = getpwent()) != NULL) {
                    int taglio = strlen(username);
                    char *temp = pw_real -> pw_gecos;
                    temp[taglio] = '\0';
                    if (strcmp(temp, username) == 0) {
                        // Real name matches
                        username = pw_real ->pw_name;
                        break;
                    }
                }
                endpwent();
                }
            if (getpwnam(username) == NULL)
            {
               printf("Utente %s non esistente\n", username);
               return 0;
            }
        }
            
        if (counter > 2 || (!l_flag && !p_flag && !s_flag && !m_flag))
        {
            printf("Situazione di default L\n");
        }
        else if (l_flag)
        {
            int l = opzione_L(username);
            return l;
        } 
        else if (p_flag)
        {
            int p = opzione_P(username);
            return p;
        } 
        else if (s_flag)
        {
            int s = opzione_S(username);
        } 
        else if (m_flag)
        {
            printf("opzione m\n");
        }
    }
    

    return 0;

}




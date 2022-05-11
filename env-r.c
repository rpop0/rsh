#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>

/*
 * I only needed to implement env and env -u, which was pretty simple. I just checked if we did the -u option and saved the variable name and a flag.
 * The default env command prints out all the envirnoment variables. Using env -u "VARNMAE" prints out the envirnoment variables without that
 */
int main(int argc, char **argv){
    int option;
    char varName[50];
    int uFlag = 0;
    while(( option = getopt(argc, argv,"u:h")) != -1){
        switch (option) {
            case 'u':
                uFlag = 1;
                strcpy(varName, optarg);
                break;
            case 'h':
                printf("Welcome to the help page of the 'env' command.\n");
                printf("This command works like the standard unix 'env' command.\n");
                printf("USAGE: env [OPTION] [NAME=VALUE]\n");
                printf("It prints out all the environment variables and adds NAME=VALUE variable.\n");
                printf("-u [NAME],            removes a variable from the environment\n");
                exit(0);
            default:
                break;
        }
    }

    if(uFlag == 1){ //If the U flag is set, just unset the variable
        unsetenv(varName);
    } else { //If it's not, we loop through any variables that we might have given and then add them to the environment.
        for(int i = optind;i<argc;i++){ //If none of the two conditions above happen, we add all the files in an array.
            putenv(argv[i]);
        }
    }

    extern char **environ;


    int i = 0;
    while(environ[i] != 0){
        printf("%s\n", environ[i]);
        i++;
    }

    return 0;
}
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <fcntl.h>
#include<getopt.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>



#define M_GREEN "\033[0;32;1m"
#define M_WHITE "\033[0;37;1m"
#define M_BLUE "\033[0;34;1m"
#define M_RED "\033[0;31;1m"
#define M_WHITE_NB "\033[0;37m"

char g_initialWorkingDir[512];

/*
 * Adding this here to explain how I run the commands created by myself. So when running the program, inti() gets called.
 * In init, I set the g_initialWorkingDir variable to the path of program. This is strictly because I need to run the commands implemented by myself.
 * Each command implemented by myself is an individual program that's compiled and found in the 'bin' folder.
 * Each time a command is run, the program checks if the command is wc, expand or env. If it's either one of the three, it adds the folder to the path.
 * using the addCommandsToPath() function. This needs to be done every time as I've found that the environment variables get reset in each new process.
 */

void addCommandsToPath(){
    char* pathEnv = getenv("PATH"); //Get the current PATH env variable.
    char path[512] = "PATH="; //Here we will build the new path.
    strcat(path,pathEnv); //Concatenate the already existing path.
    strcat(path,":"); //Add the ':' to append to the variable.
    strcat(path, g_initialWorkingDir); //We add the initial working dir.
    strcat(path, "/bin"); //Then add the bin folder that contains the binaries of the commands.
    putenv(path);
}

void init(){
    getcwd(g_initialWorkingDir, sizeof(g_initialWorkingDir));
    system("clear");
    printf(M_RED"--------------------------RSH--------------------------\n"
                "Welcome to RSH. If you pipe or redirect commands, make sure there is a space before and after the pipe '|' or redirect '>'.\n" M_WHITE);
}



int shellInput(char* str){
    char* input;
    usleep((__useconds_t) 1000);
    input = readline("\n╰✗ " M_WHITE_NB);
    if(strlen(input)==0){
        return 0;
    }
    strcpy(str, input);
    add_history(str);
    free(input);
    return 1;
}

int hasPipe(char* str){
    int pipeno = 0;
    for(int i=0;i<strlen(str);i++){
        if(str[i] == '|'){
            pipeno++;
        }
    }

    return pipeno;
}

void splitOnSpace(char* str, char** parsedCommand){

    //If someone reads this, for me it's currently 18th of December, 2020. I got the program to work last night, but today I realized that I didn't try
    //Things like <grep file.txt "test test">, meaning that I only tested with a single word within the quotes. I spent the whole day trying around 5 methods until
    //I managed to come up with this. I can say that I am proud.
    int i;
    for(i=0;i<100 && str != NULL;i++){
        if(str[0] == '\"'){ //If the string starts a quote <">.
            strsep(&str, "\""); //Cuts everything including the quote.
            parsedCommand[i] = strsep(&str, "\""); //Cuts the whole command up until the next quote and places it in the array at position i
            strsep(&str, " "); //After the
        } else {
            parsedCommand[i] = strsep(&str, " ");
        }

        if(str == NULL){//If it doesn't find anything or is at the end, just exit the function; parsedCommand will always end with a NULL, because the arguments in execvp need to be terminated by a NULL.
            parsedCommand[++i] = NULL;
            break;
        } else if (strlen(parsedCommand[i]) == 0){ // If the length is 0, it means it's a space, checking for this basically means that you can have a command like "ls      -l" register as "ls -l"
            i = i-1; //If it has multiple spaces, decrement i by 1 for each space so the parsed array doesn't have gaps between the commands, args etc.
        }
    }
    /*
     * This right here checks if the last element is a blank space. What happens is if you autocomplete a file with a tab it will usually add a space
     * after the file name, if you directly click enter, it will bug up the parsed string. So if there is a string with length 0 at the end, just set it to
     * NULL it works.
     */
    if(strlen(parsedCommand[i-1]) == 0){
        parsedCommand[i-1] = NULL;
    }

}

int execIntExtNP(char **parsedCommand, char *redirectTo){ //Executes an internal/external command (External meaning normal commands, internal meaning commands created by me.
    if(strcmp(parsedCommand[0], "ls") == 0 && strcmp(redirectTo, "/") == 0){ //Wanted to do this. If the command is LS
        int i = 0;
        while(parsedCommand[i] != NULL){
            i++;
        }
        parsedCommand[i++] = "--color=always"; //We go to the first NULL position and replace it with this option, which enables colors for ls.
        parsedCommand[i] = NULL; //Just set the next to null.
    }

    if(strcmp(parsedCommand[0], "wc") == 0){
        addCommandsToPath();
        parsedCommand[0] = "wc-r";
    } else if(strcmp(parsedCommand[0], "expand") == 0){
        addCommandsToPath();
        parsedCommand[0] = "expand-r";
    } else if(strcmp(parsedCommand[0], "env") == 0){
        addCommandsToPath();
        parsedCommand[0] = "env-r";
    }

    return execvp(parsedCommand[0], parsedCommand); //Right now just executes external as no internals created.
}

int execIntExtPiped(char* parsedPipe[25][25], int index){
    if(strcmp(parsedPipe[index][0], "wc") == 0){
        addCommandsToPath();
        parsedPipe[index][0] = "wc-r";
    } else if(strcmp(parsedPipe[index][0], "expand") == 0){
        addCommandsToPath();
        parsedPipe[index][0] = "expand-r";
    } else if(strcmp(parsedPipe[index][0], "env") == 0){
        addCommandsToPath();
        parsedPipe[index][0] = "env-r";
    }
    return execvp(parsedPipe[index][0], parsedPipe[index]);
}

void execNonPiped(char** parsedCommand, char* redirectTo){
    int processId = fork(); //Create a child process
    if(processId == -1) {
        printf("Fork failed.");
        return;
    }
    if(processId == 0){ //If in the child process
        if(strcmp(redirectTo, "/") != 0){
            int fd;
            fd = creat(redirectTo, 0664);
            if(fd != -1){
                dup2(fd, 1); //Instead of writing to STDOUT, it will write to the file.
                close(fd);
            }
        }
        int status = execIntExtNP(parsedCommand, redirectTo);
        if(status == -1){
            printf("%s: command not found.\n", parsedCommand[0]);
            exit(1);
        }
    } else {
        wait(NULL);
    }
}


void cd(char** parsedCommand){
    chdir(parsedCommand[1]);
}

int handleBuiltin(char** parsedCommand){
    char* cmdList[10] = {"exit", "version" ,"cd", "help"};
    int i = 0;
    int cmdID = -1;
    while(cmdList[i] != NULL){
        if(strcmp(parsedCommand[0], cmdList[i]) == 0){
            cmdID = i;
        }
        i++;
    }

    switch (cmdID) {
        case 0:
            exit(0);
        case 1:
            printf(M_WHITE " RSH Version 1.0\n Simple command line interpreter for my\n Operating Systems class.\n" M_WHITE);
            break;
        case 2:
            cd(parsedCommand);
            break;
        case 3:
            printf("RSH command line interpreter.\n");
            printf("The following shell commands are defined internally.\n");
            printf("cd [DIR]              changes the current working directory\n");
            printf("version               displays the program version and author information\n");
            printf("help                  shows the information that you are currently viewing\n");
            printf("exit                  exits the shell\n");
            printf("It is important that all pipes and redirection characters are spaced and not touching the actual commands.\n");
            printf("Besides shell commands, you are able to run any program through this shell, like you would\n");
            printf("with BASH, ZSH, FISH, etc.\n\n");
            printf("This program also implements the following commands, and even if they are named exactly like their counterparts,\n");
            printf("internally the command changes to execute the corresponding custom binaries.\n");
            printf("wc                    print newline, word, and byte counts for each file\n");
            printf("expand                covert tabs to spaces\n");
            printf("env                   prints the environment and unsets variables\n");
            printf("In order to see details about the three programs above, execute the with the option 'h' (e.g. wc -h )\n");
        default:
            break;
    }
    return cmdID;
}


/*
 * So this function basically transforms the parsed command if it contains pipes, it basically makes it a matrix, each row being it's own command.
 * so, the [ ls -l | cat ] would look like this internally
 * ["ls"] ["-l"] [(null)]
 * ["cat" [(null)] [(null)]
 *
 * By doing it like this, the commands can be easily be executed when piped, since by accessing parsedpipe[n] you get the whole array to execute with execvp
 * and by doing parsedpipe[n][0] you get the command itself
 */
void parsePipe(char** parsedCommand, char* parsedPipe[25][25]){
    int i=0;
    int carry = 0;
    while(parsedCommand[i] != NULL){
        int j = 0;
        if(i != 0){
            j = carry + 1;
        }
        if(parsedCommand[j] != NULL && strcmp(parsedCommand[j], "|") != 0){
            int k = 0;
            while(parsedCommand[j] != NULL && strcmp(parsedCommand[j], "|") != 0){
                parsedPipe[i][k] = parsedCommand[j];
                k++;
                j++;
            }
            parsedPipe[i][k] = NULL;
        }
        carry = j;
        i++;
    }
}


void executePipe(char* parsedPipe[25][25], int pipeno, char* redirectTo){
    int pfd[2*pipeno];
    for(int i=0;i<pipeno*2;i=i+2){ //Creating pipeno pipes
        pipe(pfd+i);
    }
    if(pipeno == 1) { // If the command only has one pipe. I'm doing this because if we have one pipe, the first pipe needs only it's output changed and the last needs only input to be changed. If we have more than two, the ones between need both in and out.
        int pid = fork(); //We fork for the first command
        if (pid == 0) { //Here we are in the child of the first command.
            dup2(pfd[1], 1); //What this does is copies the end of the pipe in which you write in place of STDOUT for this specific process, meaning that this command will output to the pipe instead of to STDOUT.
            close(pfd[0]); //Close the pipes
            close(pfd[1]); //Close the pipes
            int status = execIntExtPiped(parsedPipe, 0); // Here, we execute the first command.
            if(status == -1){
                printf("%s: command not found.\n", parsedPipe[0][0]);
                exit(1);
            }
        } else {
            int pid_2 = fork(); //Now again, if we are in the parent, we fork out
            if (pid_2 == 0) { //We are in the child for th second command
                dup2(pfd[0], 0); //Here we do the same thing, but instead of replacing STDOUT, we replace STDIN, basically giving the second command the reading end of the pipe from the last command, passing the output of the first as input to the second.
                close(pfd[0]); //CLose pipes
                close(pfd[1]);
                if(strcmp(redirectTo, "/") != 0){
                    int fd;
                    fd = creat(redirectTo, 0664);
                    if(fd != -1){
                        dup2(fd, 1); //Instead of writing to STDOUT, it will write to the file.
                        close(fd);
                    }
                }

                int status = execIntExtPiped(parsedPipe, 1); //And then exec.
                if(status == -1){
                    printf("%s: command not found.\n", parsedPipe[1][0]);
                    exit(1);
                }
            }
        }
        close(pfd[0]); //We close the pipes again in the parent
        close(pfd[1]);
        wait(NULL); //And wait for the two children to finish.
        wait(NULL);
    } else { //We basically need to loop pipeno+1 times. If a command has 4 pipes, it will execute 5 individual commands.
        for(int i=0;i<pipeno+1;i++){
            int pid = fork(); //Creating a child.
            if(pid == 0){ //In the child. Here we will now need to check if the process is either the first or the last.
                if(i == 0){
                    dup2(pfd[1], 1);
                    for(int j=0;j<pipeno*2;j++){ //Closing all other pipes;
                        close(pfd[j]);
                    }
                    int status = execIntExtPiped(parsedPipe, 0); //Executing first command.
                    if(status == -1){
                        printf("%s: command not found.\n", parsedPipe[0][0]);
                        exit(1);
                    }
                } else if(i == pipeno){ // If it's at the end
                    dup2(pfd[(2*pipeno)-2], 0); //Replace the input of the last command with the reading end of the last pipe.
                    for(int j=0;j<pipeno*2;j++){ //Closing all other pipes;
                        close(pfd[j]);
                    }
                    if(strcmp(redirectTo, "/") != 0){
                        int fd;
                        fd = creat(redirectTo, 0664);
                        if(fd != -1){
                            dup2(fd, 1); //Instead of writing to STDOUT, it will write to the file.
                            close(fd);
                        }
                    }
                    int status = execIntExtPiped(parsedPipe, pipeno);
                    if(status == -1){
                        printf("%s: command not found.\n", parsedPipe[pipeno][0]);
                        exit(1);
                    }
                } else { // If it's somewhere in the middle.
                    dup2(pfd[(i-1)*2], 0);
                    dup2(pfd[(i*2)+1], 1);
                    for(int j=0;j<pipeno*2;j++){ //Closing all other pipes;
                        close(pfd[j]);
                    }
                    int status = execIntExtPiped(parsedPipe, i);
                    if(status == -1){
                        printf("%s: command not found.\n", parsedPipe[i][0]);
                        exit(1);
                    }
                }

            }
        }
        for(int j=0;j<pipeno*2;j++){ //Closing all other pipes and waiting;
            close(pfd[j]);
        }
        for(int i=0;i<pipeno;i++){
            wait(NULL);
        }

    }

}

void parseRedirects(char** parsedCommand, char *redirectTo){
    int i=0;
    while(parsedCommand[i] != NULL){
        if(strcmp(parsedCommand[i], ">") == 0){
            strcpy(redirectTo, parsedCommand[i+1]);
            parsedCommand[i] = NULL;
            parsedCommand[i+1] = NULL;
            return;
        }
        i++;
    }
}




int main(int argc, char **argv) {
    char* parsedCommand[25]; //This is a pointer to a pointer because it's basically an array of word arrays.
    char redirectTo[50] = "/";
    char* parsedPipe[25][25];
    int remoteInput = 0;
    int option;
    int port = 5050;

    while ((option=getopt(argc,argv, "shp:")) != -1){
        switch (option) {
            case 's':
                remoteInput = 1;
                break;
            case 'h':
                printf("Help section for the RSH\n");
                printf("available options:\n");
                printf("-s             start the program in remote server mode, port 5050\n");
                printf("-h             shows this page.\n");
                printf("-p [PORT]      sets the port to the server.");
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                break;
        }
    }


    char input[256];
    char workingDir[1024];
    char userName[64];

    if(remoteInput == 0) { //Normal execution.
        init(); //Calling init here because of the print.
        while(1) {
            getcwd(workingDir, sizeof(workingDir));
            getlogin_r(userName, sizeof(userName));
            printf("╭" M_GREEN "%s" M_WHITE ":" M_BLUE "%s" M_WHITE, userName, workingDir); //Printing the location prompt
            if(shellInput(input) == 0){
                return 0;
            }

            int pipeno = hasPipe(input);
            strcpy(redirectTo, "/");
            splitOnSpace(input, parsedCommand); //Splits the command on spaces.
            parseRedirects(parsedCommand, redirectTo);

            if(handleBuiltin(parsedCommand) == -1){ // Here it executes the command as a built in command, like cd, exit etc. So non programs.
                //If it's not a built in command, we check if it's piped.
                if(pipeno == 0){ //If it has no pipes, just execute the command.
                    execNonPiped(parsedCommand, redirectTo);
                } else {
                    parsePipe(parsedCommand, parsedPipe);
                    executePipe(parsedPipe, pipeno, redirectTo);
                }
            }

        }
    } else { //Server execution
        int socketFd = socket(AF_INET, SOCK_STREAM, 0);
        if(socketFd == -1){
            printf("Error creating the server socket.\n");
            exit(0);
        }

        struct sockaddr_in ServerDetails;
        ServerDetails.sin_family = AF_INET;
        ServerDetails.sin_addr.s_addr = INADDR_ANY;
        ServerDetails.sin_port = htons(port);

        if(bind(socketFd, (struct sockaddr*) &ServerDetails, sizeof(ServerDetails)) == -1 ){
            printf("Error binding socket to IP&Port.\n");
            exit(0);
        }

        if(listen(socketFd, 10) == -1){
            printf("Error setting socket to listen.\n");
            exit(0);
        }

        printf("Server initialized, awaiting connections...\n");


        int clientSocket;
        struct sockaddr_in ClientDetails;
        int clientDetailsLen = sizeof(ClientDetails);
        while((clientSocket = accept(socketFd, (struct sockaddr*) &ClientDetails, (socklen_t*)&clientDetailsLen))) {
            int pid = fork();
            if (pid==0){
                dup2(clientSocket, STDOUT_FILENO);
                dup2(clientSocket, STDERR_FILENO);
                init();
                while (1){
                    getcwd(workingDir, sizeof(workingDir));
                    getlogin_r(userName, sizeof(userName));
                    printf("╭" M_GREEN "%s" M_WHITE ":" M_BLUE "%s" M_WHITE "\n", userName, workingDir); //Printing the location prompt
                    bzero(input, 256);
                    if (recv(clientSocket, input, 256, 0) == -1 ){
                        exit(1);
                    }

                    int pipeno = hasPipe(input);
                    strcpy(redirectTo, "/");
                    splitOnSpace(input, parsedCommand); //Splits the command on spaces.
                    parseRedirects(parsedCommand, redirectTo);


                    if(handleBuiltin(parsedCommand) == -1){ // Here it executes the command as a built in command, like cd, exit etc. So non programs.
                        //If it's not a built in command, we check if it's piped.
                        if(pipeno == 0){ //If it has no pipes, just execute the command.
                            execNonPiped(parsedCommand, redirectTo);
                        } else {
                            parsePipe(parsedCommand, parsedPipe);
                            executePipe(parsedPipe, pipeno, redirectTo);
                        }
                    }
                }
                close(socketFd);
            } else {
                printf("Incoming connection from %s\n", inet_ntoa(ClientDetails.sin_addr));
            }

        }
    }


}

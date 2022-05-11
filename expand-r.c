#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<unistd.h>

//Start reading from main.

int countTabs(char* str, int after){ //Simple function that just counts the tabs in a string.
    int i=0;
    int counter=0;
    while(str[i] != '\0'){
        if(str[i] == '\t'){
            counter++;
        }
        if(after == 1){
            if(str[i] != '\t'){
                return counter;
            }
        }
        i++;
    }
    return counter;
}


void replaceTabs(char* fileName, int spaceCount, int iFlag){
    FILE* fp; //We create a file pointer.
    if(strcmp(fileName, "std/in") == 0){ //Now, I chose "std/in" as the special file name because in unix-like systems, files can't have a slash in them.
        fp = fdopen(STDIN_FILENO, "r"); //If it has the name like that, we set the file pointer to stdin.
    } else {
        fp = fopen(fileName, "r"); //If not, just open the file.
    }
    if(fp == NULL){ //Here we check if file is valid.
        printf("Invalid file.\n");
        exit(1);
    }
    char* line = NULL;
    size_t n = 0;
    char* modifiedLine = NULL;
    int tabCount;
    int letterCount;
    while(getline(&line, &n, fp) != -1){ //We get each line in the file
        letterCount = 0; //This is so we maintain the default tab spacing of 8 characters
        int j=0;
        if(iFlag == 0){
            tabCount = countTabs(line, 0);
        } else {
            tabCount = countTabs(line, 1);
        }
        modifiedLine = (char*) malloc(sizeof(char)*(strlen(line)-tabCount+(spaceCount*tabCount))+1); //Allocate enough memory to replace the tabs with spaces;
        if(iFlag == 0){ //If the i flag is set to 0, we have a different tab number
            for(int i=0;i<=strlen(line);i++){ //Loop through the first line in the file/stdin. We are going up to strlen, including, because we need to copy the \0 as well.
                if(line[i] == '\t'){ //If it's a tab
                    int k = spaceCount-letterCount; //We replace the tab with spaces.
                    /*
                     * To explain what happens here better, if we have something like "[TAB]123[TAB]123", the real expand command will
                     * replace the first tab with 8 spaces and the second one with 5, because each line basically has tab breakpoints, which are of size 8.
                     * So we subtract the number of characters in that tab breakpoint segment from 8, and that number is how many spaces we need to add.
                     *
                     */
                    while(k != 0){
                        modifiedLine[j++] = ' '; //Here we just add k spaces
                        k--;
                    }
                    letterCount = 0; //Set it to 0, since we finished the tab breakpoint.
                } else {
                    letterCount++; //If it's a character, increase the letter count.
                    if(letterCount==spaceCount){
                        letterCount=0;
                    }
                    modifiedLine[j++] = line[i]; //And then just add it normally to the array.
                }
            }
        } else { //Here if the I flag is set, we just need to replace the tabs before any letters.
            for(int i=0;i<=strlen(line);i++){
                if(line[i] == '\t' && tabCount != 0){
                    int k = spaceCount;
                    while(k != 0){
                        modifiedLine[j++] = ' ';
                        k--;
                    }
                    tabCount--;
                } else {
                    modifiedLine[j++] = line[i];
                }
            }
        }
        printf("%s", modifiedLine); //At the end, output the line to STDOUT
        free(modifiedLine); //And free the memory of that line
    }
    free(line); //At the end, we free the memory allocated by getline
    if(fp != stdin){ //And if we opened a file, close it.
        fclose(fp);
    }
}


int main(int argc, char **argv){
    int option;
    char *fileNames[50];
    int spaceCount=-1;
    int iFlag=0;
    while((option = getopt(argc, argv, "hit:")) != -1 ){ //Handling flags and everything.
        switch (option) {
            case 'i':
                iFlag = 1;
                break;
            case 't':
                if(atoi(optarg) == 0 && optarg[0] != '0'){
                    printf("expand: tab size contains invalid character(s): ‘%s’\n", optarg);
                    exit(1);
                }
                spaceCount = atoi(optarg);
                break;
            case 'h':
                printf("Welcome to the help page of the 'expand' command.\n");
                printf("This command works like the standard unix 'expand' command.\n");
                printf("USAGE: expand [OPTION]...s [FILE]...\n");
                printf("Note here, I didn't know how to do the -t=LIST, I found no examples online on what it actually does.\n");
                printf("This command converts TABS to SPACES in each given file. By default this has 8 spaces per tab.\n");
                printf("With NO FILE or if FILE is '-', it reads from standard input.\n");
                printf("-i,            do not convert tabs after non blanks\n");
                printf("-t,            have tabs N characters apart, not 8\n");
                printf("-h,            shows this help text\n");
                exit(0);
            default:
                exit(1);
        }
    }

    int fileCounter = 0;
    if(argv[optind] != NULL && strcmp(argv[optind], "-") != 0 ){ //If there is no file or the file is named '-' fileCounter will be 0.
        for(int i = optind;i<argc;i++){ //If none of the two conditions above happen, we add all the files in an array.
            fileNames[fileCounter++] = argv[i];
        }
    }
    if(spaceCount == -1){ //Just check if we set a space with -t, if we didn't set one, set it to 8.
        spaceCount = 8;
    }


    if(fileCounter > 0){ //Here, if there is at least a file
        int i=0;
        while(fileNames[i] != NULL) { //We loop through all the files
            replaceTabs(fileNames[i], spaceCount, iFlag); //And replace the tabs for all files.
            i++;
        }
    } else { //If there is no file, it means that we take input from STDIN, so we give it a special file name.
        replaceTabs("std/in",spaceCount, iFlag);
    }

    return 0;
}
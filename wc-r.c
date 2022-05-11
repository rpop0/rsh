#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<unistd.h>


/*
 * Since the number of bytes in a file and the number of characters will always be the same, i'm just counting the chars once.
 */


int countLines(char* fileContents){
    int i=0;
    int lineCounter = 0;
    while(fileContents[i] != '\0'){ //Just loop through all the characters and count the number of newlines in the file.
        if(fileContents[i] == '\n'){
            lineCounter++;
        }
        i++;
    }
    return lineCounter;
}

int countChars(char* fileContents){
    int i = 0;
    while(fileContents[i] != '\0'){ //Count the chars
        i++;
    }
    return i;
}


int countWords(char* fileContents){
    int wordCounter = 0;
    char prev = fileContents[0];
    int i=0;
    while(fileContents[i] != '\0'){ //We loop through each character. If the current character is a space and the previous is not, we increase the word counter by one.
        if(fileContents[i] == ' ' || fileContents[i] == '\n' || fileContents[i] == '\t'){
            if(prev != ' ' && prev != '\n' && prev != '\t'){
                wordCounter++;
            }
        }
        prev = fileContents[i];
        i++;
    }
    return wordCounter;
}


int longestLine(char* fileContents){
    int i=0;
    int maxLength = 0;
    int currLineLength = 0;
    while(fileContents[i] != '\0'){
        if(fileContents[i] != '\n'){ //we loop through each character on each line
            currLineLength++;
        } else {
            if(currLineLength > maxLength){ //At the end of a line, we check if it was the longest.
                maxLength = currLineLength;
            }
            currLineLength = 0;
        }
        i++;
    }
    return maxLength;
}




int getFileLength(FILE* fp){
    int size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    return size;
}

int main(int argc, char **argv){
    int option;
    char *fileNames[50] = {NULL};
    int cFlag = 0;
    int wFlag = 0;
    int lFlag = 0;
    int bigLFlag=0;
    while(( option = getopt(argc, argv, "cwlLh")) != -1 ){ //Loop through all the arguments
        switch (option) {
            case 'c': //Assign them and set the flag for each open.
                cFlag = 1;
                break;
            case 'w':
                wFlag = 1;
                break;
            case 'l':
                lFlag = 1;
                break;
            case 'L':
                bigLFlag = 1;
                break;
            case 'h':
                printf("Welcome to the help page of the 'wc' command.\n");
                printf("This command works like the standard unix 'wc' command.\n");
                printf("USAGE: wc [OPTION]... [FILE]...\n");
                printf("It prints the number of lines, words, bytes and the longest line, based on the options.\n");
                printf("They are always printed in the following order: line, word, byte and maximum line length.\n");
                printf("-c,            prints the total byte count\n");
                printf("-w,            prints the total word count\n");
                printf("-l,            prints the total line count\n");
                printf("-L,            prints the maximum length of all the lines\n");
                exit(0);
            default:
                printf("Try 'wc -h' for more information\n");
                exit(1);
        }
    }
    int fileCounter = 0;
    if(argv[optind] != NULL && strcmp(argv[optind], "-") != 0 ){ //If there is no file or the file is named '-' fileCounter will be 0.
        for(int i = optind;i<argc;i++){ //If none of the two conditions above happen, we add all the files in an array.
            fileNames[fileCounter++] = argv[i];
        }
    }

    if(fileCounter == 0){ //If no files were found, we just read from stdin. I named the file "std/in" because files on linux can't have a slash in them.
        fileNames[0] = "std/in";
    }

//    if(cFlag == 0 && lFlag == 0 && bigLFlag == 0 && wFlag == 0){ //First, if no options are given, shows lines, words and chars
    int i=0;
    int lineTotal = 0;
    int wordTotal = 0;
    int charTotal = 0;
    int longestLineTotal = 0;

    char* fileContents = NULL;
    while(fileNames[i] != NULL){ //We loop thorugh all the files.
        FILE* fp; //Initialize the file pointer.
        if(strcmp(fileNames[0], "std/in") == 0) { //Now, if the filename is std/in.
            fp = fdopen(STDIN_FILENO, "r"); //We use the fdopen function to open STDIN_FILENO as a FILE* stream instead of a file descriptor.
        } else {
            fp = fopen(fileNames[i], "r"); //Otherwise just open the file normally.
        }
        if(fp == NULL){
            printf("Invalid file.");
            exit(1);
        }

        int size = 100; //I spent around 3 hors figuring out how to do this properly, so we start with a size of 100.
        fileContents = realloc(fileContents, size); //We usre realloc here because we are freeing the memory at the end of the while only.
        //So no matter what, the size of file's contents at the beginning of each pass through the loop will be 100.
        int j = 0;
        char ch = fgetc(fp);
        while(ch != EOF){ //We loop through all of the elements, of the file, if there is no space, just allocate more memory.
            if(j == size){
                size = size + 1000;
                fileContents = realloc(fileContents, size);
            }
            fileContents[j] = ch;
            j++;
            ch = fgetc(fp);
        }
        fileContents[j] = '\0';

        /*
         * Now the reason I did it like this, is because if we read from STDIN, like the program gets some info piped into it, we can only
         * move through the file given by STDIN once. So this is the only way that I've found that we can allocate the right amount of memory.
         * Normally we would be able to seek to the end of the file, but in the case of STDIN, we can't seek back to the beginning, so this had to be done.
         */

        if(cFlag == 0 && lFlag == 0 && bigLFlag == 0 && wFlag == 0) {
            int lines = countLines(fileContents); //Get the number from the function, display it nicely and add to total
            int words = countWords(fileContents);
            int chars = countChars(fileContents);
            if(strcmp(fileNames[0], "std/in") == 0){
                printf("%d %d %d\n", lines, words, chars);
            } else {
                printf("%d %d %d %s\n", lines, words, chars, fileNames[i]);
            }
            lineTotal += lines;
            wordTotal += words;
            charTotal += chars;
        } else {
            if(lFlag == 1){
                int lines = countLines(fileContents);
                printf("%d ", lines);
                lineTotal += lines;
            }
            if(wFlag == 1){
                int words = countWords(fileContents);
                printf("%d ", words);
                wordTotal += words;
            }
            if(bigLFlag == 1){
                int longest = longestLine(fileContents);
                printf("%d ", longest);
                longestLineTotal += longest;
            }
            if(cFlag == 1){
                int chars = countChars(fileContents);
                printf("%d ", chars);
                charTotal += chars;
            }
        }

        fclose(fp);
        i++;
    }
    free(fileContents); //And now we just free the remaining memory

    if(i > 1){ //Handling the total if there are more files.
        if(cFlag == 0 && lFlag == 0 && bigLFlag == 0 && wFlag == 0) {
            printf("%d %d %d total\n", lineTotal, wordTotal, charTotal); //Show the total if more than one file.
        }
        else {
            if(lFlag == 1){
                printf("%d ", lineTotal);
            }
            if(wFlag == 1){
                printf("%d ", wordTotal);
            }
            if(bigLFlag == 1){
                printf("%d ", longestLineTotal);
            }
            if(cFlag == 1){
                printf("%d ", charTotal);
            }
            printf("total\n");
        }
    }

    return 0;
}
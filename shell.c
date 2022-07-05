#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#define MAX 1000
//defining space for setenv to search for existing variables
#if defined (_POSIX_) || defined (__USE_POSIX)
extern char **environ;
#elif defined(_WIN32)
_CRTIMP extern char **_environ;
#endif

int totalpaths=0;
int countTotalWords(char* str)
{
    //counts total words in input string
    int i=0,count=1;
    for(i=0; i<strlen(str); i++)
    {
        if((str[i]==' ')||(str[i]=='|')||(str[i]=='<')||(str[i]=='>')||(str[i]=='\t')||(str[i]=='\b'))
        {
            count++;
        }
    }
    //keeping a buffer of 10 words to avoid errors and allocate sufficient memory
    return count+10;
}
//printing the prompt
void Prompt()
{
    //colouring the prompt magenta
    printf("\033[0;35m");
    char current_location[MAX];
    getcwd(current_location, sizeof(current_location));
    printf("\n%s", current_location);
    //reset default colour
    printf("\033[0m");
    printf("$");
}
//returns all possible paths
char** findAllPaths()
{
    int k=0;
    char* path=getenv("PATH");
    char* p=strdup(path);
    // all paths can be found by using strtok to separate paths by :
    for(int i=0; i<strlen(p); i++)
    {
        if(p[i]==':')
        {
            totalpaths++;
        }

    }
    //memory allocation
    char** allPaths=(char**)malloc(totalpaths*sizeof(char*));
    for(int j=0; j<totalpaths; j++)
    {
        allPaths[j]=(char*)malloc(MAX*sizeof(char));
    }
    //find paths using strtok
    char* temp=strtok(p, ":");

    while(temp!=NULL)
    {
        allPaths[k]=strdup(temp);
        k++;
        temp=strtok(NULL, ":");

    }
    return allPaths;
}

void parseSpace(char* str, char** parsedInput)
{
    int i;
    //duplicate str to str1 to avoid any changes in the main string
    int totalwords=countTotalWords(str);
    char* str1;
    str1 = strdup(str);
    //separate words by space
    for (i = 0; i < totalwords; i++)
    {
        parsedInput[i] = strsep(&str1, " ");
        if (parsedInput[i] != NULL)
        {
            if (strlen(parsedInput[i]) == 0)
            {
                //if string is empty, don't count it
                i--;
            }
            else continue;
        }
        else break;
    }
}
void parseForSetenv(char*str, char** parsedForSetenv)
{
    int i;
    //duplicate str to str1 to avoid any changes in the main string
    char* str1;
    str1 = strdup(str);
    //separate words by space
    for (i = 0; i < 2; i++)
    {
        parsedForSetenv[i] = strsep(&str1, "=");
        if (parsedForSetenv[i] != NULL)
        {
            if (strlen(parsedForSetenv[i]) == 0)
            {
                //if string is empty, don't count it
                i--;
            }
            else continue;
        }
        else break;
    }
}
void echoEnvirnmentVariable(char*str, char** echoString)
{
    int i;
    //duplicate str to str1 to avoid any changes in the main string
    char* str1;
    str1 = strdup(str);
    int totalwords=countTotalWords(str);
    //separate words by space
    for (i = 0; i < totalwords; i++)
    {
        echoString[i] = strsep(&str1, "$");
        if (echoString[i] != NULL)
        {
            if (strlen(echoString[i]) == 0)
            {
                //if string is empty, don't count it
                i--;
            }
            else continue;
        }
        else break;
    }
}
int parsePipe(char* str, char** pipeParsedInput)
{
    int i;
    int totalPipes=0;
    int totalwords=countTotalWords(str);
    //duplicate str to str1 to avoid any changes in the main string
    char* str1;
    str1 = strdup(str);
    for(int i=0; i<strlen(str1); i++)
    {
        if(str1[i]=='|')totalPipes++;
    }
    //separate words by pipe
    for (i = 0; i < totalwords; i++)
    {
        pipeParsedInput[i] = strsep(&str1, "|");

        if (pipeParsedInput[i] != NULL)
        {
            //if string is empty, don't count it.
            if (strlen(pipeParsedInput[i]) == 0)
            {
                i--;
            }
            else continue;
        }
        else break;

    }
    //no pipes
    if(pipeParsedInput[1]==NULL)
    {
        return 0;
    }
    //1 valid pipe
    else if(pipeParsedInput[1]!=NULL && pipeParsedInput[2]==NULL)
    {
        return 1;
    }
    //2 pipes
    else if(totalPipes==2)
    {
        return 2;
    }
// more than 2 pipes have not been handled by the corresponding functions.
    else return 0;
}
void executeSetenv(char* str, char** parsedForSetenv)
{
    if (setenv(parsedForSetenv[0], parsedForSetenv[1], 1) != 0)
    {
        perror("setenv");
        //command failed
        return;
    }

    if (environ != NULL)
    {
        for (int i = 0; environ[i]!=NULL; i++)
        {
            if (strcmp(str,environ[i]) == 0)
            {
                //command succeded
                printf("%s\n",environ[i]);
                return;
            }
        }
        printf("Variable non-existent.\n");
    }
    return;
}

//declaration of a global variable to check path validity, if path is valid it will be 1.
int checkValidityOfCommand=0;


void simple_commands(char**parsedInput)
{
    //find all possible valid paths by concatinating input command and possible paths
    char** possiblePaths=findAllPaths();
    char* s1;
    int flag[totalpaths];
    int i=0;
    for(i=0; i<totalpaths; i++)
    {
        flag[i]=0;
    }
    for(i=0; i<totalpaths; i++)
    {
        //memory allocation
        s1=(char*)malloc(MAX*sizeof(char));
        s1=strdup(possiblePaths[i]);
        //making the path
        strcat(s1,"/");
        strcat(s1,parsedInput[0]);
        //checking if path is valid
        if(access(s1,X_OK)==0 && flag[i]==0)
        {
            checkValidityOfCommand=1;
            //create child process
            pid_t child_0 = fork();
            if(child_0==0)
            {
                execv(s1,parsedInput);
                exit(0);
            }
            else if(child_0 > 0)
            {
                //wait for its execution
                wait(NULL);
                return ;
            }

            flag[i]=1;
            continue;
        }
    }
    free(s1);
    return;

}
//to handle single level piping
void pipe_commands(char** pipeParsedInput)
{
//separate statements around |
    char** beforePipe=NULL;
    char** afterPipe=NULL;
    beforePipe=(char**)malloc(MAX*sizeof(char*));
    afterPipe=(char**)malloc(MAX*sizeof(char*));
    for(int j=0; j<MAX; j++)
    {
        beforePipe[j]=(char*)malloc(MAX*sizeof(char));
        afterPipe[j]=(char*)malloc(MAX*sizeof(char));
    }
    char** possiblePaths=findAllPaths();
// remove white spaces
    parseSpace(pipeParsedInput[0],beforePipe);
    parseSpace(pipeParsedInput[1],afterPipe);

    int i;
    for(i=0; i<totalpaths; i++)
    {
        char*s2;
        s2=(char*)malloc(MAX*sizeof(char));
        s2=strdup(possiblePaths[i]);
        strcat(s2,"/");
        strcat(s2,afterPipe[0]);
        //check if command is valid
        if(access(s2,X_OK)==0)
        {
            //create file descriptor
            int fileDescriptor[2];
            pid_t child_1, child_2;
            if(pipe(fileDescriptor)<0)
            {
                printf("Could not allocate the read and write ends.\n");
                return;
            }
            //fork to obtain child process
            child_1=fork();
            if(child_1<0)
            {
                printf("Could not fork to give child_1.\n");
                return;
            }
            if(child_1==0)
            {
                close(fileDescriptor[0]);
                dup2(fileDescriptor[1], STDOUT_FILENO);
                close(fileDescriptor[1]);

                int command1 = execvp(beforePipe[0],beforePipe);
                if(command1<0)
                {
                    printf("Command 1 cannot be executed.\n");
                    exit(0);
                }
            }
            else
            {
                child_2 = fork();

                if (child_2 < 0)
                {
                    printf("\nCould not fork to give child_2.\n");
                    return;
                }
                if (child_2 == 0)
                {
                    close(fileDescriptor[1]);
                    dup2(fileDescriptor[0], STDIN_FILENO);
                    close(fileDescriptor[0]);
                    if (execvp(afterPipe[0], afterPipe) < 0)
                    {
                        printf("Command 2 cannot be executed.\n");
                        exit(0);
                    }
                }
                else
                {
                    wait(NULL);
                    wait(NULL);
                }
            }

        }
    }
    //free memory
    for(int j=0; j<MAX; j++)
    {
        free(beforePipe[j]);
        free(afterPipe[j]);
    }
    free(beforePipe);
    free(afterPipe);

}
// to handle double level piping
int double_pipe_commands(int input, int first, int last, char** pipeParsedInput,int flag)
{
    pid_t child_6;
    int command_pipe[2];
    //separate statements around |
    char** beforePipe1=NULL;
    char** afterPipe1=NULL;
    char** afterPipe2=NULL;
    //allocate memory
    beforePipe1=(char**)malloc(MAX*sizeof(char*));
    afterPipe1=(char**)malloc(MAX*sizeof(char*));
    afterPipe2=(char**)malloc(MAX*sizeof(char*));
    for(int j=0; j<MAX; j++)
    {
        beforePipe1[j]=(char*)malloc(MAX*sizeof(char));
        afterPipe1[j]=(char*)malloc(MAX*sizeof(char));
        afterPipe2[j]=(char*)malloc(MAX*sizeof(char));
    }
    char** possiblePaths=findAllPaths();
    // remove white spaces
    parseSpace(pipeParsedInput[0],beforePipe1);
    parseSpace(pipeParsedInput[1],afterPipe1);
    parseSpace(pipeParsedInput[2],afterPipe2);

    int i;
    for(i=0; i<totalpaths; i++)
    {
        char*s2;
        s2=(char*)malloc(MAX*sizeof(char));
        s2=strdup(possiblePaths[i]);
        strcat(s2,"/");
        strcat(s2,afterPipe1[0]);
        //check if command is valid
        if(access(s2,X_OK)==0)
        {
            int pipettes[2];
            //Invoke pipes
            pipe( pipettes );
            //fork to give child_6
            child_6 = fork();
            if (child_6 == 0)
            {
                if (first == 1 && last == 0 && input == 0)
                {
                    // First command
                    dup2( pipettes[1], STDOUT_FILENO );
                }
                else if (first == 0 && last == 0 && input != 0)
                {
                    // Middle command
                    dup2(input, STDIN_FILENO);
                    dup2(pipettes[1], STDOUT_FILENO);
                }
                else
                {
                    // Last command
                    dup2( input, STDIN_FILENO );
                }
                //flag==1 has been used for first command
                if(flag==1)
                {
                    if (execvp(beforePipe1[0], beforePipe1) == -1)
                        _exit(EXIT_FAILURE);
                }
                //flag==2 has been used for middle command
                else if(flag==2)
                {
                    if (execvp(afterPipe1[0], afterPipe1) == -1)
                        _exit(EXIT_FAILURE);
                }
                //flag==3 has been used for last command
                else if(flag==3)
                {
                    if (execvp(afterPipe2[0], afterPipe2) == -1)
                        _exit(EXIT_FAILURE);
                }
            }

            if (input != 0)
                close(input);

            // No new commands to be written
            close(pipettes[1]);

            // last==1 indicates last command, i.e.Nothing more needs to be read
            if (last == 1)
                close(pipettes[0]);

            return pipettes[0];
        }
    }
//free all the memory
    for(int j=0; j<MAX; j++)
    {
        free(beforePipe1[j]);
        free(afterPipe1[j]);
        free(afterPipe2[j]);
    }
    free(beforePipe1);
    free(afterPipe1);
    free(afterPipe2);
}

char checkOutputRedirection(char* str, char** redirectOutput)
{

    int i;
    int totalwords=countTotalWords(str);
    //duplicate str to str1 to avoid any changes in the main string
    char* str1;
    str1 = strdup(str);
    for (i = 0; i < totalwords; i++)
    {
        //parse to separate the strings before and after >
        redirectOutput[i] = strsep(&str1, ">");

        if (redirectOutput[i] != NULL)
        {
            if (strlen(redirectOutput[i]) == 0)
            {
                i--;
            }
            else continue;
        }
        else break;
    }
    i=0;
    //nothing after symbol
    if(redirectOutput[1]==NULL)
    {
        return 'f';
    }
    else return 't';


}
char checkInputRedirection(char* str, char** redirectInput)
{
    int i;
    int totalwords=countTotalWords(str);
    //duplicate str to str1 to avoid any changes in the main string
    char* str1;
    str1 = strdup(str);
    for (i = 0; i < totalwords; i++)
    {
        //parse to separate the strings before and after <
        redirectInput[i] = strsep(&str1, "<");
        if (redirectInput[i] != NULL)
        {
            if (strlen(redirectInput[i]) == 0)
            {
                i--;
            }
            else continue;
        }
        else break;
    }
//nothing after symbol
    if(redirectInput[1]==NULL)
    {
        return 'f';
    }
    else return 't';


}
char checkOutputRedirectionToAppend(char* str, char** redirectOutputToAppend)
{

    int i;
    //find total words
    int totalwords=countTotalWords(str);
    char* str1;
    //duplicate input string to another string to avoid changes in the original string.
    str1 = strdup(str);
    int flag=0;
    for(int i=0; i<strlen(str1); i++)
    {
        // check for '>>'
        if(str[i]=='>'&& str[i+1]=='>')
        {
            flag=1;
            break;
        }
    }
    if(flag==1)
    {
        //using strsep to parse symbol.
        for (i = 0; i < totalwords; i++)
        {
            redirectOutputToAppend[i] = strsep(&str1, ">");
            if(redirectOutputToAppend[i] != NULL)
            {
                if (strlen(redirectOutputToAppend[i]) == 0)
                {
                    i--;
                }
            }
            else
            {
                break;
            }

        }
        i=0;
        if(redirectOutputToAppend[1]==NULL)
        {
            //if there is nothing after symbol
            return 'f';
        }
        else return 't';
    }
//if the symbol is not found
    else return 'f';
}
void outputRedirection(char** redirectOutput)
{
    //store statemens before and after symbol
    char** beforeOutputPipe=NULL;
    char** afterOutputPipe=NULL;
    //memory allocation
    beforeOutputPipe=(char**)malloc(MAX*sizeof(char*));
    afterOutputPipe=(char**)malloc(MAX*sizeof(char*));
    for(int j=0; j<MAX; j++)
    {
        beforeOutputPipe[j]=(char*)malloc(MAX*sizeof(char));
        afterOutputPipe[j]=(char*)malloc(MAX*sizeof(char));
    }
    //remove whitespace
    parseSpace(redirectOutput[0],beforeOutputPipe);
    parseSpace(redirectOutput[1],afterOutputPipe);
    //create file descriptor
    int fdOutput;
    pid_t child_3;

    //fork to give child_3
    child_3=fork();
    if(child_3==0)
    {
        fdOutput = open(afterOutputPipe[0],O_WRONLY | O_CREAT,0666);
        dup2(fdOutput,1);
        execvp(beforeOutputPipe[0],beforeOutputPipe);
        close(fdOutput);
    }
    //free memory
    free(beforeOutputPipe);
    free(afterOutputPipe);
}
void outputRedirectionToAppend(char** redirectOutput)
{
    //store statemens before and after symbol
    char** beforeAppendSign=NULL;
    char** afterAppendSign=NULL;
    //allocate memory
    beforeAppendSign=(char**)malloc(MAX*sizeof(char*));
    afterAppendSign=(char**)malloc(MAX*sizeof(char*));
    for(int j=0; j<MAX; j++)
    {
        beforeAppendSign[j]=(char*)malloc(MAX*sizeof(char));
        afterAppendSign[j]=(char*)malloc(MAX*sizeof(char));
    }
    //remove whitespace
    parseSpace(redirectOutput[0],beforeAppendSign);
    parseSpace(redirectOutput[1],afterAppendSign);
    //create file descriptor
    int fdAppend;
    pid_t child_5;
    //fork to give child_5
    child_5=fork();
    if(child_5==0)
    {
        fdAppend = open(afterAppendSign[0],O_CREAT | O_RDWR | O_APPEND,0666);
        dup2(fdAppend,1);
        execvp(beforeAppendSign[0],beforeAppendSign);
        close(fdAppend);
    }
    //free memory
    free(beforeAppendSign);
    free(afterAppendSign);
}
void inputRedirection(char** redirectInput)
{
    //separate statements before and after '<'
    char** beforeInputPipe=NULL;
    char** afterInputPipe=NULL;
    //allocate memory
    beforeInputPipe=(char**)malloc(MAX*sizeof(char*));
    afterInputPipe=(char**)malloc(MAX*sizeof(char*));
    for(int j=0; j<MAX; j++)
    {
        beforeInputPipe[j]=(char*)malloc(MAX*sizeof(char));
        afterInputPipe[j]=(char*)malloc(MAX*sizeof(char));
    }
    //remove white spaces
    parseSpace(redirectInput[0],beforeInputPipe);
    parseSpace(redirectInput[1],afterInputPipe);
    // create file descriptor for input redirection
    int fdInput;
    pid_t child_4;
    //create child process
    child_4=fork();
    if(child_4==0)
    {
        //open input file in read mode
        fdInput = open(afterInputPipe[0],O_RDONLY, 0);
        dup2(fdInput,0);
        execvp(beforeInputPipe[0],beforeInputPipe);
        close(fdInput);
    }
    //free memory
    for(int j=0; j<MAX; j++)
    {
        free(beforeInputPipe[j]);
        free(afterInputPipe[j]);
    }
    free(beforeInputPipe);
    free(afterInputPipe);
}

void EchoCommand(char* str)
{
    int totalwords=countTotalWords(str);
    int double_inverted_comma=0, single_inverted_comma=0, overlap=1,checkForEnvironmentVariable=0, first_occourance=0;
    int single_first=0, single_second=0,double_first=0,double_second=0;
    char* str1;
    str1=strdup(str);
    int j=0;
    for(j=0; j<strlen(str1); j++)
    {
        //checking for environment variables.
        if(str1[j]=='$')
        {
            checkForEnvironmentVariable=1;
        }
    }
    int i=0;
    //traverse till first argument after echo
    while(str1[i]!=' ')
    {
        i++;
    }
    while(str1[i]==' ')
    {
        i++;
    }
    int statementStart=i,k=0;
    //if environment variable is used with echo
    if(checkForEnvironmentVariable==1)
    {
        char** echoString=(char**)malloc(sizeof(char*)*totalwords);
        for(k=0; k<totalwords; k++)
        {
            echoString[k]=(char*)malloc(sizeof(char)*MAX);
        }
        echoEnvirnmentVariable(str, echoString);
        k=0;
        if(getenv(echoString[1])!=NULL)
        {
            printf("%s\n",getenv(echoString[1]));
        }

    }
    else if(checkForEnvironmentVariable==0)
    {
        //if "" or '' are used the execution takes place according to the specifications mentioned in README file
        for(; i<strlen(str1); i++)
        {
            if(str1[i]=='"')
            {
                double_inverted_comma++;
                if(first_occourance==0)
                {
                    first_occourance=2;
                }
                if(double_inverted_comma==1)
                    double_first=i;
                if(double_inverted_comma==2)
                    double_second=i;
            }
            //39 is the ASCII representation for '
            if(str1[i]==39)
            {
                single_inverted_comma++;
                if(first_occourance==0)
                {
                    first_occourance=1;
                }
                if(single_inverted_comma==1)
                    single_first=i;
                if(single_inverted_comma==2)
                    single_second=i;
            }
        }
        //overlap=1 indicates " ' ' " or ' "" ' ; overlap=0 indicates "" '' or '' ""
        if(double_inverted_comma!=0 && single_inverted_comma!=0)
        {
            if(double_second<single_first)
            {
                overlap=0;
            }
            if(single_second<double_first)
            {
                overlap=0;
            }
        }



        //the inverted commas must be closed
        if(single_inverted_comma%2==1 && first_occourance==1)
        {
            printf("Invalid command, close Inverted comma.\n");
        }
        else if(double_inverted_comma%2==1 && first_occourance==2)
        {
            printf("Invalid command, close Inverted comma.\n");
        }
        //according to whichever of the 2 types preceeds in overlap=1 case, output has been printed skipping the 1st of them. Note that only 1 time overlap has been handled.
        else if(first_occourance==1 && overlap==1)
        {
            for(i=statementStart; i<strlen(str1); i++)
            {
            //ignore '
                if(str1[i]==39)
                {
                    continue;
                }
                else
                {
                    printf("%c",str1[i]);
                }
            }
        }
        else if(first_occourance==2 && overlap==1)
        {
            for(i=statementStart; i<strlen(str); i++)
            {
            //ignore "
                if(str1[i]=='"')
                {
                    continue;
                }
                else
                {
                    printf("%c",str1[i]);
                }
            }
        }
        else if(double_inverted_comma==0 && single_inverted_comma==0)
        {
            for(i=statementStart; i<strlen(str); i++)
            {
                printf("%c",str1[i]);
            }
        }
        else if(overlap==0)
        {
            for(i=statementStart; i<strlen(str); i++)
            {
            //ignore "
                if(str1[i]=='"')
                {
                    continue;
                }
                if(str1[i]==39)
                {
                    continue;
                }
                else
                {
                    printf("%c",str1[i]);
                }
            }
        }
    }
}

int main(int argc, char *argv[], char * environmentVariables[])
{
    int i=0;
    //declare pointers
    char* inputString=NULL;
    char* buffer=NULL;

    char** parsedInput=NULL;
    char** pipeParsedInput=NULL;
    char** parsedForSetenv=NULL;
    char** redirectOutput=NULL;
    char** redirectInput=NULL;
    char** redirectOutputToAppend=NULL;

    FILE* f1=fopen("/tmp/history.txt","w+");
    printf("\033[0;36m");
    printf("\n-----------------------------------MINI SHELL-----------------------------------\n");
    printf("\033[0m");
    while (1)
    {
        checkValidityOfCommand=0;
        //---------------------------------------------------------------------MEMORY ALLOCATION FOR INPUT------------------------------------------------------------------
        if(buffer==NULL)
        {
            buffer=(char*)malloc(MAX*sizeof(char));
        }
        else if(buffer!=NULL)
        {
            //free the memory initially allocated, and allocate new memory
            free(buffer);
            buffer=(char*)malloc(MAX*sizeof(char));
        }
        if(inputString==NULL)
        {
            inputString=(char*)malloc(MAX*sizeof(char));
        }
        else
        {
            //free the memory initially allocated, and allocate new memory
            free(inputString);
            inputString=(char*)malloc(MAX*sizeof(char));
        }
        //----------------------------------------------------------------READ INPUT AND GET TOTAL NUMBER OF WORDS-------------------------------------------------------------
        // print current directory path
        Prompt();

        //read input and store it in a buffer
        buffer = readline("");
        if (strlen(buffer) != 0)
        {
            fprintf(f1,"%s\n",buffer);
            strcpy(inputString, buffer);
        }
        int totalWords=countTotalWords(inputString);
        //-------------------------------------------------------------------MEMORY ALLOCATION AND FREEING----------------------------------------------------------------------
        if(parsedForSetenv==NULL)
        {
            parsedForSetenv=(char**)malloc(MAX*sizeof(char*));
        }
        else
        {
            //free initially allocated memory
            free(parsedForSetenv);
            parsedForSetenv=(char**)malloc(2*sizeof(char*));
        }
        for(int j=0; j<2; j++)
        {
            parsedForSetenv[j]=(char*)malloc(sizeof(char)*MAX);
        }

        if(parsedInput==NULL)
        {
            parsedInput=(char**)malloc(MAX*sizeof(char*));
        }
        else
        {
            //free the memory initially allocated, and allocate new memory
            free(parsedInput);
            parsedInput=(char**)malloc(totalWords*sizeof(char*));
        }
        for(int j=0; j<totalWords; j++)
        {
            parsedInput[j]=(char*)malloc(sizeof(char)*MAX);
        }

        if(pipeParsedInput==NULL)
        {
            pipeParsedInput=(char**)malloc(MAX*sizeof(char*));
        }
        else
        {
            int j=0;
            //free the memory initially allocated, and allocate new memory
            free(pipeParsedInput);
            pipeParsedInput=(char**)malloc(totalWords*sizeof(char*));
        }
        for(int j=0; j<totalWords; j++)
        {
            pipeParsedInput[j]=(char*)malloc(sizeof(char)*MAX);
        }
        if(redirectOutput==NULL)
        {
            redirectOutput=(char**)malloc(totalWords*sizeof(char*));
        }
        else
        {
            int j=0;
            //free the memory initially allocated, and allocate new memory
            free(redirectOutput);
            redirectOutput=(char**)malloc(totalWords*sizeof(char*));
        }
        for(int j=0; j<totalWords; j++)
        {
            redirectOutput[j]=(char*)malloc(sizeof(char)*MAX);
        }
        if(redirectInput==NULL)
        {
            redirectInput=(char**)malloc(totalWords*sizeof(char*));
        }
        else
        {
            int j=0;
            //free the memory initially allocated, and allocate new memory
            free(redirectInput);
            redirectInput=(char**)malloc(totalWords*sizeof(char*));
        }
        for(int j=0; j<totalWords; j++)
        {
            redirectInput[j]=(char*)malloc(sizeof(char)*MAX);
        }
        if(redirectOutputToAppend==NULL)
        {
            redirectOutputToAppend=(char**)malloc(MAX*sizeof(char*));
        }
        else
        {
            int j=0;
            //free the memory initially allocated, and allocate new memory
            free(redirectOutputToAppend);
            redirectOutputToAppend=(char**)malloc(totalWords*sizeof(char*));
        }
        for(int j=0; j<totalWords; j++)
        {
            redirectOutputToAppend[j]=(char*)malloc(sizeof(char)*MAX);
        }

        //-----------------------------------------------------------------------INDICATOR VARIABLES----------------------------------------------------------------------------
        int indicatingIfPiped=parsePipe(inputString, pipeParsedInput);
        char indicatingOutputRedirection=checkOutputRedirection(inputString, redirectOutput);
        char indicatingOutputRedirectionToAppend=checkOutputRedirectionToAppend(inputString, redirectOutputToAppend);
        char indicatingInputRedirection=checkInputRedirection(inputString, redirectInput);
        //---------------------------------------------------------------------------EXECUTION----------------------------------------------------------------------------------


        //--------------------------------------------------------------------------ENVIRONMENT  VARIABLES----------------------------------------------------------------------
        int i=0,checkForSetenv=0;
        //remove whitespace
        parseSpace(inputString, parsedInput);
        for(i=0; i<strlen(inputString); i++)
        {
            if(inputString[i]=='=')
            {
                checkForSetenv=1;
                break;
            }
        }
        i=0;
        //printing all the global variables using printenv
        if(strcmp(inputString,"printenv")==0)
        {
            if(environmentVariables!=NULL)
            {
                while(environmentVariables[i]!=NULL)
                {
                    printf("> %s\n",environmentVariables[i]);
                    i++;
                }
            }

        }
        else if(strcmp(parsedInput[0],"printenv")==0)
        {
            if(getenv(parsedInput[1])!=NULL)
            {
                printf("%s\n",getenv(parsedInput[1]));
            }
        }
        //executing setenv command
        else if(checkForSetenv==1)
        {
            parseForSetenv(inputString, parsedForSetenv);
            //printf("%s\n%s\n",parsedForSetenv[0],parsedForSetenv[1]);
            executeSetenv(inputString, parsedForSetenv);

        }

        //--------------------------------------------------------------------------INTERNAL COMMANDS---------------------------------------------------------------------------

        //--------------------------------------------------------------------------CHANGE DIRECTORY - CD-------------------------------------------------------------------------
        else if(strcmp(inputString,"cd")==0)
        {
            char* home=getenv("HOME");
            if(chdir(home)==-1)
            {
                printf("Error while using getenv.\n");
            }
        }


        //cd to specific directory
        else if(strcmp(parsedInput[0],"cd")==0)
        {
            chdir(parsedInput[1]);
        }
        //--------------------------------------------------------------------------HISTORY-------------------------------------------------------------------------------------
        else if(strcmp(parsedInput[0],"history")==0)
        {
            //executing linux command cat history.txt using C
            fseek(f1,0L,SEEK_SET);
            char c=fgetc(f1);
            while(c!=EOF)
            {
                printf("%c",c);
                c=fgetc(f1);
            }
        }
        //CLEAR HISTORY AND REMOVE FILE
        else if((strcmp(parsedInput[0],"quit")==0)||(strcmp(parsedInput[0],"exit")==0)||(strcmp(parsedInput[0],"x")==0))
        {
            if(remove("/tmp/history.txt")==0)
            {
                printf("The history.txt file has been removed & program will terminate.\n");
            }
            break;
        }
        //-------------------------------------------------------------------------ECHO--------------------------------------------------------------------------------------------
        //ECHO WITH INVERTED COMMAS- handles cases with '' and "" too.
        //It will work even if <,>,| etc. are added WITHIN '' OR "".
        else if(strcmp(parsedInput[0],"echo")==0)
        {
            if(indicatingIfPiped==0 && indicatingInputRedirection=='f' && indicatingOutputRedirection=='f' && indicatingOutputRedirectionToAppend=='f')
            {
                EchoCommand(inputString);
            }
            else
            {
                simple_commands(parsedInput);
            }
        }
        else
        {
            //---------------------------------------------------------------EXECUTION OF LINUX COMMANDS----------------------------------------------------------------------------


            if(indicatingIfPiped==0 && indicatingOutputRedirection=='f' && indicatingInputRedirection=='f')
            {
                simple_commands(parsedInput);
                if(checkValidityOfCommand==0)
                {
                    printf("Invalid Command.\n");
                    break;
                }
            }
            //-------------------------------------------------------------EXECUTION OF PIPED COMMANDS-------------------------------------------------------------------------------
            else if(indicatingIfPiped==1)
            {
                pipe_commands(pipeParsedInput);
            }
            //------------------------------------------------------------EXECUTION OF DOUBLE-PIPED COMMANDS-------------------------------------------------------------------------
            else if(indicatingIfPiped==2)
            {
                int input=0,first=1;
                input=double_pipe_commands(input, first, 0, pipeParsedInput,1);
                first=0;
                input=double_pipe_commands(input, first, 0, pipeParsedInput,2);
                first=0;
                input=double_pipe_commands(input, first, 1, pipeParsedInput,3);

                int i;
                for (i = 0; i < 2; ++i)
                {
                    wait(NULL);
                }

            }
            //---------------------------------------------------------------REDIRECTION-------------------------------------------------------------------------------------------
            //----------OUTPUT REDIRECTION - FILE: WRITE---------
            else if(indicatingOutputRedirection=='t' && indicatingOutputRedirectionToAppend=='f' && indicatingInputRedirection=='f')
            {
                outputRedirection(redirectOutput);
            }
            //------------INPUT REDIRECTION - FILE: READ----------
            else if(indicatingInputRedirection=='t' && indicatingOutputRedirection=='f' && indicatingOutputRedirectionToAppend=='f')
            {
                inputRedirection(redirectInput);
            }
            //---------OUTPUT REDIRECTION - FILE: APPEND-----------
            else if(indicatingOutputRedirectionToAppend=='t' && indicatingInputRedirection=='f')
            {
                outputRedirectionToAppend(redirectOutputToAppend);
            }
            else
            {
                printf("Invalid Command.\n");
                break;
            }

        }
    }
}














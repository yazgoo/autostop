#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
int autostop(char* command, char* regexp, int passes)
{
    char* pids = "echo $$ && ";
    int commandLen = strlen(command);
    int pidsLen = strlen(pids);
    char pidAndcommand[commandLen + pidsLen];
    strcpy(pidAndcommand, pids);
    strcpy(pidAndcommand + pidsLen, command);
    FILE* output = popen(pidAndcommand, "r");
    if(output == NULL) return 1;
    setvbuf(output, NULL, _IONBF, 0);
    char lineBuffer[BUFSIZ];
    int result;
    int pid;
    fgets(lineBuffer, BUFSIZ, output);
    result = sscanf(lineBuffer, "%d\n", &pid);
    regex_t regex;
    regmatch_t regmatch;
    if(!regcomp(&regex, regexp, REG_EXTENDED))
    {
        while(fgets(lineBuffer, BUFSIZ, output) != NULL)
        {
            printf("%s", lineBuffer);
            if(!regexec(&regex, lineBuffer, 1, &regmatch, 0))
            {
                if(passes <= 0)
                {
                    kill(pid, SIGSTOP);
                    fgetc(stdin);
                    kill(pid, SIGCONT);
                }
                passes--;
            }
        }
        regfree(&regex);
    }
    return pclose(output);
}
int displayUsage(char** argv)
{
    printf("Usage: %s command regex\n", argv[0]);
    return 0;
}
int displayHelp(char** argv)
{
    printf("Issue a command and stop on regex matches\n");
    displayUsage(argv);
    printf("Parameters:\n");
    printf("   command     the command you want to stop\n");
    printf("   regex       the regexp you want to trigger the stop\n");
    printf("Options:\n");
    printf("   --help|-h   display this help\n");
    printf("   -pN         passes N times before stoping\n");
    return 0;
}
int main(int argc, char** argv)
{
    char* command, *regexp;
    char* argument;
    int j = 0;
    int i;
    int passes = 0;
    int tmp;
    for(i = 0; i < argc; i++)
    {
        argument = argv[i];
        if(argument[0] == '-')
        {
            if(!strcmp(argument, "--help") || !strcmp(argument, "-h"))
                return displayHelp(argv);
            else if(sscanf(argument, "-p%d", &tmp) == 1) passes = tmp;
            else
                return displayUsage(argv);
        }
        else
        {
            if(j == 1) command = argument;
            else if(j == 2) regexp = argument;
            else if(j != 0) return displayUsage(argv);
            j++;
        }
    }
    if(j != 3) return displayUsage(argv);
    autostop(command, regexp, passes);
    return 0;
}

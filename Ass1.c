#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char** str_split(char*, const char);
void arrangeArg(char**,char*);
int interactiveMode(char**,int);
int batchMode(char**,int);
char* splitSemi(char*,int);

int main(int argc, char *argv[] )
{
    char *cmd[1000];
    char **subcmd[1000];
    int status=1,totalSemi=0,i;
    char text[1000],*tmptext;

//if argc =2 will read file form argument#2 and run batchMode
    if( argc == 2 ) {
        cmd[1]=argv[1]; //keep argument in cmd variable
        status=batchMode(cmd,status);

    }
    else if( argc > 2 ) { //recevive only 2 argument 
        fprintf(stderr,"Too many arguments supplied.\n");
        return 0;
    }

    while(status) //status =0 when receive quit command
    {
        printf("prompt > ");
        scanf(" %[^\n]",text);
	    totalSemi = countSemi(text);
    for(i=0 ;i<totalSemi+1 ;i++)
	{
		tmptext = splitSemi(text,i);
		arrangeArg(cmd,tmptext);
		if(!strcasecmp(cmd[0],"shell") )//if write "shell" follow by {filename} will also run file using batchMode
        	{
            		status=batchMode(cmd,status);
        	}
        	else
        	{
            		status=interactiveMode(cmd,status);
        	}
	}        
    }
    return 0;
}

//arrange argument to correct syntax before do str_split
void arrangeArg(char** cmd,char* text)
{
    char **tokens;
    char **str;


   char blank[1000];
   int c = 0, d = 0;
   int i=0,j=0;

   int current=0;
 //remove blank if between word have more than 1 blank
   while (text[c] != '\0') {
      if (text[c] == ' ') {
         int temp = c + 1;
         if (text[temp] != '\0') {
            while (text[temp] == ' ' && text[temp] != '\0') {
               if (text[temp] == ' ') {
                  c++;
               }  
               temp++;
            }
         }
      }
      blank[d] = text[c];
      c++;
      d++;
   }
 
   blank[d] = '\0';
    
    tokens = str_split(blank,' '); //seperate each argument with spacebar in function str_split and keep each argument in cmd

        if (tokens)
        {
            int i;
            int pid;
            for (i = 0; *(tokens + i); i++)
            {
                cmd[i]=tokens[i];
		
            }
            cmd[i]=NULL;
        }
	
}

//run command after arrangeArg and str_split using cmd variable
int interactiveMode(char** cmd,int status)
{
    if(!fork()){
        if(execvp(cmd[0],cmd)<0 && strcasecmp(cmd[0],"quit")){
            fprintf(stderr,"\'%s\'is not recognized as an internal or external command,\noperable program or batch file.\n",cmd[0]); //don't have command 
        }
            exit(0);
    }else{
        wait();
        if(!strcasecmp(cmd[0],"quit"))
        {
            printf("Exit program...\n"); //exit program
            status=0;
        }
    }
    return status;
    

}

//read file and run command same as interactiveMode but read command form file
int batchMode(char** cmd,int status)
{
    FILE *fp;
    FILE *file = fopen ( cmd[1], "r" );
    int totalSemi=0,i=0;
	char **tmptext;

    if ( file != NULL )
    {
       char line [ 128 ]; /* or other suitable maximum line size */
       while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
       {
            strtok(line, "\r\n"); //fgets will consume \r\n so this is remove unusable character
	        totalSemi = countSemi(line);
        for(i=0 ;i<totalSemi+1 ;i++)
        {
            tmptext = splitSemi(line,i);
            arrangeArg(cmd,tmptext);
            status=interactiveMode(cmd,status);
        }

       }
       fclose ( file );
    }
    else
    {
       perror ( cmd[1] ); /* why didn't the file open? */
    }
    return status;
    
}

//spilt each argument and keep it in token variable
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
/* count all semicolon in text in put*/
int countSemi(char text[])
{
	int i,count=0;
	for(i=0 ; i<strlen(text);i++)
	{
		if(text[i]==';')
		{
			count++;
		}
	}
	return count;
}
/* split semicolon and return substring for cmd */
char *splitSemi(char *text,int index)
{
	int i,old=0,count=0;
	char *sub = (char *)malloc(sizeof(char)*1000);
	for(i=0 ;i<strlen(text) ;i++)
	{
		if(text[i]==';')
		{
			if(count==index)
			{
				memcpy(sub,&text[old],i-old);
				sub[i-old]='\0';
				break;
			}else {
				old = i+1;
				count++;
			}
		}else if(i==strlen(text)-1)
		{
			memcpy(sub,&text[old],i-old+1);
			sub[i-old+1]='\0';
			break;
		}
	}
	while(sub[0]==' '){
		memcpy(sub,&sub[1],strlen(sub)-1);
		sub[strlen(sub)-1]='\0';
	}
	return sub;
}























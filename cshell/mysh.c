//usr/bin/make -s "${0%.c}" && ./"${0%.c}" "$@"; s=$?; rm ./"${0%.c}"; exit $s
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#define DEF(a,b) ((a) ? (a) : (b))

#define IN
#define OUT
#define INOUT

int makeargv (
    IN	const char *   s, 		// String to be parsed
    IN	const char *   delimiters,	// String of delimiters
    OUT       char *** argvp		// Argument array
    )
{
    int		error;
    int		i;
    int		numtokens;
    const char * snew;
    char       * t;

    // Make sure that none of the inputs is NULL

    if ( ! ( s && delimiters && argvp ) )
    {
	errno = EINVAL;
	return ( -1 );
    }

    *argvp = NULL;
    snew = s + strspn ( s, delimiters );	// Real start of string

    t = ( char * ) malloc ( strlen ( snew ) + 1 );
    if ( !t )
    {
	fprintf ( stderr, "Error allocating memory\n" );
	return ( -1 );
    }

    strcpy ( t, snew );
    numtokens = 0;

    // Count the number of tokens

    if ( strtok ( t, delimiters ) )
	for ( numtokens = 1; strtok ( NULL, delimiters ); numtokens++ );

    // Create argument array for pointers to tokens

    *argvp = ( char ** ) malloc ( ( numtokens + 1 ) * sizeof ( char * ) );
    error = errno;
    if ( ! *argvp )
    {
	free ( t );
	errno = error;
	return ( -1 );
    }

    if ( ! numtokens )
	free ( t );
    else
    {
	strcpy ( t, snew );
	**argvp = strtok ( t, delimiters );
	for ( i = 1; i < numtokens; i++ )
	{
	    *((*argvp) + i ) = strtok ( NULL, delimiters );
	}
    }
    *(( *argvp ) + numtokens ) = NULL;
    return ( numtokens );
}


void freemakeargv (
    INOUT char ** argv
    )
{
    if ( ! argv )
	return;

    if ( ! *argv )
	free ( * argv );

    free ( argv );
}



//traverse the path to find a matching executable
int searchPath(char * command, char ** found, struct stat * info, char ** path, int pathlen)
{
  char * dir;
  char full[40];
  int i;

  for (i = 0; i < pathlen; i++ ){
    sprintf(full,"%s/%s",path[i],command);
    if ( stat(full,info) == 0)
      if (S_ISREG(info->st_mode) && (info->st_mode & S_IXUSR)){
        *found = (char*)malloc(strlen(full+1));
        sprintf(*found,"%s",full);
        return 0;
      }
  }
  return -1;
}

//function to generate the prompt
char * makeprompt(char * prompt,char * pwd) {

  //use default prompt or set vars to make one
  if (!getenv("PS3")) { strcpy(prompt,"$ "); return prompt; }
  strcpy(prompt,"");
  char * prompv = getenv("PS3");
  char prompc;
  char buffer[2];

  //loop through each char of PS1 to build prompt
  for (int i=0; i<strlen(prompv); i++){
    prompc = prompv[i];
    
    //handle escape characters
    if (prompc == '\\' && i<strlen(prompv)-1){
      prompc = prompv[i+1];      

      switch (prompc){
        case 'u':
          strcat(prompt,DEF(getenv("USER"),""));
          break;
        case 'w':
          strcat(prompt,pwd);
          break;
        case 'n':
          strcat(prompt,"\n");
          break;
        case 'h':
          strcat(prompt,DEF(getenv("HOSTNAME"),""));
          break;
      }

    }
    //handle normal characters
    else if (i>0 && (char)prompv[i-1] != '\\'){
      buffer[0] = prompc;
      buffer[1] = '\0';
      strcat(prompt,buffer); 
    }
  }

  return prompt;

}

//void interrupter() { kill(0,2); }

int main(int argc, char**argv)
{

  //set the prompt
  char prompt[200];
  makeprompt(prompt,getenv("PWD"));

  //get and parse PATH
  char * path = DEF(getenv("MYPATH"), getenv("PATH"));
  char ** pathArray;
  int pathlen;

  if ( ( pathlen = makeargv ( path, ":", &pathArray ) ) == -1 ){
    fprintf ( stderr, "Failed to construct an argument array for PATH");
    return 1;
  }

  //create command variables
  char command[300],cwd[80];
  char * binpath;
  struct stat info;
  int pid, uid = getuid();
  char ** arglist;
  
  //reassign ctrl c
  //struct sigaction sigint;
  //sigint.sa_handler = &interrupter;
  //sigaction(2,&sigint,NULL);
  sigset_t	sigmask;
	sigaddset ( &sigmask, SIGINT );
	sigprocmask ( SIG_BLOCK, &sigmask, NULL );

  //loop for commands until ctrl-d
  while(!feof(stdin)){

    //get and parse the command
    printf("%s ",prompt);
    fgets(command,sizeof(command),stdin);
    strtok(command,"\n");
    makeargv (command, " ", &arglist);


    //change directory if permitted
    if (!strcmp(arglist[0],"cd")){
      //go home
      if (!arglist[1]){
        chdir(getenv("HOME"));
        makeprompt(prompt,getwd(cwd));
      }
      //go to target directory
      else {
        stat(arglist[1],&info);
        if ((S_ISDIR(info.st_mode)) &&
          ((info.st_mode & S_IROTH) && (uid != info.st_uid)) ||
          (info.st_mode & S_IRUSR) && (uid == info.st_uid)){
          chdir(arglist[1]);
          makeprompt(prompt,getwd(cwd));
        }
        else
          printf("You can't do that.\n");
      }
    }


    //exit commands
    else if ((!strcmp(arglist[0],"exit")) || (!strcmp(arglist[0],"logout")))
      return (0);

    //execute command
    else if (!searchPath(arglist[0],&binpath,&info,pathArray,pathlen)){
      pid = fork();
      if (pid == 0){
        sigprocmask ( SIG_UNBLOCK, &sigmask, NULL );
        execv(binpath,arglist);
      } else
        wait(0);
    } else if (strcmp(command,"\n")) printf("%s: command not found.\n",command);


  }

  return (0);
}


/*
 * A simple unix shell which support &,>,< and 
 *can set prompt and exit
 * 2015-6-29 by Tangyaping
 */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/wait.h>

#define TRUE 1
#define MAXLINE 100

static void sig_int(int);

int main()
{
	char *arg_list[10];
	int status;
	int counter = 0;
	int counter1 = 0;
	pid_t pid;
	char buf[100];
	int fdIn, fdOut;
	char *prompt;
	char *value = "->";    //initial  prompt
	int out_redi_flag = 0;   //some flags
	int in_redi_flag = 0;
	int bg_flag = 0;
	int setpromt = 0;   
	int out_pos,in_pos;

	if(signal(SIGINT,sig_int) == SIG_ERR )
			printf("signal error\n");

	while(TRUE)
	{
		setenv("PS1",value,1);   //set value to environment variable PS1
		prompt = getenv("PS1");
		printf("%s",prompt);
		
		if(!fgets(buf,MAXLINE,stdin))
		{
			return 0;
		}

		/* parse the command */
		arg_list[counter] = strtok(buf," \n");  //divide the command with space and \n
		counter = 0;
		while(arg_list[counter] != NULL)
		{
			counter++;
			arg_list[counter] = strtok(NULL," \n");
		}

		counter1 = 0;

		while(arg_list[counter1] != NULL)        // there is still string in the command
		{
			if(!strcmp(arg_list[counter1],"<"))   //there is input redirect in the command 
			{
				if(arg_list[counter1+1] != NULL)
				{
					in_redi_flag = 1;               
					in_pos = counter1;             //record the position for the child process to create new file 
				}
				else
					printf("NO input file specified");
				
				arg_list[counter1] = 0;          // set the positon of < to zero for execvp to execute
			}

			else if(!strcmp(arg_list[counter1],">"))       // there is output redirect 
			{
				if(arg_list[counter1+1] != NULL)
				{
					out_redi_flag = 1;
					out_pos =counter1;
				}
				else
					printf("No outfile specified");
				
				arg_list[counter1] = 0;
			}

			else if(!strcmp(arg_list[counter1],"&"))     //there is background 
			{
				bg_flag = 1;
				arg_list[counter1] = 0;
			}

			else if(!strcmp(arg_list[0],"exit") && !arg_list[1])    //there is only exit 
					exit(0);

			else if(!strcmp(arg_list[counter1],"PS1") && !strcmp(arg_list[counter1+1],"="))  //set value to PS1
			{
				if(arg_list[counter1+2] != NULL)
				{
					value = arg_list[counter1+2];
					setpromt = 1;
				}
				else
					printf("please input the prompt\n");
			}
		
			counter1++;
		}
       
		/* execute the command */

		if((pid = fork()) < 0)
		{
			printf("fork error\n");
			exit(0);
		}
		else if(pid == 0)             // in the child process
		{
			if(in_redi_flag == 1)     //  there is input redirect 
			{
				 fdIn = open(arg_list[in_pos+1],O_RDONLY);
			    dup2(fdIn,STDIN_FILENO);
				close(fdIn);
			}

			if(out_redi_flag == 1)   // there is output redirect 
			{
				fdOut = open(arg_list[out_pos+1],
		         O_CREAT | O_WRONLY |O_TRUNC, 0644);
			   dup2(fdOut,STDOUT_FILENO);
			   close(fdOut);

			}

			if(setpromt == 1)     // already set the promt 
				exit(0);	
	
			execvp(arg_list[0],arg_list);  // execute the command 
			printf("command error\n");
			exit(0);
		}

		else
		{
			if(bg_flag == 1)      //deal with the background process
	  			waitpid(pid,&status,WUNTRACED);
			else
				waitpid(pid,&status,0);
		}
		//set the flags to zero for the next loop
		out_redi_flag = 0;
		in_redi_flag = 0;
		out_pos = 0;
		in_pos = 0;
		counter1 = 0;
		bg_flag = 0;
		setpromt = 0;
	}
		return 0;
}

/* signal handler */
void sig_int(int signo)
{
	printf("interrupt\n");
	exit(0);
}

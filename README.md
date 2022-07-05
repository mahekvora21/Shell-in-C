# Shell In C

## Language used: C
## Operating System used: Linux Ubuntu
----------------------------------------------------------------------------------------------------------------------------------------------
### Execution
```

gcc 200101062_Assign02.c -lreadline
./a.out 200101062_Assign02.c
```


----------------------------------------------------------------------------------------------------------------------------------------------
### Features of the shell
----------------------------------------------------------------------------------------------------------------------------------------------
Envionment variables-> They can be accessed using printenv or echo, new variables can be declared, which is handled using setenv
```
USER
HOME
SHELL
TERM
PATH
LANG
PWD
```
----------------------------------------------------------------------------------------------------------------------------------------------
### Internal commands executed
```
cd                : has been implemented using chdir() function
history           : prints the contents of the history file, which includes all commands given till that time.
quit or exit or x : removes history file as required by flowchart.
echo              : All the cases including handling white spaces have been handled.
		     Global Environment variables have been handled successfully.
                    In case of Inverted commas, " ' ' " or ' " " ' or " " only or ' ' only or " " '' has been treated as valid.
                    if inverted Inverted commas are not closed it is invalid.Kindly do not give inputs like " " " ' ' "(as this comprises of both 		     cases together)
                    Kindly note that commands like echo $HOME have alse been executed, using getenv and parsing input for the $ sign.
printenv          : Lists all the global environment variables
setenv            : Can set a new value to an new variable. 
		     Note : Use this syntax to check setenv:
		     
		     VARIABLE=value
		     
		     The program detects the presence of = and creates a new variable, just as it happens in a normal linux terminal.
```

----------------------------------------------------------------------------------------------------------------------------------------------
### Linux Commands executed 
They have been implemented using the exec family of functions.
```
ls
cat
passwd
date
cal
more
less
clear
cp
mkdir
grep
man
sleep
pwd
env
touch
rm
rmdir
mv
script
apropos
find
which
shutdown
chmod
whereis
```
----------------------------------------------------------------------------------------------------------------------------------------------
### Redirection 
NOTE: All of these have been executed for external commands only.Thus they will not work for echo, cd, setenv, printenv, history.
		It works well for all external commands.

```
Single level piping eg. ls | grep D
Double level piping eg. ls | grep D | wc
Single level input redirection eg. cat <input1.txt
Single level output redirection eg. ls >output1.txt
Single level output redirection to append eg. ls >>output1.txt
```
----------------------------------------------------------------------------------------------------------------------------------------------
### Overview of the program
In the main function :
1. memory has been allocated for reading input
2. If input is not empty, it is added to history file as required
3. memory allocation to store the input after it was parsed by 
	a.space
	b.|
	c.<
	d.>
	e.>>
4. Execution of commands using an infinite while loop which will break if the command is invalid. If it terminates after a command/ does not generate 
   prompt, then please re execute the program.
5. First check is for environment variables. printenv, setenv, getenv have been executed.
6. Next, check is for cd and cd with specific directory
7. history prints contents of "/tmp/history.txt" on terminal
8. quit or x or exit will remove history file as required by flowchart and terminate the program.
9. echo has been executed with the specified limitations mentioned above.
9. Execution of simple linux commands(no pipes or redirection) is done by simple_commands
10. All the indicator variables have been made to check the existance of pipe or redirection and accordingly they have been executed
11. Piping upto two levels is successfully executed for external commands.
12. Simultaneous < and > has not been handled in this program.
----------------------------------------------------------------------------------------------------------------------------------------------




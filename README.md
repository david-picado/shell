# Linux based Shell 2020 - 2021

## Basic functions for the Shell
- author [-l | -n] : Prints the name and login of the creator
                     of the shell
- getpid : Prints the pid of the process executing the Shell
- getppid : Prints the pid of the shell's parent
- pwd : Prints the current shell working directory
- chdir [direct] : Changes the current directory of the shell given the new path ("direct")
- date : Just prints your date in the format dd/mm/yy
- time : Prints the current time in the format hh:mm:ss
- historic [-c | -N | -rN] : Show/clears the historic of commands executed by this shell. You can also clear this historic
by adding the argument -c

## File Systems (System V)
- Part I
    - create [-dir] name : Creates a file or directory in the file system. The parameter name represent the name of the file
    or directory. If -dir is given a directory is to be created, otherwise it is understood that an empty file will be created
    Finally if name is not given, the contents of the current directory will be listed.
    
    - delete [-rec] name1 name2 ... : Deletes files and directories.
        - If -rec is given and some of the names stand for a directory, it will try to delete that directory and ALL OF THE CONTENTS
        - If -rec is not given, a directory will only be deleted if it is empty.
        - rec with any type of file will delete them as usual.
        - If no name is given, the contents of the current working directory 

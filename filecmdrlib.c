/*
 * 130072007
 */

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "filecmdrlib.h"

#define MODE_S_LEN  11
#define TIME_S_LEN  17

/* 
 * is_user_exec helper function to test user execute permission for given 
 * file mode and owner uid and gid.
 * Uses getuid() to get the uid of the calling process and getgid() to get the 
 * gid of the calling process.
 * This function is not part of the filecmdrlib interface.
 */ 
bool is_user_exec(mode_t mode, uid_t ouid, gid_t ogid) {
    if (ouid == getuid())
        return mode & S_IXUSR;
   
    if (ogid == getgid())
        return mode & S_IXGRP;
    
    return mode & S_IXOTH;
}

int execfile(char *path) {
    // Prompt user for args
    printf("Enter any arguments to %s: ", path);

    // Declare arg string buffer
    char buffer[ARG_MAX + 1];

    // Read input to buffer and err check
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return -1;
    }

    // Set newline equal to string terminator and initialise buffer pointer
    buffer[strcspn(buffer, "\n")] = '\0';
    char *bp = buffer;
    
    // Declare arg token arr, set first element to path and increment i
    char *tokens[ARG_MAX/2];

    int i = 0;
    tokens[i++] = path;

    // Place whitespace-delimited args into arr until end of buffer found
    while((tokens[i] = strsep(&bp, " \t")) != NULL) {
        i++;
    }

    // Exec file at path with arg tokens, return to indicate err
    return execv(path, tokens);
}

int listdir(char *path) {
    // Declare required variables
    struct dirent **namelist;
    int n;

    // Assign n, run scandir and err check
    if ((n = scandir(path, &namelist, NULL, alphasort)) == -1 || 
    	chdir(path) == -1) {
        return -1;
    }

    // For each entry in namelist array
    for (int i = 0; i < n; i++) {
    	// Call printfinf on directory name and free mem
        if (printfinf(namelist[i]->d_name) == -1) {
        	return -1;
        } free(namelist[i]);
    }

    // Free mem and return 
    free(namelist);
    return 0;
}

int listfile(char *path) {
    // Declare required variables
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Initialise fp and error check
    if ((fp = fopen(path, "r")) == NULL) {
        return -1;
    }

    // Retrieve each line and print to stdout until EOF, add newline
    while ((read = getline(&line, &len, fp)) != -1) {
    	// Err check if less chars written than read
        if (fwrite(line, sizeof(char), read, stdout) < read) {
        	return -1;
        }
    } putchar('\n');

    // Free memory, close stream and return
    free(line);
    fclose(fp);

    return 0;
}

char *mode2str(mode_t mode, uid_t ouid, gid_t ogid) {
    // Error checking
    if(mode < MODE_MIN || mode > MODE_MAX) {
        errno = EDOM;
        return NULL;
    }

    // Declare mode_s
    char *mode_s; 

    // Initialise mode_s, allocate memory and check for err
    if ((mode_s = calloc(MODE_S_LEN, sizeof(char))) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    // Set file type flag
    if ((mode & S_IFMT) == S_IFDIR) {
        mode_s[0] = 'd';
    }

    else if ((mode & S_IFMT) == S_IFLNK) {
        mode_s[0] = 'l';
    }

    else if ((mode & S_IFMT) == S_IFIFO ||
        (mode & S_IFMT) == S_IFCHR ||
        (mode & S_IFMT) == S_IFBLK) {
        mode_s[0] = 'o';
    }

    else if (is_user_exec(mode, ouid, ogid)) {
        mode_s[0] = 'e';
    }

    else if ((mode & S_IFMT) == S_IFREG) {
        mode_s[0] = 'f';
    }

    // Set permissions flags
    mode_s[1] = (mode & S_IRUSR ? 'r' : '-');
    mode_s[2] = (mode & S_IWUSR ? 'w' : '-');
    mode_s[3] = (mode & S_IXUSR ? 'x' : '-');
    mode_s[4] = (mode & S_IRGRP ? 'r' : '-');
    mode_s[5] = (mode & S_IWGRP ? 'w' : '-');
    mode_s[6] = (mode & S_IXGRP ? 'x' : '-');
    mode_s[7] = (mode & S_IROTH ? 'r' : '-');
    mode_s[8] = (mode & S_IWOTH ? 'w' : '-');
    mode_s[9] = (mode & S_IXOTH ? 'x' : '-');

    return mode_s;
}

int printfinf(char *path) {
	// Determine usable file path
	if (path == NULL || path == '\0') {
		return FTYPE_ERR;
	}

    // Declare required structs
    struct stat sb;
    struct passwd *pwd;
    
    // Assign information to struct, if assignment fails, method terminates
    if (lstat(path, &sb) == -1) {
        return FTYPE_ERR;
    }

    // Retrieve password file and err check
    if ((pwd = getpwuid(sb.st_uid)) == NULL) {
        return FTYPE_ERR;
    }

    // Allocate string representations of mode and time
    char *mode_s;
    char *time_s;

    // Err check
    if ((mode_s = mode2str(sb.st_mode, sb.st_uid, sb.st_gid)) == NULL ||
    	(time_s = time2str(sb.st_mtime)) == NULL) {
    	return FTYPE_ERR;
    }

    // Assign formatted output to string
    printf("%s %-8s %12lld %s %s\n",
    	mode_s,
    	pwd->pw_name,
    	sb.st_size,
    	time_s,
    	path);

    // Free dynamically allocated memory
    free(mode_s);
    free(time_s);

    // Determine file type and return
    if ((sb.st_mode & S_IFMT) == S_IFDIR) {
        return FTYPE_DIR;
    }

    if ((sb.st_mode & S_IFMT) == S_IFLNK) {
        return FTYPE_LNK;
    }

    if ((sb.st_mode & S_IFMT) == S_IFIFO ||
        (sb.st_mode & S_IFMT) == S_IFCHR ||
        (sb.st_mode & S_IFMT) == S_IFBLK) {
        return FTYPE_OTH;
    }

    if (is_user_exec(sb.st_mode, sb.st_uid, sb.st_gid)) {
        return FTYPE_EXE;
    }

    if ((sb.st_mode & S_IFMT) == S_IFREG) {
        return FTYPE_REG;
    }

    // Error if none of the above if statements evaluate to true
    return FTYPE_ERR;
}

char *time2str(time_t time) {
    // Declare format string and variables
    static char *str_fmt = "%02d/%02d/%4d %02d:%02d";

    struct tm *tb;
    char *time_s;

    // Assign struct and error check
    if ((tb = localtime(&time)) == NULL) {
        return NULL;
    }

    // Assign memory and err check
    if ((time_s = calloc(TIME_S_LEN, sizeof(char))) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    // Print given variables to time_s using given format
    sprintf(time_s, str_fmt, 
        tb->tm_mday, 
        tb->tm_mon + 1, 
        tb->tm_year + 1900, 
        tb->tm_hour, 
        tb->tm_min);

    return time_s;
}

int useraction(int ftype, char *path) {
    // If not a directory, executable or regular file, return
    if (ftype > FTYPE_REG) {
        return 0;
    }

    // Initialise array of prompt options
    static const char *action_prompt[] = {
        "Do you want to list the directory %s (y/n): ",
        "Do you want to execute %s (y/n): ",
        "Do you want to list the file %s (y/n): " 
    };

    // Print relevant prompt to console
    printf(action_prompt[ftype], path);

    // Retrieve user input, clear remaining characters in stdin
    int input = getchar();
    
    if (fseek(stdin, 0, SEEK_END) == -1) {
    	return -1;
    }

    // If user action required, enter switch
    if (input == 'y' || input == 'Y') {
        // Switch to execute relevant method and return
        switch(ftype) {
            case FTYPE_DIR:
            return listdir(path);

            case FTYPE_EXE:
            return execfile(path);

            case FTYPE_REG:
            return listfile(path);
        }
    }

    // Return if no user action required
    return 0;
}
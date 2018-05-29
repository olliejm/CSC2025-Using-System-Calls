#ifndef FILECMDRLIB_H  /* FILECMDRLIB_H not defined */
#define FILECMDRLIB_H  /* define it to prevent multiple inclusion */
#include <sys/types.h>

#define FTYPE_ERR   -1      /* flag for error return from printfinf */
#define FTYPE_DIR   0       /* flag for directory ('d' - 'd') */
#define FTYPE_EXE   1       /* flag for user executable regular file
                             * ('e' - 'd')
                             */
#define FTYPE_REG   2       /* flag for regular file ('f' - 'd')*/
#define FTYPE_LNK   8       /* flag for symbolic link ('l' - 'd') */
#define FTYPE_OTH   11      /* flag for other file ('o' - 'd') */

#define MODE_MIN    0010000 /* minimum file mode value */
#define MODE_MAX    0167777 /* maximum file mode value */

/*
 * Documentation of each function in filecmdrlib follows in comments to 
 * the function declarations. Specification of error values is incomplete 
 * because they are dependent on the choice of system calls and C library
 * functions that are used to implement the filecmdrlib functions.
 */

/* 
 * Function name: execfile
 * 
 * Description: 
 * Execute the file/program specified by the given path (replacing the 
 * currently executing process). If the file is executed, this function does
 * not return to the calling process.
 * An optional extension prompts for the user to provide a line of 
 * command arguments before executing the program.
 * 
 * Parameters:
 * path - the path name (absolute or relative) of the file/program to execute.
 *
 * Return values:
 * If the execfile function returns, there will have been a failure to execute
 * the specified file, the return value will be -1 and the global variable 
 * errno will be set to indicate the error.
 * If execution of the file succeeds, execfile does not return. However, there
 * may be errors resulting from the separate execution of the file.
 * 
 * Errors:
 * errno values will be set by system call(s) used for exec'ing a file.
 */
int execfile(char *path);

/* 
 * Function name: listdir
 * 
 * Description: 
 * List the content of the directory specified by the given path. For each
 * file/directory in the directory, print to stdout the same output as the
 * output from the printfinf function.
 * 
 * Parameters:
 * path - the path name (absolute or relative) of the directory to list.
 *
 * Return values:
 * 0 on successful completion.
 * -1 if an error occurs (e.g. the directory cannot be opened for 
 * reading or malloc cannot allocate enough memory to hold the directory data
 * structures). The global variable errno will be set to indicate the error.
 *
 * Errors:
 * errno values will be set by system calls for scanning and changing 
 * directories.
 */
int listdir(char *path);

/* 
 * Function name: listfile
 *
 * Description:
 * List the content of the file specified by the given path by printing 
 * each line of the file to stdout.
 * 
 * Parameters:
 * path - the path name (absolute or relative) of the file to list/print.
 * 
 * Return values:
 * 0 on successful completion.
 * -1 if an error occurs and the global variable errno will be set to 
 * indicate the error.
 *
 * Errors:
 * errno values will be set by system calls for opening, reading, writing
 * and closing a file or stream.
 */
int listfile(char *path);

/* 
 * Function name: mode2str
 *
 * Description:
 * Convert the given file mode bits to a string representation that is similar
 * in form to the output from the Unix ls command. That is, 3 triples of 'rwx' 
 * characters that give the owner, group and other permissions.
 * A '-' character in place of a 'r', 'w' or 'x' character denotes denial
 * of the relevant permission for the relevant role (user, group or other).
 * The permission characters are preceded by one of the following file type
 * characters:
 * 'd' for directory
 * 'e' for a regular file that is user executable (calling user has execute
 *     permission as user, group or other - the is_user_exec helper 
 *     function in filecmdrlib.c provides a test for this)
 * 'f' for regular file 
 * 'l' for symbolic link 
 * 'o' for any other type of file
 * The mode string is always 11 characters long (1 for the type, 9 for the
 * permissions and 1 for the string terminator).
 * 
 * Examples mode strings:
 * drwxr-xr-x
 *      denotes a directory (d) with owner read (r), write (w) and
 *      execute (x) permision (rwx), group read and execute permission (r-x)
 *      and other read and execute permission (r-x).
 * frw-r-----
 *      denotes a regular file with owner read and write permission (rw-), 
 *      group read permission (r--) and no permissions for other (---)
 *
 * The string returned from this function does not include any 
 * representation of the set user id, set group id or save swapped text bits 
 * of the file mode.
 *
 * See man pages for stat and chmod for definition of the mode bits and 
 * valid values. 
 * 
 * Storage for the string is allocated dynamically by this function. If there
 * is insufficient memory for the allocation, the function returns NULL. 
 * It is the responsibility of the user of the mode2str function to release
 * the memory allocated for the returned string.
 *
 * Note
 * 
 * Parameters:
 * mode - the file mode to convert to a string.
 * ouid - the user id (uid) of the file's owner
 * ogid - the group id (gid) of the file's owner
 * 
 * Return values:
 * On successful completion, a dynamically allocated string representation of
 * the given mode.
 * NULL if an error occurs and the global variable errno will be set to 
 * indicate the error.
 * 
 * Errors:
 * EDOM if mode is out of the valid range for a file mode
 * ENOMEM if there is insufficient memory to allocate the mode string
 */
char *mode2str(mode_t mode, uid_t ouid, gid_t ogid);

/* 
 * Function name: printfinf
 *
 * Description:
 * Print/display to stdout a single line of information about the user 
 * specified file. The line of information has the following fields 
 * separated by spaces:
 * - file mode string (as converted from mode by mode2str)
 * - file owner's user name (obtained from the password file uid to user name
 *   mapping)
 * - size in bytes
 * - last modification date (in dd/mm/yyyy hh:mm format, as converted from 
 *   time by time2str)
 * - path name as specified by the user
 *
 * drwxr-xr-x root         1408 08/09/2016 20:06 /
 * is a typical line of print out for the '/' directory on Minix. The 
 * additional spaces between "root" (the owner's user name) and 1408 (the size
 * in bytes) is to allow for larger file sizes. The size is right aligned.
 *
 * The line is terminated by the new line character.
 *
 * This function returns one of the file type flags defined in this header 
 * file or FTYPE_ERR (-1) in the case of error.
 * 
 * Parameters:
 * path - the path name (absolute or relative) of the file or directory to 
 * print information about.
 * 
 * Return values:
 * FTYPE_DIR if path is a directory
 * FTYPE_EXE if path is a user executable regular file
 * FTYPE_REG if path is a regular file (not user executable)
 * FTYPE_LNK if path is a symbolic link
 * FTYPE_OTH if path is any other type of file 
 * -1 (FTYPE_ERR) if an error occurs and the global variable errno will be set
 * to indicate the error.
 *
 * Errors:
 * errno values will be set by system calls used for obtaining file
 * information etc.
 */
int printfinf(char *path);

/* 
 * Function name: time2str
 *
 * Description:
 * Convert the given time since the epoch to a string representation of the
 * following form:
 * dd/mm/yyyy hh:mm
 * where:
 * dd is a 2 digit day of the month from 01 to 31 (28, 29 or 30 for relevant
 * months)
 * mm is a 2 digit month of the year from 01 to 12
 * yyyy is a 4 digit year
 * hh is a 2 digit hour from 00 to 23
 * mm is a 2 digit minute from 00 to 59
 *
 * E.g. the string "08/09/2016 20:06" is 20:06 on 8 September 2016.
 * 
 * The time string is always 17 characters long (16 characters for the date
 * information and 1 character for the string terminator).
 *
 * Storage for the string is allocated dynamically by this function. If there
 * is insufficient memory for the allocation, the function returns NULL. 
 * It is the responsibility of the user of the time2str function to release
 * the memory allocated for the returned string.
 * 
 * Parameters:
 * time - the time in seconds since the epoch (00:00:00 UTC, Jan 1 1970) to
 * convert to a string.
 *
 * Return values:
 * On successful completion, a dynamically allocated string representation of
 * the given time.
 * NULL if an error occurs and the global variable errno will be set to 
 * indicate the error.
 * 
 * Errors:
 * ENOMEM if there is insufficient memory to allocate the time string
 * EINVAL, EOVERFLOW and memory allocation errors from underling 
 * library calls (see man pages for ctime and localtime)
 */
char *time2str(time_t time);

/* 
 * Function name: useraction
 *
 * Description:
 * Give the user the option of executing an appropriate action on the    
 * file/directory specified by the given file type and path.
 * - For a directory, the user is prompted to list the directory. If they
 *   respond 'y' or 'Y' to the prompt, the content of the directory will be 
 *   listed (printed to stdout) and then the function will return with a 
 *   status indicating success or failure. The function returns immediately if
 *   the user enters anything other than 'y' or 'Y'.
 * - For a user executable regular file, the user is prompted to execute the
 *   file. If they respond 'y' or 'Y' to the prompt, the file is executed.
 *   If execution is successful, the function does not return. If execution
 *   unsuccessful, the function will return with an error status.
 *   The function returns immediately if the user enters anything other than 
 *   'y' or 'Y'.
 * - For a regular file that is not user executable, the user is prompted
 *   to list the file. If they respond 'y' or 'Y' to the prompt, the content 
 *   of the file will be listed (printed to stdout) and then the function will
 *   return with a status indicating success or failure. The function returns
 *   immediately if the user enters anything other than 'y' or 'Y'.
 * - For any other type of file, the function has no effect and returns 
 *   immediately with a success status.
 *
 * This function uses listdir to list a directory, listfile to list a file and
 * execfile to execute a file. The return status and errors from this function
 * are, in part, determined by those functions.
 *
 * Parameters:
 * ftype - the FTYPE flag that indicates the type of the file (e.g. FTYPE_DIR
 * for directory, FTYPE_EXE for user executable regular file and FTYPE_REG for
 * regular file)
 * path - the path (absolute or relative) of the file/directory
 * 
 * Return values:
 * 0 on successful completion. Successful completion includes: immediate
 * return because the specified path is not a directory or regular file or 
 * because the user responds with anything other than 'y' or 'Y' when prompted, 
 * and success of the underlying action: listdir or listfile. See execfile for
 * the status after execution of a file.
 * -1 in the event of an error and the global variable errno will be set to 
 * indicate the error. Errors are the result of an error in the 
 * corresponding action function (listdir, listfile or execfile).
 * 
 * Errors:
 * See listdir, listfile and execfile for details. 
 */
int useraction(int ftype, char *path);

#endif /* end of not defined FILECMDRLIB_H */

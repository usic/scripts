/*
* usic_usersettle <-m|-r> -p path [ -u uid] [ -g gid]
* sakib@meta.ua
* USIC,2008
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glob.h>
#include <stdlib.h>
#define MAX_PATH 4096
// #define NDEBUG

int cleanup_userhome(char*);

int main(int argc,char* argv[]){
char userpath[MAX_PATH];
bzero(userpath, sizeof(userpath)-1);
uid_t uid=0;
gid_t gid=0;
pid_t child_pid = 0;
int opt;
char action = 0;
while ( (opt=getopt(argc,argv,"mrp:u:g:") ) != EOF){
	switch (opt){
		case 'm':
			if (action != 0 ) _exit(EXIT_FAILURE);
			action = 'm';
			break;
		case 'r':
			if (action != 0 ) _exit(EXIT_FAILURE);
			action = 'r';
			break;
		case 'p':
			if ( strlen(optarg) >= MAX_PATH ) _exit(EXIT_FAILURE);
			bzero(userpath, sizeof(userpath)-1);
			strncpy(userpath,optarg,sizeof(userpath) - 1);
			break;
		case 'u':
			if ( ! ( action == 0 || action == 'm') ) _exit(EXIT_FAILURE);
			uid = atoi(optarg);
			/*#ifndef NDEBUG
				fprintf(stderr, "uid --> %d\n", uid);
			#endif*/
			break;
		case 'g':
			if ( ! ( action == 0 || action == 'm') ) _exit(EXIT_FAILURE);
			gid = atoi(optarg);
			/* #ifndef NDEBUG
				fprintf(stderr, "gid --> %d\n", gid);
			#endif*/
			break;
		case '?':
			fprintf(stderr,"USAGE: usic_usersettle <-m|-r> -p path [ -u uid] [ -g gid ]\n");
			_exit(EXIT_FAILURE);
		}
	}
if (strlen(userpath) == 0 ) _exit(EXIT_FAILURE);
setuid(0);
setgid(0);
switch (action){
	case 'm':
		if ( mkdir(userpath,0700) != 0){
			int errsv = errno;
			perror("mkdir failed");
			_exit(errsv);
			}
		if ( chown(userpath,uid,gid) != 0){
			int errsv = errno;
			perror("chown failed");
			_exit(errsv);
			}
		break;
	case 'r':
		child_pid = fork();
		if (child_pid){
			int status=0;
			waitpid(child_pid,&status,0);
			if (WEXITSTATUS(status)){
				#ifndef NDEBUG 
					fprintf(stderr,"cleanup_userhome() failed with status %d\n", status);
				#endif
				_exit(status);
				}
			if (rmdir(userpath) !=0 ){
				#ifndef NDEBUG
					fprintf(stderr,"rmdir() %s failed: ", userpath);
					perror("");
				#endif
				_exit(EXIT_FAILURE);
				}
		} else {
			struct stat sb;
			stat(userpath,&sb);
			setuid(sb.st_uid);
			setgid(sb.st_gid);
			#ifndef NDEBUG
				fprintf(stderr,"cleaning up %s under %d:%d\n",userpath,getuid(),getgid());
			#endif
			if (cleanup_userhome(userpath) != 0) _exit(EXIT_FAILURE);
			_exit(EXIT_SUCCESS);
			}
		break;
	default:
		_exit(EXIT_FAILURE);
	}
return EXIT_SUCCESS;
}

int cleanup_userhome(char* path){
	int in=0,status=EXIT_SUCCESS;
	struct stat sb;
	size_t path_length = strlen(path);
	char* gl_path = (char*)malloc( path_length + 4 );

	glob_t globent;
	globent.gl_offs = 0;

	bzero(gl_path, path_length + 4);
	strncpy(gl_path, path, path_length);
	gl_path[path_length] = '/';
	gl_path[path_length+1] = '*';
	glob(gl_path, 0, NULL, &globent);

	gl_path[path_length+1] = '.';
	gl_path[path_length+2] = '*';

	glob(gl_path, GLOB_APPEND,NULL,&globent);
	for (; in<globent.gl_pathc;++in){
		if ( lstat(globent.gl_pathv[in], &sb) != 0 ){
			perror("cleanup_userhome()");
			_exit(errno);
			}
		if ( strstr(globent.gl_pathv[in], "/.") || strstr(globent.gl_pathv[in], "/..") ) continue;
		if ( ( (sb.st_mode & S_IFMT) == S_IFDIR ) && ( (sb.st_mode & S_IFMT) != S_IFLNK) ){
			if ( cleanup_userhome(globent.gl_pathv[in]) != EXIT_SUCCESS ){
				status = EXIT_FAILURE;
				break;
				}
			if (rmdir(globent.gl_pathv[in]) != 0){
				#ifndef NDEBUG
					fprintf(stderr,"cleanup_userhome(): rmdir %s failure: ", globent.gl_pathv[in]);
					perror("");
				#endif	
				status = EXIT_FAILURE;
				break;
				}
		} else {
			if (unlink(globent.gl_pathv[in]) != 0){
				#ifndef NDEBUG
					fprintf(stderr,"cleanup_userhome(): unlink %s failure: ", globent.gl_pathv[in]);
					perror("");
				#endif
				_exit(errno);
				}
			}
		}
	free(gl_path);
	return status;
	}

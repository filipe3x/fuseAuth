/*
 * FUSE: Filesystem in Userspace
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 *
 * these files must be compiled in '/root/fuseAuth' folder by 'root'
 */

#define INSTALL_PATH /root/fuseAuth

unsigned create_token(char* request_id, int* size)
{
	char command[200];
	const char* file_code = "/root/fuseAuth/get_code";
	mkfifo(file_code, 0640);
	const char* file_request_id = "/root/fuseAuth/get_request_id";
	mkfifo(file_request_id, 0640);

	uid_t user_id = fuse_get_context()->uid;
	sprintf(command, "ruby /root/fuseAuth/ruby_script.rb send %u", (unsigned) user_id);

	// send sms
	pid_t pid = fork();
	if(pid == 0)
	{
		// Substituir o filho pelo programa que queremos executar
		system(command);
		exit(0);
	} else {
	// if sms sent successfully, get returned 'request_id'
	if(1) 
	{
		char buf[100];
		int fd_request_id = open(file_request_id, O_RDONLY);

		if (fd_request_id == -1)
			return -errno;

		int i = 0; int count;
		while ((count = read(fd_request_id, &buf[i], 1)) > 0)
			i++;

		buf[i] = '\0';
		close(fd_request_id);

		strncpy(request_id, buf, i);
		*size = i;

		return 0;
	} 
	else 
	{
		return 1;
	}
	}

}

unsigned validate_code(char* request_id)
{
	char buf[100];
	char command[200];
	const char* file = "/root/fuseAuth/get_code";

	int fd = open(file, O_RDONLY);

	if (fd == -1)
		return -errno;

	int i = 0; int count;
	while ((count = read(fd, &buf[i], 1)) > 0)
		i++;

	buf[i] = '\0';
	close(fd);

	sprintf(command, "ruby /root/fuseAuth/ruby_script.rb verify %s %s", buf, request_id);
	int returned_code = system(command);

	//if(strncmp(buf, "123", i) == 0) //code is correct
	if(returned_code == 0) //code is correct
	{
		return 0;
	}
	else //wrong code
	{
		return 1;
	}	
	
}

/*
 * Creates files on the underlying file system in response to a FUSE_MKNOD
 * operation
 */
static int mknod_wrapper(int dirfd, const char *path, const char *link,
	int mode, dev_t rdev)
{
	int res;

	if (S_ISREG(mode)) {
		res = openat(dirfd, path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISDIR(mode)) {
		res = mkdirat(dirfd, path, mode);
	} else if (S_ISLNK(mode) && link != NULL) {
		res = symlinkat(link, dirfd, path);
	} else if (S_ISFIFO(mode)) {
		res = mkfifoat(dirfd, path, mode);
#ifdef __FreeBSD__
	} else if (S_ISSOCK(mode)) {
		struct sockaddr_un su;
		int fd;

		if (strlen(path) >= sizeof(su.sun_path)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd >= 0) {
			/*
			 * We must bind the socket to the underlying file
			 * system to create the socket file, even though
			 * we'll never listen on this socket.
			 */
			su.sun_family = AF_UNIX;
			strncpy(su.sun_path, path, sizeof(su.sun_path));
			res = bindat(dirfd, fd, (struct sockaddr*)&su,
				sizeof(su));
			if (res == 0)
				close(fd);
		} else {
			res = -1;
		}
#endif
	} else {
		res = mknodat(dirfd, path, mode, rdev);
	}

	return res;
}

#include "include/tnfs.h"

/* 
 * TNFS_MAX_RESULTS must be in contrast with the total tnfs_buffer size and max_path length !!!
 * for example is TNFS_MAX_PATH_LEN is 256 and TNFS_MAX_RESULTS = 50 then TNFS_BUFFERSIZE must be: 10 bytes + (13 bytes + 256) * 50 = 13460 bytes 
 */
const uint8_t TNFS_MAX_RESULTS = 58;
const char    TNFS_PROTOCOL_VERSION[] = {0x02, 0x01};

/* tnfs global variables */
char 	 tnfs_buffer[TNFS_BUFFERSIZE];	// send and receive buffer
uint16_t tnfs_session_id = 0;		// stores current session id received from the server
uint8_t  tnfs_request_id = 0;		// request id increases each new request


/* sends the allready buffered data to the server and waits for a response */
int tnfs_sendReceive(int length)
{
   int retry = 0;
   int rlength;
   
   /* if we do not get an response from the server we will send the command again for several times */
   do {
    	netw_send(tnfs_buffer, length);
#ifdef DEBUG
    	printf("sent: ");
    	for(int i = 0 ; i < length ; i++) {
    		printf("%hhX ", tnfs_buffer[i]);
    	}
    	printf("\n");
#endif
    	rlength = netw_receive(tnfs_buffer, TNFS_BUFFERSIZE);
   	retry++;
   }while(rlength == NETW_ERR_TIMEOUT && retry < TNFS_SEND_RETRIES);
   
   /* if to many retries we give up */
   if(retry == TNFS_SEND_RETRIES) {
#ifdef DEBUG
   	printf("Server did not respond, transfer aborted!\n\n");
#endif
   	tnfs_buffer[4] = TNFS_EPROTO; // set the error code in the 5th byte like the server does normally
   	return TNFS_EPROTO * -1; // -27 Protocol error
   }
    
#ifdef DEBUG
    printf("recv: ");
    for(int i = 0 ; i < rlength ; i++) {
    	printf("%hhX ", tnfs_buffer[i]);
    }
    printf("\n");
#endif
    
    /* check for errors returned by server */
    if(tnfs_buffer[4] != 0x00 && tnfs_buffer[4] != 0x21) {
#ifdef DEBUG
    	printf("Server returned error code: %hhX\n\n", tnfs_buffer[4]);
#endif
	return tnfs_buffer[4] * -1; // negative number representing the error code
    }
    
#ifdef DEBUG
    printf("\n");
#endif

    return rlength;
}

/* buffers a new command header */
void tnfs_prepareCommand(uint8_t cmd)
{
    memset(tnfs_buffer, 0, sizeof(tnfs_buffer));
    memcpy(&tnfs_buffer[0], &tnfs_session_id, 2);
    tnfs_buffer[2] = tnfs_request_id++;
    tnfs_buffer[3] = cmd;
}

/* Establish a new session */
int tnfs_mount(char* dir, char* username, char* password)
{
    int length = 6;
    int retry_time;
    size_t result;
    
    tnfs_prepareCommand(0x00);
    memcpy(&tnfs_buffer[4], TNFS_PROTOCOL_VERSION, 2);
    strcpy(&tnfs_buffer[length], dir);
    length += strlen(dir)+1;
    strcpy(&tnfs_buffer[length], username);
    length += strlen(username)+1;
    strcpy(&tnfs_buffer[length], password);
    length += strlen(password)+1;
    
    length = tnfs_sendReceive(length);
    if(tnfs_buffer[4] == 0x00) {
    	memcpy(&tnfs_session_id, &tnfs_buffer[0], 2); // the session id is needed for all other requests
    	memcpy(&retry_time, &tnfs_buffer[7], 2); // the minimal retry time determined by the server
	setTimeoutTime(retry_time); // sets the timeout for waiting on the server response, see netw.c

#ifdef DEBUG
    	printf("session id: %d \n", tnfs_session_id);
    	printf("server minimal retry time: %d \n\n", retry_time);
#endif
    }

    return tnfs_buffer[4] * -1; // return code
}

/* Ends the session */
int tnfs_umount()
{
    tnfs_prepareCommand(0x01);
    tnfs_sendReceive(4);

    return tnfs_buffer[4] * -1; // return code
}

/* Open a directory */
int tnfs_opendir(char* path)
{
    int length = 4;

    tnfs_prepareCommand(0x10);
    strcpy(&tnfs_buffer[length], path);
    length += strlen(path)+1;
    
    length = tnfs_sendReceive(length);
    if(length == 6 && tnfs_buffer[4] == 0x00)
    	return tnfs_buffer[5]; // tnfs file handle
    
    return tnfs_buffer[4] * -1; // return code
}

/* reads one entry from the open directory */
int tnfs_readdir(char handle, char* dest)
{
    int length = 5;

    tnfs_prepareCommand(0x11);
    tnfs_buffer[4] = handle;
    
    length = tnfs_sendReceive(length);
    if(tnfs_buffer[4] == 0x00) {
    	strcpy(dest, &tnfs_buffer[5]);
    }
    
    return tnfs_buffer[4] * -1; // return code
}

/* Open a directory (with a lot of options) */
int tnfs_opendirx(char* path, char* pattern, uint8_t diropts, uint8_t sortopts, struct dirx_data* data)
{
    int length = 8;

    tnfs_prepareCommand(0x17);
    tnfs_buffer[4] = diropts;			// directory options
    tnfs_buffer[5] = sortopts;			// sort options
    // leave tnfs_buffer[6] and [7] to zero  because we want the total files found
    strcpy(&tnfs_buffer[length], pattern);		// search pattern
    length += strlen(pattern)+1;
    strcpy(&tnfs_buffer[length], path);		// directory path
    length += strlen(path)+1;
    
    length = tnfs_sendReceive(length);
    
    memset(data, 0, sizeof(struct dirx_data)); // set whole structure to zeros
    if(length == 8 && tnfs_buffer[4] == 0x00) {
    	data->handle = tnfs_buffer[5];
    	memcpy(&data->entries, &tnfs_buffer[6], 2); // copy the number of matching directory entries found in odirx.entries.
    }
    
    return tnfs_buffer[4] * -1; // Return code
}

/* Closes a directory */
int tnfs_closedir(char handle)
{
    int length = 5;

    tnfs_prepareCommand(0x12);
    tnfs_buffer[4] = handle;
    
    tnfs_sendReceive(length);
    
    return tnfs_buffer[4] * -1;
}

/* fills the buffer with multiple entries from the open directory with extra stat information for each entry */
int tnfs_readdirx(struct dirx_data* data)
{
    int length = 6;

    tnfs_prepareCommand(0x18);
    tnfs_buffer[4] = data->handle;
    tnfs_buffer[5] = TNFS_MAX_RESULTS;
    
    length = tnfs_sendReceive(length);
    
    if(length > 8 && tnfs_buffer[4] == 0x00) {
    	data->count  = tnfs_buffer[5];
    	data->status = tnfs_buffer[6];
    	memcpy(&data->dirpos, &tnfs_buffer[7], 2); // copy the position of first entry as given by TELLDIR
    }
    
    return tnfs_buffer[4] * -1;
}

/* reads one entry from the open directory with extra stat information */
int tnfs_nextdirx(struct dirx_data* data, struct dirx_item* xitem)
{
    int code;
    
    if(data->entry >= data->count) {
    	if(data->status == TNFS_DIRSTATUS_EOF) {
    	    return TNFS_EOF;
    	}
	code = tnfs_readdirx(data);
	if(code != 0) {
	    return code * -1; // error
	}
    	data->needle = 9;
    	data->entry = 0;
    }
    /* fill dirx_item structure */
    xitem->flags = tnfs_buffer[data->needle];
    memcpy(&xitem->size, &tnfs_buffer[data->needle + 1], 4); 
    memcpy(&xitem->modified, &tnfs_buffer[data->needle + 5], 4); 
    memcpy(&xitem->created, &tnfs_buffer[data->needle + 9], 4); 
    xitem->name = &tnfs_buffer[data->needle + 13];
    
    /* increase counters */
    data->needle += strlen(xitem->name) + 14;
    data->entry++;
    
    return 0;
}

/* Returns the entry position within current directory results */
int tnfs_telldir(char handle, uint32_t* position) 
{
    int length = 5;

    tnfs_prepareCommand(0x15);
    tnfs_buffer[4] = handle;
    
    length = tnfs_sendReceive(length);
    
    if(length == 9 && tnfs_buffer[4] == 0x00) {
    	memcpy(position, &tnfs_buffer[5], 4);
    }

    return tnfs_buffer[4];
}

/* Moves current directory results position to new value */
int tnfs_seekdir(char handle, uint32_t position) 
{
    int length = 9;

    tnfs_prepareCommand(0x16);
    tnfs_buffer[4] = handle;
    memcpy(&tnfs_buffer[5], &position, 4);
    
    tnfs_sendReceive(length);
    
    return tnfs_buffer[4];
}

/* Make a new directory */
int tnfs_mkdir(char* dir)
{
    int length = 4;

    tnfs_prepareCommand(0x13);
    strcpy(&tnfs_buffer[length], dir);
    length += strlen(dir)+1;
    
    tnfs_sendReceive(length);

    return tnfs_buffer[4];
}

/* Deletes a empty directory */
int tnfs_rmdir(char* dir)
{
    int length = 4;

    tnfs_prepareCommand(0x14);
    strcpy(&tnfs_buffer[length], dir);
    length += strlen(dir)+1;
    
    tnfs_sendReceive(length);

    return tnfs_buffer[4];
}

/* Open a file */
int tnfs_open(char* filename, uint16_t flags, uint16_t mode)
{
    int length = 8;

    tnfs_prepareCommand(0x29);
    memcpy(&tnfs_buffer[4], &flags, 2);
    memcpy(&tnfs_buffer[6], &mode, 2);
    strcpy(&tnfs_buffer[length], filename);
    length += strlen(filename)+1;

    length = tnfs_sendReceive(length);
    if(tnfs_buffer[4] != 0x00)
    	return tnfs_buffer[4] * -1;
    
    return tnfs_buffer[5]; // filehandle
}

/* read data from a file */
int tnfs_read(char* data, uint8_t handle, uint16_t maxlen)
{
    int length = 7;

    tnfs_prepareCommand(0x21);
    tnfs_buffer[4] = handle;
    memcpy(&tnfs_buffer[5], &maxlen, 2);

    length = tnfs_sendReceive(length);
    
    if(tnfs_buffer[4] != 0x00) {
        return tnfs_buffer[4] * -1; // return code
    }
    
    memcpy(&maxlen, &tnfs_buffer[5], 2);
    memcpy(data, &tnfs_buffer[7], maxlen);
    
    return maxlen; // actual length of data
}

/* write data to a file */
int tnfs_write(char* data, uint8_t handle, uint16_t maxlen)
{
    int length = 7;

    tnfs_prepareCommand(0x22);
    tnfs_buffer[4] = handle;
    memcpy(&tnfs_buffer[5], &maxlen, 2);
    memcpy(&tnfs_buffer[7], data, maxlen);
    length += maxlen;

    tnfs_sendReceive(length);
    
    return tnfs_buffer[4] * -1; // return code
}

/* close a file */
int tnfs_close(uint8_t handle)
{
    int length = 5;

    tnfs_prepareCommand(0x23);
    tnfs_buffer[4] = handle;
    tnfs_sendReceive(length);
    
    return tnfs_buffer[4] * -1; // Return code
}

/* Get stat information from a file */
int tnfs_stat(char* filename, struct fstat* st)
{
    int length = 4;

    tnfs_prepareCommand(0x24);
    strcpy(&tnfs_buffer[length], filename);
    length += strlen(filename)+1;

    tnfs_sendReceive(length);
    if(tnfs_buffer[4] == 0x00) {
    	memcpy(&st->mode, &tnfs_buffer[5], 2);
    	memcpy(&st->uid, &tnfs_buffer[7], 2);
    	memcpy(&st->gid, &tnfs_buffer[9], 2);
    	memcpy(&st->size, &tnfs_buffer[11], 4);
    	memcpy(&st->atime, &tnfs_buffer[15], 4);
    	memcpy(&st->mtime, &tnfs_buffer[19], 4);
    	memcpy(&st->ctime, &tnfs_buffer[23], 4);
    	strcpy(st->uidstring, &tnfs_buffer[27]);
    	strcpy(st->gidstring, &tnfs_buffer[28+strlen(st->uidstring)]);
    }
    
    return tnfs_buffer[4] * -1; // Return code
}

/* Seeks to a new position in a file */
int tnfs_lseek(uint8_t handle, uint8_t seektype, uint32_t position)
{
    int length = 10;

    tnfs_prepareCommand(0x25);
    tnfs_buffer[4] = handle;
    tnfs_buffer[5] = seektype;
    memcpy(&tnfs_buffer[6], &position, 4);

    tnfs_sendReceive(length);
   
    return tnfs_buffer[4] * -1; // Return code
}

/* Delete a file */
int tnfs_unlink(char* filename)
{
    int length = 4;

    tnfs_prepareCommand(0x26);
    strcpy(&tnfs_buffer[length], filename);
    length += strlen(filename)+1;

    tnfs_sendReceive(length);
   
    return tnfs_buffer[4] * -1; // Return code
}

/* change permissions for a file */
int tnfs_chmod(uint16_t mode, char* filename)
{
    int length = 6;

    tnfs_prepareCommand(0x27);
    memcpy(&tnfs_buffer[4], &mode, 2);
    strcpy(&tnfs_buffer[length], filename);
    length += strlen(filename)+1;

    tnfs_sendReceive(length);
   
    return tnfs_buffer[4] * -1; // Return code
}

/* rename or moves a file within a filesystem */
int tnfs_rename(char* source, char* destination)
{
    int length = 4;

    tnfs_prepareCommand(0x28);
    strcpy(&tnfs_buffer[length], source);
    length += strlen(source)+1;
    strcpy(&tnfs_buffer[length], destination);
    length += strlen(destination)+1;

    tnfs_sendReceive(length);
   
    return tnfs_buffer[4] * -1; // Return code
}

/* Requests the size of the mounted filesystem */
int tnfs_size(uint32_t* kb)
{
    int length = 4;
    tnfs_prepareCommand(0x30);
    tnfs_sendReceive(length);
    
    memset(kb, 0, sizeof(uint32_t));
    if(tnfs_buffer[4] == 0) {
        memcpy(kb, &tnfs_buffer[5], 4);
    }
   
    return tnfs_buffer[4] * -1; // Return code
}

/* Requests the amount of free space on the filesystem */
int tnfs_free(uint32_t* kb)
{
    int length = 4;
    tnfs_prepareCommand(0x31);
    tnfs_sendReceive(length);
    
    memset(kb, 0, sizeof(uint32_t));
    if(tnfs_buffer[4] == 0) {
        memcpy(kb, &tnfs_buffer[5], 4);
    }
   
    return tnfs_buffer[4] * -1; // Return code
}


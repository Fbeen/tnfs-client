#include "include/tnfs.h"
#include "include/netw.h"

/* helper function to print the date and time from a unix timestamp on the screen */
char* time2human(uint32_t timestamp, char* dest)
{
    struct tm *info;
    time_t rawtime = timestamp;
    info = localtime( &rawtime );
    sprintf(dest, "%d-%d-%d %d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min);
    return dest;
}

/* start of our program. demonstrates the tnfs-client functions */
int main(/* int argc, char const* argv[] */)
{
    uint8_t handle;
    char buffer[256];
    char filename[TNFS_MAX_PATH_LEN];
    char hcreated[20];
    char hmodified[20];
    int fh, rcode;
    
    // netw_connect("tnfs.fujinet.online", TNFS_PORT, true);    
    // netw_connect("192.168.178.119", TNFS_PORT, false);    
    netw_connect("127.0.0.1", TNFS_PORT, false);    
    tnfs_mount("/", "", "");
    
/*  
    // NOBODY WANT TO USE opendir/readdir BECAUSE IT SHOWS HIDDEN FILES AND SPECIAL FILES AND THE LIST IS UNSORTED
    printf("> List of root directory with opendir/readdir:\n\n");
    handle = tnfs_opendir("/");
    while(tnfs_readdir(handle, filename) == 0) {
     	printf("%s\n", filename);
    }
    tnfs_closedir(handle);
*/

    printf("\n[mkdir] Make a new directory \"TEST DIRECTORY\":\n\n");
    tnfs_mkdir("TEST DIRECTORY");
    
    printf("[opendirx/nextdirx] List a directory :\n\n");
    struct dirx_data data;
    struct dirx_item item;

    if(tnfs_opendirx("/", "", 0, 0, &data) == 0)
    {
        printf("This directory has %d entries.\n\n", data.entries );
    
    	printf("[seekdir] Skip first two entries:\n\n");
    	tnfs_seekdir(data.handle, 2);

    	while(tnfs_nextdirx(&data, &item) == 0) {
    	    if(item.flags == 1) {
    	    	printf("[DIR] %s\n", item.name);
    	    } else {
    	        time2human(item.created, hcreated);
    	        time2human(item.modified, hmodified);
    	    	printf("      %s\t%u\tcreated: %s\tmodified: %s\n", item.name, item.size, hcreated, hmodified);
    	    }
    	}
    }
    
    uint32_t position;
    tnfs_telldir(data.handle, &position);
    printf("\n[telldir] Show current directory position: %u.\n", position);
    tnfs_closedir(data.handle);
    
    printf("\n[open/write] Create a file and add some text.\n");
    fh = tnfs_open("Message.txt", TNFS_O_WRONLY | TNFS_O_CREAT, 0b111101101); // mode 755
    if(fh < 0) {
    	printf("error code: %d\n", fh * -1);
    }
    strcpy(buffer, "This file was automatically generated with the tnsf client sample program.");
    tnfs_write(buffer, fh, strlen(buffer));

    printf("\n[lseek] Seek 47 bytes from beginning of the file and write TNSF in capitals.\n");
    tnfs_lseek(fh, SEEK_SET, 47);
    tnfs_write("TNSF", fh, 4);
    tnfs_close(fh);

    printf("\n[open/read] Read a file, the content is: \"");
    fh = tnfs_open("Message.txt", TNFS_O_RDONLY, 0);
    if(fh < 0) {
    	printf("\" error code: %d\n", fh * -1);
    } else {
	int l = tnfs_read(buffer, fh, 256);
	for(int i = 0 ; i < l ; i++) {
	    printf("%c",buffer[i]);
	}
	tnfs_close(fh);
    }
    
    printf("\"\n\n[stat] Get information on a file:\n");
    struct fstat st;
    tnfs_stat("Message.txt", &st);
    printf("mode:  %d\n", st.mode);
    printf("uid:   %d\n", st.uid);
    printf("gid:   %d\n", st.gid);
    printf("size:  %u\n", st.size);
    time_t t = st.atime;
    printf("atime: %s", ctime(&t));
    t = st.mtime;
    printf("mtime: %s", ctime(&t));
    t = st.ctime;
    printf("ctime: %s", ctime(&t));
    printf("uidst: %s\n", st.uidstring);
    printf("gidst: %s\n\n", st.gidstring);
    
    /* chmod doesnt work with the server at this moment of writing */
/*
    rcode = tnfs_chmod(0b101101101, "message.txt");
    if(rcode < 0)
    	printf("[chmod] Error code: %hhX\n\n", rcode * -1);
    else
    	printf("[chmod] message.txt set to readonly.\n\n");
*/
    
    printf("[rename] Rename/move the file from \"Message.txt\" to \"TEST DIRECTORY/Message.bak\".\n\n");
    tnfs_rename("Message.txt", "TEST DIRECTORY/Message.bak");

    printf("-------------------------------------------------------------------------------------------------------\n");
    printf("Take a look to your file system and find the directory and file we created. Press [RETURN] to clean up.\n");
    printf("-------------------------------------------------------------------------------------------------------\n");
    getchar();
    
    printf("[unlink] Delete the file \"TEST DIRECTORY/Message.bak\".\n\n");
    tnfs_unlink("TEST DIRECTORY/Message.bak");
    
    printf("[rmdir] Remove directory \"TEST DIRECTORY\".\n\n");
    tnfs_rmdir("TEST DIRECTORY");

/*
    // size and free commands are not implemented in the current tnfsd linux server!

    uint32_t kb;
    rcode = tnfs_size(&kb);
    if(rcode < 0)
    	printf("[size] Error code: %hhX\n\n", rcode * -1);
    else
    	printf("[size] Total space on server: %d Kb.\n\n", kb);
    
    rcode = tnfs_free(&kb);
    if(rcode < 0)
    	printf("[free] Error code: %hhX\n\n", rcode * -1);
    else
        printf("[free] free space on server: %d Kb.\n\n", kb);
*/

    tnfs_umount();
    netw_disconnect();

    return 0;
}


# NTFS-client c library

## TNFS - The Trivial Network Filesystem

TNFS provides a straightforward protocol that can easily be
implemented on most 8 or 16 bit systems. It's designed to be a bit better than
FTP for the purpose of a filesystem, but not as complex as the "big" network
filesystem protocols like NFS or SMB. It is also designed to be usable
with 'incomplete' TCP/IP stacks (e.g. ones that only support UDP).

### More about the TNFS protocol
For more information about the TNFS protocol direct to the [tnfsd github page from spectrumero](https://github.com/spectrumero/tnfsd). Between the files you will find [tnfs-protocol.md](https://github.com/spectrumero/tnfsd/blob/master/tnfs-protocol.md "tnfs-protocol.md") which explains everything about the TNFS protocol in detail.


## Why this NTFS client c library?

When i heard from the NTFS protocol and Fujinet i searched around the web to find a TNFS client that could run in my Linux environment to test and discover this new protocol. I could find one written in Python but not in C although there is some Fujinet firmware written in c but hell yeah this won't work on a pc without a lot of tweaking. In my experience it is also easier to develop a library on a PC than in an embedded device. This library can be easily transferred to any embedded device that has network capabilities. 

## Usage

The code is pretty simple and straight forwards. It consists of three c files and two header files:

- **main.c** This is where  a demonstration is given of how to use the TNFS library.
- **tnfs.c** This file is the actual library with all the tnfs functions available.
- **netw.c** Consists of network functions such as creating a socket, connecting the client to the server, a send and receive function and some other things. These functions usually look a bit different on an embedded device. For this reason I have kept these functions as separate from the NTFS library as possible.
- The **header files** tnfs.h and netw.h should be clear. These must be inserted into your project in order to use the functions in tnfs.c and netw.c respectively.

### Setting up a test environment 

 1. **Setting up a test server**
 Download the tnfsd server. There are already a compiled Linux and Windows [executable](https://github.com/spectrumero/tnfsd/tree/master/bin) on the github page from spectrumero. Once downloaded, you can open a terminal window and change to the correct directory. In that directory, type in the name of the file, a space and and specify the root directory where your clients will end up. Example for Linux:
 
> cd ~/Downloads/tnfsd
> ./tnfsd ~/public

***~/Downloads/tnfsd*** is the directory that contains the tnfsd executable.
***~/public*** is the landing page for your clients that connects to your server. 
You can of course change the directory names to suit your situation.
 
 3. **Run the client**
The client has only been tested on Ubuntu 22.04 but should work fairly easily on other operating systems. A small adjustment in netw.c may be necessary. You can compile and link the program files with gcc. The easiest way is to use build.sh or copy the second and last line of this file and paste them into your terminal.

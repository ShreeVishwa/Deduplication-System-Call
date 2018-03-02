				CSE 506
			Homework Assignment #1

First I am taking the arguments from the user and then validating them. If the arguments are insufficient
then I would return back the error saying null arguments. If the arguments are as required then I am checking
if the input files exist and they are in READ ONLY mode and the output file is in WRITE ONLY mode. If all these
conditions are satisfied then then user program calls the syscall. 

Now the syscall checks for the flags that are set and then goes into the appropriate blocks of code as required.
If only the n flag is set then the program checks for the owner of the input files, checks if they are already
deduped/hardlinked, checks for the file sizes, checks if the files are regular files and if all of them pass
then the program moves into opening the files. If either of them fail then the corresponding error number is 
returned. Now the program opens the input files and reads each of them in pages of size 4KB or 4096B and copies
into a buffer and then checks if each of the bytes are same or not. If I find a place where the bytes
are not same then the function breaks and prints an error. If all the bytes are same then it returns the number of 
bytes matched. If along with the N flag, P flag is also set then even if the partial bytes are matching the syscall
returns the partial bytes.

Now when only the P flag is set, then the program again checks for all the required details and if the output
file is hardlinked with either of the input files then it returns saying the files are already hardlinked.
Now the program again reades the files in pages of size 4KB and comapres it contents byte by byte and it writes
then to a temporary file byte by byte and then once the write operation is completed successfully the it is renamed 
with the original output file and the partial bytes matching are returned.

Now when no flags are set then, again all the required checks as mentioned above are performed. If all the
checks pass then only the files are opened and read byte by byte in page size of 4KB. Now as the file contents
are being read the program makes a copy of the file at the same time. If the file contents don't match then the
error is thrown else the program goes forward to deduping. Now the second file is first unlinked, and if the unlink
fails then the temporary file created is unlinked/removed. If the unlink is successful then it is linked with
with the first file. If that link is successful then the temp file is unlinked again. If the link is not 
successful then rename the temporary file created with the original file name. So now the second file is retained 
and the temporary file is unlinked.

If all the above cases pass then the file is successfuly deduped/hardlinked.

Files included in the submission:
1) Sources - sys_xdedup.c ---- syscall code
	     xhw1.c ---- user level code 

2) Headers - params.h ---- to pass the parameters as a structure
	     flags.h ----- to check whcih flags are set

3) Makefile

4) Scripts - install_modules.sh

5) README


My own error codes:

-1001 ---- files are not identical
-1000 ---- file sizes are different
-1002 ---- not a regular file
-1003 ---- Output file is hardlinked
-1004 ---- Write to file failed
-1005 ---- Files are already deduped
-1006 ---- Error getting the path

How to run the program:

make
sh install_modeules.sh
./xdedup [-npd] input_file_1 input_file_2 output_file

References:

1) https://elixir.bootlin.com/linux/v4.8/ident/vfs_stat
2) http://www.linuxjournal.com/article/6485?page=0,2



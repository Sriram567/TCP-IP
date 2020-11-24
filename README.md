# Brief Explanation
1)The serverside code is written in server.c and the clientside code is written in client.c.
2)First compile the server code and run it.
```C
gcc server.c -o server // This will compile the code
./server // Execute it
```
3) Compile the client side code 
```C
gcc client.c -o client
```
4) Give the files to be downloaded as command line arguments.
```C
./client <file1> <file2> <file3> ...
```
5) To terminate the server or client either close the terminal session or press CTRL + C.
6) I have included subtle error message. If the file already exists in client folder we will overwrite the file . If the file doesn't have write permissions we will skip and go to other file.
7) After the transcation is completed the prompt may take a little time to appear(1-2 seconds).
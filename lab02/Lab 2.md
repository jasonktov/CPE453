Jason Tov
## Report Format
This report is being written as a markdown file via Obsidian

## 1. Get a MINIX system
#### Approach
In this lab, I used VirtualBox and the provided MINIX3 image.
1. Create a virtual machine using default settings following the lab document's given instructions. I did not give it a specific image, but set it up to run an Other Linux OS.
2. Go to virtual machine Settings->Storage and remove the SATA Controller and add a hard drive: the given MINIX3 disk image.
3. Start the virtual machine and boot into the MINIX3 drive.
#### Problems Encountered
On my first boot, MINIX3 threw an error that the mounted drive could not be found. MINIX3 did not fully boot and I could not run any commands.
#### Solutions
In Settings->Storage, I had to set the MINIX3 disk image to be virtual disk 0 rather than virtual disk 1.
#### Lessons
I had initially assumed that being disk 0 or 1 did not matter, but in minimal OS's such as this, every detail counts and no assumptions should be made about if the OS will run how I think it will run. 

## 2. Login
#### Approach
I was prompted with a login, and I logged in as root
`10.0.0.1 user: root`
#### Problems Encountered 
None
#### Solutions
N/A
#### Lessons
When the lab instructions said to login as root initially, it meant that very literally. When I first saw the login prompt, I entered `root` as a guess, but it worked.
## 3. Create a user account
#### Approach
1. Use `adduser` to add the user `jasonktov` to a group
2. Use `passwd` to set a password
#### Problems Encountered
`adduser` expects a few arguments: the username, group, and home directory. I had issues with each one.
- Username: I had initially wanted the username to be `jasonktov` but there was a limit of 8 characters. 
- Group: The group to add to must be an existing group, so I had to figure out what the existing groups were.
- Home Directory: The home directory must be preexisting
#### Solutions
- Username: I shortened the username to `jasonk`. I should have done `jasont` instead or just `jason`.
- Group: I found a list of user groups and used group `operator`.
	`vi /etc/groups`
- Home Directory: Create a directory for users. There probably is one already intended for users, but I made my own.
	`mkdir /home/users`
Commands run: 
`adduser jasonk operator /home/users`
`passwd jasonk`
#### Lessons Learned
Running `man adduser` helped, however some of the arguments like `group` while clear, required me to look more to find what to enter in as an argument.
## 4. Create a MINIX disk image and use it to store data
#### Approach
1. Create a .img file outside the virtual machine using `dd if=/dev/zero of=testfloppy bs=1024 count-1440`. I used wsl to be able to run this command although I'm using Windows.
2. Link the .img file to the virtual machine by going to Settings->Storage, creating a floppy controller, and adding the .img file.
3. Format the drive in MINIX by using `format`
4. Create a file system in the drive with `mkfs`
5. Mount the drive in `/mnt/floppy`
#### Problems Encountered
- Finding the drive in MINIX: I didn't know how to reference the virtual floppy disk in MINIX.
- Testing to see if the drive was successfully mounted
#### Solutions
- Finding the drive in MINIX: By running `fd`, I learned that floppy drives should appear in `/dev` as `fd0` or `fd1`. Going to `/dev`, I ran `ls | grep fd` and found `fd0`
- I formatted this drive with `format /dev/fd0 1440`
- I created a file system with `mkfs /dev/fd0`
- I mounted the drive with `mkdir /mnt/floppy` and `mount /dev/fd0 /mnt/floppy`. 
- Run `mount` to check if the file system was successfully mounted.
#### Lessons Learned
I learned that the terminal is very short. Running `ls /dev` resulted in a long list of files that scrolled the terminal too far to see if `fd0` was in the list. I had to use pipe operators to further and filter searches.

## 5. Accessing your data from outside MINIX
#### Approach
- Copy the testfloppy.img file to a github repo and push
- On the calpoly unix servers, pull from the repo to get the testfloppy.img
- Use the provided test programs: `minls` and `minget` to access the testfloppy.img
#### Problems Encountered
The unix servers are slow and I'm accessing them through the vpn.
#### Solutions
There isn't a way to avoid the slow unix servers beside trying to use them as little as possible and doing as much work as I can on my local machine.
#### Lessons Learned
I can probably use `scp` to transfer the .img file from my machine to the unix servers, however I already have a github setup, so I might as well just use that. There are many good ways to accomplish something, but there are also many good enough ways.
## 6. Clean up and shut down
#### Approach
Run `shutdown -r now` to properly shutdown the MINIX system
#### Problems Encountered
`shutdown` requires higher permission than what operator has access to.
#### Solutions
`su` to switch to root and run `shutdown`
#### Lessons Learned
`shutdown -r now` means to restart MINIX as well, so just `shutdown now` brings me to a different environment which I won't mess with.



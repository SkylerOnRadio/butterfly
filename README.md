# Butterfly

A **Linux** shell written from scratch in C. Butterfly parses and executes commands using core system calls such as fork, exec, and wait, and it includes a small set of built-in commands, basic output redirection, and background process support.

## Installation

Requirements:


CMake 4.3 or later
A C compiler (GCC or Clang)
Make


Clone the repository, then create a folder named build inside it and use CMake to build the project:

```bash
git clone https://www.github.com/SkylerOnRadio/butterfly.git
mkdir butterfly/build
cd butterfly/build
cmake ..
make
```

Once the build finishes, you're ready to start using the shell.

## Features

### 1. Built-in Commands

Butterfly includes a small set of built-in commands that are handled directly by the shell rather than being passed to the operating system:


echo: prints the given text or arguments to the terminal
cd: changes the current working directory. Running cd with no arguments, or cd ~, moves to the home directory. Running cd <path> moves to the specified path
help: displays a list of all built-in commands supported by the shell
exit: exits the shell
pwd: prints the current working directory to the terminal


Example usage:

```bash
echo Hello, Butterfly!
cd ~
cd /var/log
pwd
help
exit
```

### 2. Output Redirection

Butterfly supports redirecting the output of commands and executables to a file:


- `>`: redirects output to a file in truncating mode. If the file already exists, its contents are cleared before the new output is written. If it does not exist, it is created
- `>>`: redirects output to a file in appending mode. If the file already exists, the new output is added to the end of it. If it does not exist, it is created


Example usage:

```bash
ls -la > listing.txt
echo "another line" >> listing.txt
```

Input redirection (<) and piping (|) are not currently supported.

### 3. Background Tasks

Appending & to the end of a command runs it as a detached background process instead of blocking the shell until it finishes.


When a background process starts, the shell prints its process ID (PID)
When a background process finishes, the shell prints a notification showing which process completed and when it finished


Example usage:

```bash
sleep 10 &
```

The shell prompt returns immediately, and Butterfly notifies you once the background process finishes.
## Other Things

I made this project to test out forking and other systems level functions and programming, while the process has been enjoyable and interesting, the abstraction of the various functions does drive me with curiosity to know how they function underneath. I do find the documentation rather difficult to understand for someone with minimal knowledge about systems programming (I still don't really understand what streams are?) but they are still immensely helpful. Stackoverflow has also been a god send in trying to understand how certain functions work.

I would also like to point out [Stephan Brennan's article](https://brennan.io/2015/01/16/write-a-shell-in-c/) on building a shell in C to help me get a starting point and a basic version. While I may not have really done much more than the article it was immensely helpful at starting with these. Though I would like to say perhaps the explanation of the code snippets could be more elaborated upon, since I had to do a lot of googling and asking AI to understand what each line did, a little more elaboration would be appreciated I think.

Thats all for it folks. If you do happen to encounted any bugs or undefined bahavior please open a bug report.

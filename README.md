# ProgLang
ProgLang is a simple proof-of-concept programming language. It is currently under development and may not have a lot of features.

## Installation
To use ProgLang on your Computer you install the Dependencies.

### Dependencies
| Dependency | Description | Notes |
|------------|-------------|-------|
| GCC/G++ | GNU Compiler Collection | May be preinstalled on many machines |
| LLVM | Low Level Virtual Machine  |  |
| GIT | | Optional |

### Installing Dependencies

#### Arch Linux
```sh
sudo pacman -Syu gcc wget
sh -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

#### Debian and Ubuntu-based systems
```sh
sudo apt-get install build-essential wget
sh -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

### Installing git for easy cloning of the repository (optional)

#### Arch Linux
```sh
sudo pacman -Syu git
```

#### Debian and Ubuntu-based systems
```sh
sudo apt-get install git
```

### Cloning the git repository
This step requires git. If you chose to not use git, use another way to obtain the project files.
```sh
git clone --depth 1 https://github.com/JoCraft2010/proglang.git
cd proglang-master
```

### Compiling
After entering the directory you can use **make** to compile the compiler and the other parts of the project:
```sh
# Compile everything:
make
# Compile only the compiler (binary will be located in compiler/bin by default):
make fix_missing_dirs compiler
# Only run the compiler (compiling language/src/main.proglang by default and putting the binary in language/bin)
make fix_missing_dirs build_proglang
# Only build the Hangman example:
make fix_missing_dirs build_hangman
# Or only run the result located in language/bin/main:
make run_result
```

## Writing a basic program
The ProgLang programming language has a Syntax similar to C/C++ but is way less advanced and has some unique differences.

For beginning you can look at the Hangman example in language/src/hangman.proglang and build it as described previously.

If this example is too complicated here's a simplified Hello, World example with explanation:
```
// Hello, World!

/* Declare the external method printf that'll be added during linking
** @ext means the method is declared externally
** int is the return type of the method, 32-bit integer in this case
** printf is the name of the method
** void* means the first parameter is a pointer of any type
** ~ means after that pointed an indefinite amount of aruments can be supplied.
** ~ is the equivalent to an ellipse in traditional programming languages */
@ext int printf(void*, ~);

/* Declare the main function.
** It is called once at program start */
int main() {

  // Call the previously declared method printf
  printf("Hello, World!\n");

  // Return 0 meaning the program has ended successfully
  return 0;
}
```

For a more detailed list of all features wait for a (maybe) future documentation or read the compiler code.

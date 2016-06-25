# PTT Article Recommender

## Purpose

TODO(?): Fill it.


## Features

TODO(?): Fill it.


## Build & Install

### Install Dependent Packages

This project needs `cmake`, `python3`, C++ compiler, `glibc-dev` and
`gtest`, `libopencc-dev` for the dependent packages.  In debian series
distribution, you can install all of them by a single command:
```
$ sudo apt-get install cmake python3 build-essential libgtest-dev
```

Then you need to compile and build the google-test library because the
package `libgtest-dev` installs only the source code.
```
$ cd /usr/src/gtest
$ sudo cmake CMakeLists.txt
$ sudo make
$ sudo cp *.a /usr/lib
```

Note that you can also install the `google test framework` in other place.
Reference this for details. And then add
`-DGTEST_ROOT=<path_of_the_gtest_you_installed>` as an argument of the
command `cmake` later.

You can also use `-DOPENCC_ROOT=<path_of_the_libopencc_you_installed>` to
specify where the library `opencc` installed if you don't want to install
the `libopencc-dev` on the system.

### Compile & Test

1. Create a directory for storing the built out files.  For example:
   ```
   $ mkdir build && cd build
   ```
2. Configure the makefile
   ```
   $ cmake '<project_root>' [ -DCMAKE_INSTALL_PREFIX=<install_destination> ]
   ```
   If you did not specify the directory to install this project, the
   default path will be `build/installed`.

3. Compile
   ```
   $ make
   ```

4. Test
   ```
   $ make test
   ```
   If you want to get more detail test result, see the readme file in each
   sub-project's source directories for help.
   
   If you want to escape this step and do not want to compile the test program,
   add `-Dtest=OFF` as an argument of `cmake` in previous step.

5. Install
   ```
   $ make install
   ```


## Usage

TODO(?): Fill it.


## Development

### Coding Style

We follow the *Google Coding Style Guides*:
* [Python](https://google.github.io/styleguide/pyguide.html)
* [C++](https://google.github.io/styleguide/cppguide.html)

And there is some general rules:
* All the file names and the directory names can consist only *lower captical
  alphnumeric* and the *dot* symbol.  The only acceptable exception is the
  unit-test code, for example, the testing code for `some_dir/some_file.cc`
  might be at `some_dir/some_file-test.cc`.
* Each text line should contains less or equal to *80 characters*, both
  documents and in source codes.


### Project Structures

The whole project consists of 4 main parts:
* *miner*
  It fetches data from the PTT server and convert the non-structured document
  into structured form.  Then for queries from other parts of the project,
  it can returns easy-handled document fast.
* *article_analysis*
  After Getting lots of documents from the miner, it analysis' the document
  content and relationship between documents.  And it also has a mini server
  handling query about the analysis result.
* *app_server*
  It is a HTTP server handling the client side queries.
* *app_client*
  It is a Google Chrome Extension, which will ask the app_server for the
  recommend article.

All the source codes are put in the directory `src`.

In `src/scripts`, there are lots of useful scripts which can shortent
the cost time for keying commands.

See the `README.md` file in each sub-projects for details.


## Issues & Bugs

TODO(?): Fill it.


## Contributors

TODO(?): Fill it.


## Copyright

See the file `LICENSE` for details.


# TODO(?): Fix all typos.

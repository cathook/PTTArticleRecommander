# Article Analysis

## Usage

TODO(?): Fill it.


## Development

### Some Rules 

* In order to more modulize, we splits the whole program into multiple
  files inside multiple directories, and for contents inside a specific
  file (for example, `void F() {}` inside `aa/bb.cc`), it must be inside
  the namespace corrospond to the path name of the file (for example,
  `F()` will be `aa::bb::F()`)


### Basic Structure


### About the directory `dummy`

This module is used to prevent the link error because currently nothing
will be compiled and the link command will throw error. After someone 
finished the first non-inline module, it should be removed.

TODO(?): Fill it.


# TODO(?): Fix all the typos.

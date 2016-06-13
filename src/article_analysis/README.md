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

We split the program into modules:

* `logger` - Handles log message of the whole program.
* `utils` - Contains some utility function and tools.

TODO(?): Fill it.


# TODO(?): Fix all the typos.

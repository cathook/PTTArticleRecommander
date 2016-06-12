#! /usr/bin/env python3


import example_module1.a
import example_module2

def main():
    example_module1.a.b()
    example_module2.g()
    print('meow')


if __name__ == '__main__':
    main()

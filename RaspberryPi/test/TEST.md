# Testing

The modules provided in this test directory are for testing purposes only. The individual files with names of the format `file_test.cpp` are meant to be standalone tests with which the main project may be compiled, or the main file substituted by any one of them.

The files included inside `./include/` and `./src` are meant to be provide implementation of functions that aid in the testing of the main compilation. These functions may be added to the pipeline, or in some cases should substitute some other function. 

*Note:* Some of the tests are configured to be built via the provided Makefile; check for available options. 

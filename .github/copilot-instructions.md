MOST IMPORTANT INSTRUCTION: This is Vyn, NOT Rust. Do not use Rust grammar. Refresh periodically by reading examples, test, README.md, and all doc/*.md, and todo/roadmap to recenter your attention to Vyn.

The top-level directory has a script, build.sh. DO NOT USE THIS UNLESS I TELL YOU TO, the build directory will be wiped out and all source files will be compiled if successful, also in build/build_output.log will be the log of the make process for debugging the build if it passes or fails look here first. 

INSTEAD, in the top level directory you can 'make -C build -j' directly! 

The CMakeLists.txt is also in the top directory, you can regenerate a Makefile and build by 'mkdir -p build && cd build && cmake .. && make clean && make -j'

All tests are in test/  
The preferred method you can directly run 'build/vyn test/some_test.vyn' or what ever test you want to run in that or any directory. 
If you create a test for a new feature, put it in the test/ directory. 
DO NOT USE 'python3 run_tests.py --vyn <path to .vyn file>' script there to help run tests.

Do not pay attention to the 'vyn --help' messages, they are out of sync. Instead read the main.cpp source to understand what is going on.

Make regular git commits after fixing an item. A big problems is updating and fixing items and not commiting them and waiting until push. If they are committed locally in increments, they can always be updated prior to push and as well there is local history that is easily wound back if needed.

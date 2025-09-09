# miniaudioex-dev
This a development branch for future versions of miniaudioex.

# Building
There's no need to install any dependencies. On Windows and macOS there's no need to link to  anything. On Linux just link to `-lpthread`, `-lm` and `-ldl`. On BSD just link to `-lpthread` and `-lm`. On iOS you need to compile as Objective-C.

If you get errors about undefined references to `__sync_val_compare_and_swap_8`, `__atomic_load_8`, etc. you need to link with `-latomic`.
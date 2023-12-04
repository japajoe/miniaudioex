#miniaudioex
This is a modified version of the [MiniAudio](https://github.com/mackron/miniaudio) library. My aim was to extend the library so I can more easily use it in applications, and write simpler bindings for other languages. Most notable feature of this extension is the concept of an audio source. By design, MiniAudio works on a 'per sound basis', which might be desirable for some people, but for me it's not. Instead I like to create one or multiple audio sources which can all play the same sound simultaneously, while still being able to control the properties of the sound (such as spatial settings) through the audio source. If you are familiar with how audio works in the Unity game engine, then you probably get the idea.

#Building
There's no need to install any dependencies. On Windows and macOS there's no need to link to  anything. On Linux just link to `-lpthread`, `-lm` and `-ldl`. On BSD just link to `-lpthread` and `-lm`. On iOS you need to compile as Objective-C.

If you get errors about undefined references to `__sync_val_compare_and_swap_8`, `__atomic_load_8`, etc. you need to link with `-latomic`.
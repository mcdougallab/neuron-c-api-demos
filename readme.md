# NEURON C API Examples
This repository is at an early stage of development, but it is intended to hold examples of using NEURON via C.

Due to its early stage of development, the code currently assumes a `pip install neuron==8.2.0` on a mac with x86_64. The main dependencies here are the name of the library (`.dylib` on a mac, but should be `.dll` on Windows or `.so` on Linux) and how name mangling is handled for certain functions that are not `extern "C"` in the NEURON 8.2.0.

To run the demo change into the `demo1` folder and type `make demo1`. To actually run this, the file `libnrniv.dylib` needs to be on the library search path (easiest is to just put it in the folder).

To find `libnrniv.dylib`, locate NEURON's `__init__.py` file via e.g. `
```
python -c "import neuron; print(neuron.__file__)"
```
and in the folder that contains it, there should be a `.dylibs/libnrniv.dylib`. (Note: the `.dylibs` folder is hidden by default since it starts with a `.` but you can still change into that folder).

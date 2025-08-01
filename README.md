This is a bunch of templates and functions that make maya node creation not completely terrible.
There's so much boilerplate to get through to just get and set data from plugs in a node's compute method.
This made me angry, so I wrote this library.

I've tried to template everything so that the compiler can optimize most of it away.
And for now, this is just a single header file for you to drop into your project.

## The really useful functions are

### getDenseArrayHandleData and getDenseArrayChildHandleData

These get data from array plugs (or children of array plugs) and put them in std::vectors or M*Arrays
Filling in any undefined spots with a default value.

### getSparseArrayHandleData and getSparseArrayChildHandleData

These get data from array plugs (or children of array plugs) and put them in std::unordered_maps keyed by the indices

### setOutputArrayData

Sets an index of an output plug array (or its children) to the given value.
This automatically deals with all the MArrayDataBuilder and stuff and using the correct MFn* for building output M*Array data types

### MArrayInputDataHandleRange

A nice range-based iterator over the sparse values of an MArrayDataHandle.

`for (auto [index, handle] : MArrayInputDataHandleRange(arrayHandle)) { ... }`


---
There's more templates in there for automatically getting the correct function sets or data types

There's also some stupid functions at the bottom that aren't meant to be used. They're just there to demonstrate boilerplate stuff that I always forget.

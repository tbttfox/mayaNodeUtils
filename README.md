This is a bunch of templates and functions that make maya node creation not completely terrible.
There's so much boilerplate to get through to just get and set data from plugs in a node's compute method.
This made me angry, so I wrote this library.

I've tried to template everything so that the compiler can optimize most of it away.
And for now, this is just a single header file for you to drop into your project.

Everything is in the `maya_node_utils` namespace, so you may want to add `using namespace maya_node_utils;` to your source file.


## The really useful functions are ...

### getFullArrayHandleData

These get data from array plugs (or children of array plugs) and put them in std::vectors or M*Arrays
Filling in any undefined spots with a default value.

### getCompactArrayHandleData

These get data from array plugs (or children of array plugs) and put them in std::vectors or M*Arrays
Skipping over any undefined values

### getCompactIndexArrayHandleData

These get data from array plugs (or children of array plugs) and put them in std::vectors or M*Arrays
Skipping over any undefined values, and also returning a std::vector<UINT> of indexes

### getSparseArrayHandleData

These get data from array plugs (or children of array plugs) and put them in std::unordered_maps keyed by the indices

### setOutputArrayData

Sets an index of an output plug array (or its children) to the given value.
This automatically deals with all the MArrayDataBuilder and stuff and using the correct MFn* for building output M*Array data types

### MArrayInputDataHandleRange

A nice range-based iterator over the sparse values of an MArrayDataHandle.

`for (auto [index, handle] : MArrayInputDataHandleRange(arrayHandle)) { ... }`


## The general setup is ...

### Overloading
There's a LOT of overloads for each of the useful functions, but they mostly boil down to "Get the array handle to loop over, and the data handle to get the value from"

Each function name has overloads following this sort of pattern

    // Loop over an array handle directly
    (MArrayDataHandle& arrayHandle, ...)
    (MDataBlock& dataBlock, MObject& attr, ...)

    // Loop over an array handle and get data from any nested set of non-array children
    (MArrayDataHandle& arrayHandle, const std::vector<MObject>& children, ...)
    (MDataBlock& dataBlock, MObject& attr, const std::vector<MObject>& children, ...)


### Value Getter

Most of the functions also take a `valueGetter` which is a callable with signature `(MDataHandle& h, MStatus* status)`.
However, in all cases this argument defaults to `DefaultHandleValueGetter` which should almost always return the correct type for you.

I think it's only really useful to pass the `valueGetter` if you need to handle nested arrays.
But in that case, I'd just `MArrayInputDataHandleRange` and write my own nested loop. It'll be easier than dealing with the lambdas,
and you can use the `defaultHandleValueGetter` (note the lowercase D) wrapper function to get your data.

---
There's a bunch of templates in there for automatically getting the correct function sets or data types like `DefaultHandleValueGetter`, `ElementType`, and `getMFnDataTypeForData`

There's also some stupid functions at the bottom that aren't meant to be used. They're just there to demonstrate boilerplate stuff that I always forget.

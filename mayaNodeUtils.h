/*

Some Example usage:

Get all the aOrigMatrix non-array child values from the aOrig array plug and store it in restMats
Similar for the two others

    std::vector<MVector> restAAs;
    std::vector<MMatrix> restMats;
    std::vector<bool> restUseMats;
    getDenseArrayChildHandleData(dataBlock, aOrig, aOrigMatrix, restMats);
    getDenseArrayChildHandleData(dataBlock, aOrig, aOrigAxisAngle, restAAs);
    getDenseArrayChildHandleData(dataBlock, aOrig, aOrigUseMatrix, restUseMats);



All as one loop to a struct. Probably a little faster, but harder to read.

    // Build a lambda that that makes a tuple of the 3 attributes
    auto valueGetter = [&](MDataHandle& h) {
        return std::make_tuple(
            h.child(aOrigMatrix).asMatrix(),
            h.child(aOrigAxisAngle).asVector(),
            h.child(aOrigUseMatrix).asBool()
        );
    };
    std::vector<std::tuple<MMatrix, MVector, bool>> restMVBs;
    getDenseArrayHandleData(dataBlock, aOrig, restMVBs, valueGetter);


*/

#pragma once

#include <maya/MAngle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MColorArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MDistance.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MIndexMapper.h>
#include <maya/MInt64Array.h>
#include <maya/MIntArray.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrixArray.h>
#include <maya/MObjectArray.h>
#include <maya/MPlugArray.h>
#include <maya/MPointArray.h>
#include <maya/MPxGeometryFilter.h>
#include <maya/MStringArray.h>
#include <maya/MTime.h>
#include <maya/MTimeArray.h>
#include <maya/MTypes.h>  // defines types like float3/double4/int2/etc...
#include <maya/MUint64Array.h>
#include <maya/MUintArray.h>
#include <maya/MVectorArray.h>

#include <iterator>
#include <type_traits>
#include <unordered_map>
#include <utility>  // For std::pair
#include <vector>

namespace maya_node_utils {

/************************************
A consistent way for pulling the stored type from std::vector and Maya arrays
************************************/
// clang-format off
template <typename Container> struct ElementType;
template <typename T, typename Alloc> struct ElementType<std::vector<T, Alloc>> { using type = T; };
template <> struct ElementType<MColorArray>       { using type = MColor;       };
template <> struct ElementType<MDagPathArray>     { using type = MDagPath;     };
template <> struct ElementType<MDoubleArray>      { using type = double;       };
template <> struct ElementType<MFloatArray>       { using type = float;        };
template <> struct ElementType<MFloatPointArray>  { using type = MFloatPoint;  };
template <> struct ElementType<MFloatVectorArray> { using type = MFloatVector; };
template <> struct ElementType<MInt64Array>       { using type = MInt64;       };
template <> struct ElementType<MIntArray>         { using type = int;          };
template <> struct ElementType<MMatrixArray>      { using type = MMatrix;      };
template <> struct ElementType<MObjectArray>      { using type = MObject;      };
template <> struct ElementType<MPlugArray>        { using type = MPlug;        };
template <> struct ElementType<MPointArray>       { using type = MPoint;       };
template <> struct ElementType<MStringArray>      { using type = MString;      };
template <> struct ElementType<MTimeArray>        { using type = MTime;        };
template <> struct ElementType<MUint64Array>      { using type = MUint64;      };
template <> struct ElementType<MUintArray>        { using type = MUint;        };
template <> struct ElementType<MVectorArray>      { using type = MVector;      };

template <typename Container> struct FnSetType;
template <> struct FnSetType<MDoubleArray>      { using type = MFnDoubleArrayData;      };
template <> struct FnSetType<MFloatArray>       { using type = MFnFloatArrayData;       };
template <> struct FnSetType<MFloatVectorArray> { using type = MFnFloatVectorArrayData; };
template <> struct FnSetType<MIntArray>         { using type = MFnIntArrayData;         };
template <> struct FnSetType<MMatrixArray>      { using type = MFnMatrixArrayData;      };
template <> struct FnSetType<MPointArray>       { using type = MFnPointArrayData;       };
template <> struct FnSetType<MStringArray>      { using type = MFnStringArrayData;      };
template <> struct FnSetType<MUint64Array>      { using type = MFnUInt64ArrayData;      };
template <> struct FnSetType<MUintArray>        { using type = MFnUintArrayData;        };
template <> struct FnSetType<MVectorArray>      { using type = MFnVectorArrayData;      };

template <typename Container> using ETypeT = typename ElementType<Container>::type;
template <typename Container> using FnSetTypeT = typename FnSetType<Container>::type;

template <typename T, typename MFnT>
inline T MObjectHandleValueGetter_impl(MObject& ret) {
    MStatus stat;
    MFnT mfnd(ret, &stat);
    if (stat) {
        return mfnd.array();
    }
    return T();
}

template <typename T>
struct DefaultHandleValueGetter {
    T operator()(MDataHandle& handle) const {

        if constexpr      (std::is_same_v<T, MAngle>)        { return handle.asAngle();       }
        else if constexpr (std::is_same_v<T, MTime>)         { return handle.asTime();        }
        else if constexpr (std::is_same_v<T, MDistance>)     { return handle.asDistance();    }
        else if constexpr (std::is_same_v<T, MString>)       { return handle.asString();      }
        else if constexpr (std::is_same_v<T, MVector>)       { return handle.asVector();      }
        else if constexpr (std::is_same_v<T, bool>)          { return handle.asBool();        }
        else if constexpr (std::is_same_v<T, char>)          { return handle.asChar();        }
        else if constexpr (std::is_same_v<T, double>)        { return handle.asDouble();      }
        else if constexpr (std::is_same_v<T, double2>)       { return handle.asDouble2();     }
        else if constexpr (std::is_same_v<T, double3>)       { return handle.asDouble3();     }
        else if constexpr (std::is_same_v<T, double4>)       { return handle.asDouble4();     }
        else if constexpr (std::is_same_v<T, float>)         { return handle.asFloat();       }
        else if constexpr (std::is_same_v<T, float2>)        { return handle.asFloat2();      }
        else if constexpr (std::is_same_v<T, float3>)        { return handle.asFloat3();      }
        else if constexpr (std::is_same_v<T, int>)           { return handle.asInt();         }
        else if constexpr (std::is_same_v<T, int2>)          { return handle.asInt2();        }
        else if constexpr (std::is_same_v<T, int3>)          { return handle.asInt3();        }
        else if constexpr (std::is_same_v<T, short>)         { return handle.asShort();       }
        else if constexpr (std::is_same_v<T, short2>)        { return handle.asShort2();      }
        else if constexpr (std::is_same_v<T, short3>)        { return handle.asShort3();      }
        else if constexpr (std::is_same_v<T, unsigned char>) { return handle.asUChar();       }
        else if constexpr (std::is_same_v<T, MFloatMatrix>)  { return handle.asFloatMatrix(); }
        else if constexpr (std::is_same_v<T, MFloatVector>)  { return handle.asFloatVector(); }
        else if constexpr (std::is_same_v<T, MInt64>)        { return handle.asInt64();       }
        else if constexpr (std::is_same_v<T, MMatrix>)       { return handle.asMatrix();      }

        else if constexpr (std::is_same_v<T, MDoubleArray>)      { return MObjectHandleValueGetter_impl<T, MFnDoubleArrayData>     (handle.data()); }
        else if constexpr (std::is_same_v<T, MFloatArray>)       { return MObjectHandleValueGetter_impl<T, MFnFloatArrayData>      (handle.data()); }
        else if constexpr (std::is_same_v<T, MIntArray>)         { return MObjectHandleValueGetter_impl<T, MFnIntArrayData>        (handle.data()); }
        else if constexpr (std::is_same_v<T, MMatrixArray>)      { return MObjectHandleValueGetter_impl<T, MFnMatrixArrayData>     (handle.data()); }
        else if constexpr (std::is_same_v<T, MPointArray>)       { return MObjectHandleValueGetter_impl<T, MFnPointArrayData>      (handle.data()); }
        else if constexpr (std::is_same_v<T, MStringArray>)      { return MObjectHandleValueGetter_impl<T, MFnStringArrayData>     (handle.data()); }
        else if constexpr (std::is_same_v<T, MUint64Array>)      { return MObjectHandleValueGetter_impl<T, MFnUInt64ArrayData>     (handle.data()); }
        else if constexpr (std::is_same_v<T, MUintArray>)        { return MObjectHandleValueGetter_impl<T, MFnUintArrayData>       (handle.data()); }
        else if constexpr (std::is_same_v<T, MFloatVectorArray>) { return MObjectHandleValueGetter_impl<T, MFnFloatVectorArrayData>(handle.data()); }
        else if constexpr (std::is_same_v<T, MVectorArray>)      { return MObjectHandleValueGetter_impl<T, MFnVectorArrayData>     (handle.data()); }

        else { static_assert(false, "Unsupported MDataHandle type."); }
    }
};

template <typename MFnDT>
MFnData::Type getMFnDataTypeForData() {
    if constexpr      (std::is_same_v<MFnDT, MFnNumericData>)       { return MFnData::kNumeric;        }
    else if constexpr (std::is_same_v<MFnDT, MFnPluginData>)        { return MFnData::kPlugin;         }
    else if constexpr (std::is_same_v<MFnDT, MFnGeometryData>)      { return MFnData::kPluginGeometry; }
    else if constexpr (std::is_same_v<MFnDT, MFnStringData>)        { return MFnData::kString;         }
    else if constexpr (std::is_same_v<MFnDT, MFnMatrixData>)        { return MFnData::kMatrix;         }
    else if constexpr (std::is_same_v<MFnDT, MFnStringArrayData>)   { return MFnData::kStringArray;    }
    else if constexpr (std::is_same_v<MFnDT, MFnDoubleArrayData>)   { return MFnData::kDoubleArray;    }
    else if constexpr (std::is_same_v<MFnDT, MFnFloatArrayData>)    { return MFnData::kFloatArray;     }
    else if constexpr (std::is_same_v<MFnDT, MFnIntArrayData>)      { return MFnData::kIntArray;       }
    else if constexpr (std::is_same_v<MFnDT, MFnPointArrayData>)    { return MFnData::kPointArray;     }
    else if constexpr (std::is_same_v<MFnDT, MFnVectorArrayData>)   { return MFnData::kVectorArray;    }
    else if constexpr (std::is_same_v<MFnDT, MFnMatrixArrayData>)   { return MFnData::kMatrixArray;    }
    else if constexpr (std::is_same_v<MFnDT, MFnComponentListData>) { return MFnData::kComponentList;  }
    else if constexpr (std::is_same_v<MFnDT, MFnMeshData>)          { return MFnData::kMesh;           }
    else if constexpr (std::is_same_v<MFnDT, MFnLatticeData>)       { return MFnData::kLattice;        }
    else if constexpr (std::is_same_v<MFnDT, MFnNurbsCurveData>)    { return MFnData::kNurbsCurve;     }
    else if constexpr (std::is_same_v<MFnDT, MFnNurbsSurfaceData>)  { return MFnData::kNurbsSurface;   }
    else if constexpr (std::is_same_v<MFnDT, MFnSphereData>)        { return MFnData::kSphere;         }
    else if constexpr (std::is_same_v<MFnDT, MFnArrayAttrsData>)    { return MFnData::kDynArrayAttrs;  }
    else if constexpr (std::is_same_v<MFnDT, MFnSubdData>)          { return MFnData::kSubdSurface;    }
    else if constexpr (std::is_same_v<MFnDT, MFnNObjectData>)       { return MFnData::kNObject;        }
    else if constexpr (std::is_same_v<MFnDT, MFnNIdData>)           { return MFnData::kNId;            }
    else {static_assert(false, "Unsupported MDataHandle type.");}
}
// clang-format on

/************************************
Range Iterator for MArrayDataHandles
************************************/

class MArrayInputDataHandleRange {
   public:
    class Iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<unsigned int, MDataHandle>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        explicit Iterator(MArrayDataHandle* handle, unsigned int index = 0)
            : m_handle(handle), m_index(index), m_count(handle ? handle->elementCount() : 0) {
            if (m_handle && m_index < m_count) {
                m_handle->jumpToArrayElement(m_index);
            }
        }

        // Dereference operator returning {index, inputHandle}
        value_type operator*() const { return {m_handle->elementIndex(), m_handle->inputValue()}; }

        // Pre-increment
        Iterator& operator++() {
            if (m_handle && ++m_index < m_count) {
                m_handle->jumpToArrayElement(m_index);
            }
            return *this;
        }

        // Post-increment
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        // Pre-decrement
        Iterator& operator--() {
            if (m_handle && m_index > 0) {
                --m_index;
                m_handle->jumpToArrayElement(m_index);
            }
            return *this;
        }

        // Post-decrement
        Iterator operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        // Comparisons
        bool operator==(const Iterator& other) const {
            return m_index == other.m_index && m_handle == other.m_handle;
        }
        bool operator!=(const Iterator& other) const { return !(*this == other); }

       private:
        MArrayDataHandle* m_handle;
        unsigned int m_index;
        unsigned int m_count;
    };

    explicit MArrayInputDataHandleRange(MArrayDataHandle& handle) : m_handle(handle) {}

    Iterator begin() { return Iterator(&m_handle, 0); }
    Iterator end() { return Iterator(&m_handle, m_handle.elementCount()); }

   private:
    MArrayDataHandle& m_handle;
};

/************************************
Templates for putting typed data into MDataHandles
************************************/

struct HandleBuilder {
    MDataHandle handle;
    MArrayDataBuilder builder;
};

inline MDataHandle setOutputArrayData(MDataHandle& handle, const std::vector<MObject>& children) {
    MStatus status;
    for (const MObject& child : children) {
        handle = handle.child(child);
    }
    return handle;
}

inline HandleBuilder setOutputArrayData(
    MArrayDataHandle& arrayHandle, unsigned int index, const std::vector<MObject>& children
) {
    MStatus status;

    MArrayDataBuilder builder = arrayHandle.builder(&status);
    CHECK_MSTATUS(status);

    MDataHandle parhandle = builder.addElement(index, &status);
    CHECK_MSTATUS(status);
    MDataHandle childhandle = setOutputArrayData(parhandle, children);
    return {childhandle, builder};
}

template <typename T>
inline MDataHandle setOutputArrayData(
    MArrayDataHandle& arrayHandle, unsigned int index, const std::vector<MObject>& children,
    T& value
) {
    using FnSet = FnSetTypeT<T>;
    MStatus status;
    auto [handle, builder] = setOutputArrayData(arrayHandle, index, children);

    // Create a new typed data object with the correct function set
    FnSet fnData;
    MObject dataObj = fnData.create(&status);
    CHECK_MSTATUS(status);

    // Set the value
    fnData.set(value);

    // Store the object on the element
    handle.set(dataObj);
    handle.setClean();

    // Reassign builder to array handle
    arrayHandle.set(builder);
    arrayHandle.setClean();
}

template <typename T>
inline MDataHandle setOutputArrayData(
    MDataBlock& block, MObject& parAttr, unsigned int index, const std::vector<MObject>& children,
    T& value
) {
    MStatus stat;
    MArrayDataHandle arrayHandle = block.outputArrayValue(parAttr, &stat);
    return setOutputArrayData(arrayHandle, index, children, value);
}

/************************************
Templates for getting data out of MDataHandles
************************************/
/************************************
Templates for appending a value to an stl or maya array
************************************/

// Maya array append default
template <typename ArrayType>
inline auto defaulter(ArrayType& array)
    -> decltype(std::declval<ArrayType>().setLength(array.length() + 1), void()) {
    array.setLength(array.length() + 1);
}

// STL vector append default
template <typename ArrayType>
inline auto defaulter(ArrayType& array)
    -> decltype(std::declval<ArrayType>().emplace_back(), void()) {
    array.emplace_back();
}

// Maya array .append
template <typename ArrayType, typename ValueType>
inline auto appender(ArrayType& outArray, ValueType& val)
    -> decltype(std::declval<ArrayType>().append(std::declval<ETypeT<ArrayType>>()), void()) {
    outArray.append(val);
}

// STL vector .push_back
template <typename ArrayType, typename ValueType>
inline auto appender(ArrayType& outArray, ValueType& val)
    -> decltype(std::declval<ArrayType>().push_back(std::declval<ETypeT<ArrayType>>()), void()) {
    outArray.push_back(val);
}

/************************************
Dense getter templates
************************************/

template <typename DefaultPusher, typename ValuePusher>
inline void getDenseArrayMultiHandleData(
    MArrayDataHandle& arrayHandle, unsigned int minSize, DefaultPusher defaultPusher,
    ValuePusher valuePusher
) {
    unsigned int prevIdx = 0;
    for (auto [index, handle] : MArrayInputDataHandleRange(arrayHandle)) {
        for (; prevIdx < index; ++prevIdx) {
            defaultPusher();
        }
        valuePusher(index, handle);
        prevIdx = index + 1;
    }

    // Fill up to the requested min size
    for (; prevIdx < minSize; ++prevIdx) {
        defaultPusher();
    }
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getDenseArrayHandleData(
    MArrayDataHandle& arrayHandle, T& ret, unsigned int minSize = 0,
    ValueGetter valueGetter = ValueGetter()
) {
    auto valuePusher = [&ret, &valueGetter](unsigned int index, MDataHandle& handle) {
        appender(ret, valueGetter(handle));
    };
    getDenseArrayMultiHandleData(arrayHandle, minSize, defaulter, valuePusher);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getDenseArrayHandleData(
    MDataBlock& dataBlock, MObject& attr, T& ret, unsigned int minSize = 0,
    ValueGetter valueGetter = ValueGetter()

) {
    MArrayDataHandle arrayHandle = dataBlock.inputArrayValue(attr);
    getDenseArrayHandleData(arrayHandle, ret, minSize, valueGetter);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getDenseArrayChildHandleData(
    MArrayDataHandle& arrayHandle, MObject& childAttr, T& ret, unsigned int minSize = 0,
    ValueGetter valueGetter = ValueGetter()
) {
    auto childValueGetter = [&childAttr, &valueGetter](MDataHandle& h) {
        return valueGetter(h.child(childAttr));
    };
    getDenseArrayHandleData(arrayHandle, ret, minSize, childValueGetter);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getDenseArrayChildHandleData(
    MDataBlock& dataBlock, MObject& attr, MObject& childAttr, T& ret, unsigned int minSize = 0,
    ValueGetter valueGetter = ValueGetter()
) {
    MArrayDataHandle handle = dataBlock.inputArrayValue(attr);
    getDenseArrayChildHandleData(handle, ret, childAttr, minSize, valueGetter);
}

/************************************
Sparse getter templates
************************************/
template <typename T, typename ValuePusher>
inline void getSparseArrayMultiHandleData(MArrayDataHandle& arrayHandle, ValuePusher valuePusher) {
    for (auto [index, handle] : MArrayInputDataHandleRange(arrayHandle)) {
        valuePusher(handle);
    }
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getSparseArrayHandleData(
    MArrayDataHandle& arrayHandle, std::unordered_map<unsigned int, T>& ret,
    ValueGetter valueGetter = ValueGetter()
) {
    auto valuePusher = [&ret, &valueGetter](unsigned int index, MDataHandle& handle) {
        ret[index] = valueGetter(handle);
    };
    getSparseArrayMultiHandleData(arrayHandle, valuePusher);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getSparseArrayHandleData(
    MDataBlock& dataBlock, MObject& attr, std::unordered_map<unsigned int, T>& ret,
    ValueGetter valueGetter = ValueGetter()
) {
    MArrayDataHandle arrayHandle = dataBlock.inputArrayValue(attr);
    getSparseArrayHandleData<T>(arrayHandle, ret, valueGetter);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getSparseArrayHandleData(
    MArrayDataHandle& arrayHandle, MObject& childAttr, std::unordered_map<unsigned int, T>& ret,
    ValueGetter valueGetter = ValueGetter()
) {
    auto childValueGetter = [&childAttr, &valueGetter](MDataHandle& h) {
        return valueGetter(h.child(childAttr));
    };
    getSparseArrayChildHandleData<T>(arrayHandle, ret, childValueGetter);
}

template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline void getSparseArrayChildHandleData(
    MDataBlock& dataBlock, MObject& attr, std::unordered_map<unsigned int, T>& ret,
    ValueGetter valueGetter = ValueGetter()
) {
    MArrayDataHandle arrayHandle = dataBlock.inputArrayValue(attr);
    getSparseArrayChildHandleData<T>(arrayHandle, ret, valueGetter);
}

/************************************
Reminder templates
*************************************
These templates aren't here for use in a tool(though they could technically be used)
They're mostly here as reminders of how to accomplish common tasks that aren't very obvious
************************************/

/*
Get the vertex indices that are deformerd by *self
geomIndex is normally 0

Returns
    bool: True if we're working on the whole mesh

Return By Ref
    affectMap: The list of indices we're affecting
    affectCount: The number of vertices we're affecting
*/
template <typename T>
inline bool getAffectedIndices(
    T* self,  // MPxGeometryFilter or MPxDeformerNode
    unsigned int geomIndex, MUintArray& affectMap, unsigned int& affectCount
) {
    MIndexMapper mapper = self->indexMapper(geomIndex);
    affectCount = mapper.affectCount();
    affectMap = mapper.affectMap();
    return mapper.isIdentityMap();
}

/*
How to have an input plug forwarded to the output so we can make changes to that output
*/
template <typename T, typename ValueGetter = DefaultHandleValueGetter<ETypeT<T>>>
inline T forwardInputPlug(
    MDataBlock& dataBlock, MObject& inAttr, MObject& outAttr,
    ValueGetter valueGetter = ValueGetter()
) {
    // get the input geometry and input groupId
    MDataHandle hInputGeom = dataBlock.inputValue(inAttr);
    MDataHandle hOutput = dataBlock.outputValue(outAttr);
    hOutput.copy(hInputGeom);
    return valueGetter(hOutput);
}

/*
Correctly set up a typed attribute as an output
*/

template <typename MFnDT>
void createOutputTypedAttr(MString& longName, MString& shortName) {
    MStatus status;
    MFnTypedAttribute tAttr;
    MFnDT fnTypedData;
    MFnData::Type dtype = getMFnDataTypeForData<MFnDT>();
    MObject defaultObject = fnTypedData.create(&status);
    MObject mOutSubDArrayAttr = tAttr.create(longName, shortName, dtype, defaultObject);
}

}  // namespace maya_utils

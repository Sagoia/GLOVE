/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/**
 *  @file       arrays.hpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      A simple interface is provided for handling all the accesses to
 *              the arrays of classes needed in GLOVE using the map container.
 *
 */

#ifndef __ARRAYS_HPP__
#define __ARRAYS_HPP__

#include <stdlib.h>

template <class T, uint32_t N = 1>
class Array {
private:
    typedef T Element;
    const static uint32_t ElementSize = sizeof(Element);
    const static uint32_t DefaultCapacity = N;
        
    Element *mData;
    uint32_t mCapacity;
    uint32_t mSize;

    void Expend(uint32_t capacity) {
        Element *oldData = mData;
        uint32_t oldSize = ElementSize * mCapacity;
        mCapacity = capacity;
        mData = (Element *)malloc(ElementSize * mCapacity);
        memcpy(mData, oldData, oldSize);
        free(oldData);
    }
        
public:
    Array(uint32_t capacity = DefaultCapacity)
    : mCapacity(capacity), mSize(0) {
        if (mCapacity == 0) { mCapacity = 1; }
        mData = (Element *)malloc(ElementSize * mCapacity);
    }
        
    ~Array(void) { free(mData); }
        
    inline void             Clear(void)                         { mSize = 0; }
    inline bool             Empty(void)                 const   { return (mSize == 0); }
    inline uint32_t         Capacity(void)              const   { return mCapacity; }
    inline uint32_t         Size(void)                  const   { return mSize; }
    inline Element&         operator [](uint32_t i)             { return mData[i]; }
    inline const Element&   operator [](uint32_t i)     const   { return mData[i]; }
    inline Element&         Back(void)                          { return mData[mSize - 1]; }
    inline void             PopBack(void)                       { if (mSize > 0) { --mSize; } }
    inline void             PushBack(const Element e)           { if (mSize == mCapacity) { Expend(mCapacity << 1);} mData[mSize] = e; ++ mSize; }
    inline void             Erase(uint32_t i)                   { if (i < mSize) { for (uint32_t idx = i + 1; idx < mSize; ++idx) { mData[idx - 1] = mData[idx]; } --mSize; } }
    inline void             Reserve(uint32_t size)              { if (size > mCapacity) { Expend(size); } }
};

/**
 * @brief A templated class for handling the memory allocation, indexing and
 * searching of all the different arrays of classes.
 *
 * A separate map container is created for every class that the GLOVE supports.
 * This map is used later for the creation of the various new objects,
 * their indexing and searching. The key value of the map is used as the GL
 * handle, therefore the 0 value is not permitted. The mapped value is the
 * generated object.
 */
template <class ELEMENT>
class ObjectArray {
private:
    typedef ELEMENT * ELEMENT_PRT;

    uint32_t mCounter;                 /**< The id (key-value of the map)
                                          reserved during the creation of a new
                                          object. */
    Array<ELEMENT_PRT> mObjects;  /**< The templated map container (one
                                          for each different class that maps
                                          id to a specific object). */
public:

    /**
    * @brief The constructor initializes the key value to a non-usable value in
    * order to be assigned later to a correct one, through the allocate method.
    */
    ObjectArray() :
    mCounter(0)
    {

    }

    /**
    * @brief The destructor removes all elements from the map container (which
    * are destroyed), leaving the container with a size of 0.
    */
    ~ObjectArray()
    {
        for (uint32_t i = 0; i < mObjects.Size(); ++i) {
            delete mObjects[i];
        }
        mObjects.Clear();
    }

    /**
    * @brief Returns the GL handle and reserves this as the new key value.
    * @return The GL handle.
    */
    inline uint32_t Allocate()
    {
        ELEMENT *element = new ELEMENT();
        mObjects.PushBack(element);
        return ++mCounter;
    }

    /**
    * @brief Removes from the map container a single element with the given
    * key value (element is  destroyed).
    * @param index: The GL handle of the element to be destroyed.
    */
    inline bool Deallocate(uint32_t index)
    {
        if (index <= mObjects.Size()) {
            delete mObjects[index-1];
            mObjects[index-1] = nullptr;
            return true;
        }
        return false;
    }

    /**
     * @brief Searches the container for an element with a key equivalent to
     * index and returns it.
     * @param index: The GL handle of the element to be found or to be created.
     * @return A pointer to the element in the map.
     *
     * In case the key value is not found (thus, the element does not exist)
     * a new object is created. Consequently this method is the only way to
     * insert a new element in the map.
     */
    inline ELEMENT *Object(uint32_t index)
    {
        if (index <= mObjects.Size()) {
            return mObjects[index-1];
        }
        return nullptr;
    }

    /**
     * @brief Searches the container for an element with a key equivalent to
     * index.
     * @param index: The GL handle of the element to be found.
     * @return The decision whether the element exists or not.
     */
    inline bool ObjectExists(uint32_t index) const
    {
        if (index <= mObjects.Size()) {
            return (mObjects[index-1] != nullptr);
        }
        return false;
    }

    /**
     * @brief Returns the GL handle of a specific element of the container.
     * @param *element: The element to be searched in the container.
     * @return The GL handle of the element.
     *
     * The container is traversed using the mapped value as the search value.
     * The GL handle is returned in case the wanted element exists, else the
     * returned value is 0.
     */
    inline uint32_t GetObjectId(const ELEMENT * element) const
    {
        for (uint32_t i = 0; i < mObjects.Size(); ++i) {
            if (mObjects[i] == element) {
                return i+1;
            }
        }

        return ~0;
    }

    /**
     * @brief Returns the map container for a specific class.
     * @return The map container.
     */
    inline Array<ELEMENT_PRT>& GetObjects(void)
    {
        return mObjects;
    }
};

#endif // __ARRAYS_HPP__

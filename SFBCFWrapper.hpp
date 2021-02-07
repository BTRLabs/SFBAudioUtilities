//
// Copyright (c) 2012 - 2021 Stephen F. Booth <me@sbooth.org>
// MIT license
//

#pragma once

#import <CFNetwork/CFNetwork.h>
#import <CoreFoundation/CoreFoundation.h>
#if !TARGET_OS_IPHONE
#import <ImageIO/ImageIO.h>
#import <Security/Security.h>
#endif

namespace SFB {

/// A wrapper around a Core Foundation object.
///
/// @c CFWrapper simplifies the use of CFTypes in C++ by wrapping a CF object, ensuring
/// @c CFRelease will be called when the @c CFWrapper goes out of scope.
/// @tparam T A @c CFType
template <typename T>
class CFWrapper
{

public:

#pragma mark Creation and Destruction

	/// Creates a new @c CFWrapper
	inline CFWrapper()
	: CFWrapper(nullptr)
	{}

	/// Creates a new @c CFWrapper
	CFWrapper(const CFWrapper& rhs)
	: mObject(rhs.mObject), mRelease(rhs.mRelease)
	{
		if(mObject && mRelease)
			CFRetain(mObject);
	}

	/// Replaces the wrapped object
	CFWrapper& operator=(const CFWrapper& rhs)
	{
		if(mObject != rhs.mObject) {
			if(mObject && mRelease)
				CFRelease(mObject);

			mObject = rhs.mObject;
			mRelease = rhs.mRelease;

			if(mObject && mRelease)
				CFRetain(mObject);
		}

		return *this;
	}

	/// Destroys this @c CFWrapper and ensure @c CFRelease() is called if necessary
	~CFWrapper()
	{
		if(mObject && mRelease)
			CFRelease(mObject);
		mObject = nullptr;
	}

	/// Creates a new @c CFWrapper
	CFWrapper(CFWrapper&& rhs)
	: mObject(rhs.mObject), mRelease(rhs.mRelease)
	{
		rhs.mObject = nullptr;
	}

	/// Replaces the wrapped object
	CFWrapper& operator=(CFWrapper&& rhs)
	{
		if(mObject != rhs.mObject) {
			if(mObject && mRelease)
				CFRelease(mObject);

			mObject = rhs.mObject;
			mRelease = rhs.mRelease;

			rhs.mObject = nullptr;
		}

		return *this;
	}


	/// Create a new @c CFWrapper
	/// @note The @c CFWrapper takes ownership of @c object
	/// @param object The object to wrap
	inline explicit CFWrapper(T object)
	: CFWrapper(object, true)
	{}

	/// Creates a new @c CFWrapper
	/// @param object The object to wrap
	/// @param release Whether this @c CFWrapper should take ownership of @c object
	CFWrapper(T object, bool release)
	: mObject(object), mRelease(release)
	{}

#pragma mark Assignment

	/// Replaces the wrapped object
	/// @note The @c CFWrapper takes ownership of @c rhs
	/// @param rhs The object to wrap
	CFWrapper& operator=(const T& rhs)
	{
		if(mObject != rhs) {
			if(mObject && mRelease)
				CFRelease(mObject);

			mObject = rhs;
			mRelease = true;
		}

		return *this;
	}

#pragma mark Pointer management

	/// Relinquishes ownership of the wrapped object and returns it
	inline T Relinquish()
	{
		T object = mObject;
		mObject = nullptr;

		return object;
	}

#pragma mark Equality testing

	/// Tests two @c CFWrapper objects for equality using @c CFEqual()
	inline bool operator==(const CFWrapper& rhs) const
	{
		if(mObject == rhs.mObject)
			return true;

		// CFEqual doesn't handle nullptr
		if(!mObject || !rhs.mObject)
			return false;

		return CFEqual(mObject, rhs.mObject);
	}

	/// Tests two @c CFWrapper objects for inequality
	inline bool operator!=(const CFWrapper& rhs) const
	{
		return !operator==(rhs);
	}

#pragma mark Core Foundation object access

	/// Returns @c true if the wrapped object is not @c nullptr
	inline explicit operator bool() const
	{
		return mObject != nullptr;
	}

	/// Returns @c true if the wrapped object is @c nullptr
	inline bool operator!() const
	{
		return !operator bool();
	}

	/// Returns the wrapped object
	inline operator T() const
	{
		return mObject;
	}


	/// Returns a pointer to the wrapped object
	inline T * operator&()
	{
		return &mObject;
	}


	/// Returns the wrapped object
	inline T Object() const
	{
		return mObject;
	}

#pragma mark Core Foundation object creation

	/// Creates a new wrapped @c CFStringRef using @c CFStringCreateWithCString with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFStringRef>::value>>
	CFWrapper(const char *cStr, CFStringEncoding encoding)
	: CFWrapper(CFStringCreateWithCString(kCFAllocatorDefault, cStr, encoding))
	{}

	/// Creates a new wrapped @c CFStringRef using @c CFStringCreateWithFormatAndArguments with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFStringRef>::value>>
	CFWrapper(CFDictionaryRef formatOptions, CFStringRef format, ...) CF_FORMAT_FUNCTION(3,4)
	: CFWrapper()
	{
		va_list ap;
		va_start(ap, format);
		*this = CFStringCreateWithFormatAndArguments(kCFAllocatorDefault, formatOptions, format, ap);
		va_end(ap);
	}

	/// Creates a new wrapped @c CFNumberRef using @c CFNumberCreate with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFNumberRef>::value>>
	CFWrapper(CFNumberType theType, const void *valuePtr)
	: CFWrapper(CFNumberCreate(kCFAllocatorDefault, theType, valuePtr))
	{}

	/// Creates a new wrapped @c CFArrayRef using @c CFArrayCreate with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFArrayRef>::value>>
	CFWrapper(const void **values, CFIndex numValues, const CFArrayCallBacks *callBacks)
	: CFWrapper(CFArrayCreate(kCFAllocatorDefault, values, numValues, callBacks))
	{}

	/// Creates a new wrapped @c CFMutableArrayRef using @c CFArrayCreateMutable with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFMutableArrayRef>::value>>
	CFWrapper(CFIndex capacity, const CFArrayCallBacks *callBacks)
	: CFWrapper(CFArrayCreateMutable(kCFAllocatorDefault, capacity, callBacks))
	{}

	/// Creates a new wrapped @c CFDictionaryRef using @c CFDictionaryCreate with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFDictionaryRef>::value>>
	CFWrapper(const void **keys, const void **values, CFIndex numValues, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks)
	: CFWrapper(CFDictionaryCreate(kCFAllocatorDefault, keys, values, numValues, keyCallBacks, valueCallBacks))
	{}

	/// Creates a new wrapped @c CFMutableDictionaryRef using @c CFDictionaryCreateMutable with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFMutableDictionaryRef>::value>>
	CFWrapper(CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks)
	: CFWrapper(CFDictionaryCreateMutable(kCFAllocatorDefault, capacity, keyCallBacks, valueCallBacks))
	{}

	/// Creates a new wrapped @c CFDataRef using @c CFDataCreate with the default allocator
	template <typename = std::enable_if<std::is_same<T, CFDataRef>::value>>
	CFWrapper(const UInt8 *bytes, CFIndex length)
	: CFWrapper(CFDataCreate(kCFAllocatorDefault, bytes, length))
	{}

private:

	/// The Core Foundation object
	T mObject;
	/// Whether @c CFRelease should be called on destruction or reassignment
	bool mRelease;

};

#pragma mark Typedefs for common CF types

/// A wrapped @c CFTypeRef
using CFType = CFWrapper<CFTypeRef>;
/// A wrapped @c CFDataRef
using CFData = CFWrapper<CFDataRef>;
/// A wrapped @c CFMutableDataRef
using CFMutableData = CFWrapper<CFMutableDataRef>;
/// A wrapped @c CFStringRef
using CFString = CFWrapper<CFStringRef>;
/// A wrapped @c CFMutableStringRef
using CFMutableString = CFWrapper<CFMutableStringRef>;
/// A wrapped @c CFAttributedStringRef
using CFAttributedString = CFWrapper<CFAttributedStringRef>;
/// A wrapped @c CFMutableAttributedStringRef
using CFMutableAttributedString = CFWrapper<CFMutableAttributedStringRef>;
/// A wrapped @c CFDictionaryRef
using CFDictionary = CFWrapper<CFDictionaryRef>;
/// A wrapped @c CFMutableDictionaryRef
using CFMutableDictionary = CFWrapper<CFMutableDictionaryRef>;
/// A wrapped @c CFArrayRef
using CFArray = CFWrapper<CFArrayRef>;
/// A wrapped @c CFMutableArrayRef
using CFMutableArray = CFWrapper<CFMutableArrayRef>;
/// A wrapped @c CFSetRef
using CFSet = CFWrapper<CFSetRef>;
/// A wrapped @c CFMutableSetRef
using CFMutableSet = CFWrapper<CFMutableSetRef>;
/// A wrapped @c CFBagRef
using CFBag = CFWrapper<CFBagRef>;
/// A wrapped @c CFMutableBagRef
using CFMutableBag = CFWrapper<CFMutableBagRef>;
/// A wrapped @c CFPropertyListRef
using CFPropertyList = CFWrapper<CFPropertyListRef>;
/// A wrapped @c CFBitVectorRef
using CFBitVector = CFWrapper<CFBitVectorRef>;
/// A wrapped @c CFMutableBitVectorRef
using CFMutableBitVector = CFWrapper<CFMutableBitVectorRef>;
/// A wrapped @c CFCharacterSetRef
using CFCharacterSet = CFWrapper<CFCharacterSetRef>;
/// A wrapped @c CFMutableCharacterSetRef
using CFMutableCharacterSet = CFWrapper<CFMutableCharacterSetRef>;
/// A wrapped @c CFURLRef
using CFURL = CFWrapper<CFURLRef>;
/// A wrapped @c CFUUIDRef
using CFUUID = CFWrapper<CFUUIDRef>;
/// A wrapped @c CFNumberRef
using CFNumber = CFWrapper<CFNumberRef>;
/// A wrapped @c CFBooleanRef
using CFBoolean = CFWrapper<CFBooleanRef>;
/// A wrapped @c CFErrorRef
using CFError = CFWrapper<CFErrorRef>;
/// A wrapped @c CFDateRef
using CFDate = CFWrapper<CFDateRef>;
/// A wrapped @c CFReadStreamRef
using CFReadStream = CFWrapper<CFReadStreamRef>;
/// A wrapped @c CFWriteStreamRef
using CFWriteStream = CFWrapper<CFWriteStreamRef>;
/// A wrapped @c CFHTTPMessageRef
using CFHTTPMessage = CFWrapper<CFHTTPMessageRef>;
#if !TARGET_OS_IPHONE
/// A wrapped @c SecKeychainItemRef
using SecKeychainItem = CFWrapper<SecKeychainItemRef>;
/// A wrapped @c SecCertificateRef
using SecCertificate = CFWrapper<SecCertificateRef>;
/// A wrapped @c SecTransformRef
using SecTransform = CFWrapper<SecTransformRef>;
/// A wrapped @c CGImageSourceRef
using CGImageSource = CFWrapper<CGImageSourceRef>;
#endif

} // namespace SFB

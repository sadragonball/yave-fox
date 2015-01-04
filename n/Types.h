/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/
#ifndef N_TYPES_H
#define N_TYPES_H

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <type_traits>
#include <typeinfo>

namespace n {

namespace core {
	class String;
}

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t byte;
typedef size_t uint;

extern uint typeId;

// be wary of templates therefor be wary of bullshit

template<typename O, typename...>
O &makeOne();

struct NullType
{
	NullType() = delete;
};

struct Nothing
{
	template<typename... Args>
	Nothing(Args...) {}

	template<typename... Args>
	void operator()(Args...) const {}

	template<typename T>
	operator T() const {
		throw Nothing();
	}
};

template<int I>
struct IntToType
{
	static constexpr bool value = I;
};

template<bool B>
struct BoolToType // false
{
	static constexpr bool value = B;
	BoolToType() {}
	BoolToType(std::false_type) {}
	operator std::false_type() const {
		return std::false_type();
	}
};

template<>
struct BoolToType<true>
{
	static constexpr bool value = true;
	BoolToType() {}
	BoolToType(std::true_type) {}
	operator std::true_type() const {
		return std::true_type();
	}
};

typedef BoolToType<false> FalseType;
typedef BoolToType<true> TrueType;

#define N_GEN_TYPE_HAS_MEMBER(className, member) \
template<typename HasMemberType> \
class className { \
	typedef byte Yes[1]; \
	typedef byte No[2]; \
	template<typename U, bool B> \
	struct SFINAE { \
		struct Fallback { int member; }; \
		struct Derived : HasMemberType, Fallback { }; \
		template<class V> \
		static No &test(decltype(V::member) *); \
		template<typename V> \
		static Yes &test(V *); \
		static constexpr bool value = sizeof(test<Derived>(0)) == sizeof(Yes); \
	}; \
	template<typename U> \
	struct SFINAE<U, true> { \
		static constexpr bool value = false; \
	}; \
	static constexpr bool isPrimitive = !std::is_class<HasMemberType>::value && !std::is_union<HasMemberType>::value; \
	public: \
		static constexpr bool value = SFINAE<HasMemberType, isPrimitive>::value; \
};

#define N_GEN_TYPE_HAS_METHOD(className, method) \
template<typename HasMethodType, typename HasMethodRetType, typename... HasMethodArgsType> \
class className { \
	template<typename U, bool B> \
	struct SFINAE { \
		template<typename V> \
		static auto test(V *) -> typename std::is_same<decltype(std::declval<V>().method(std::declval<HasMethodArgsType>()...)), HasMethodRetType>::type; \
		template<typename V> \
		static std::false_type test(...); \
		typedef decltype(test<U>(0)) type; \
	}; \
	template<typename U> \
	struct SFINAE<U, true> { \
		typedef std::false_type type; \
	}; \
	public: \
		static constexpr bool value = SFINAE<HasMethodType, std::is_fundamental<HasMethodType>::value>::type::value; \
};

namespace internal {
	N_GEN_TYPE_HAS_MEMBER(IsConstIterableInternal, const_iterator)
	N_GEN_TYPE_HAS_MEMBER(IsNonConstIterableInternal, iterator)

	template<typename T, bool B>
	struct IsNonConstIterableDispatch
	{
		static constexpr bool value = !std::is_same<typename T::iterator, NullType>::value;
	};

	template<typename T>
	struct IsNonConstIterableDispatch<T, false>
	{
		static constexpr bool value = false;
	};

	template<typename T>
	struct IsNonConstIterable
	{
		static constexpr bool value = IsNonConstIterableDispatch<T, IsNonConstIterableInternal<T>::value>::value;
	};

	template<typename T, bool B>
	struct IsConstIterableDispatch
	{
		static constexpr bool value = !std::is_same<typename T::const_iterator, NullType>::value;
	};

	template<typename T>
	struct IsConstIterableDispatch<T, false>
	{
		static constexpr bool value = false;
	};

	template<typename T>
	struct IsConstIterable
	{
		static constexpr bool value = IsConstIterableDispatch<T, IsConstIterableInternal<T>::value>::value;
	};


	template<typename T, bool P>
	struct TypeContentInternal // P = false
	{
		typedef NullType type;
	};

	template<typename T>
	struct TypeContentInternal<T *, true>
	{
		typedef T type;
	};

	template<typename T>
	struct TypeContentInternal<T, false>
	{
		typedef decltype(((T *)0)->operator*()) type;
	};

	template<typename T, bool P>
	struct IsDereferencable // P = false
	{
		typedef byte Yes[1];
		typedef byte No[2];

		static_assert(sizeof(Yes) != sizeof(No), "SFINAE : sizeof(Yes) == sizeof(No)");

		template<typename U>
		static Yes &test(decltype(&U::operator*));
		template<typename U>
		static No &test(...);
		public:
			static constexpr bool value = sizeof(test<T>(0)) == sizeof(Yes);
	};

	template<typename T>
	struct IsDereferencable<T, true>
	{
		static constexpr bool value = false;
	};
}

template<typename T>
struct VoidToNothing
{
	typedef T type;
};

template<>
struct VoidToNothing<void>
{
	typedef Nothing type;
};


template<typename From, typename To> // U from T
class TypeConversion
{
	typedef byte Yes[1];
	typedef byte No[2];

	static_assert(sizeof(Yes) != sizeof(No), "SFINAE : sizeof(Yes) == sizeof(No)");

	static Yes &test(To);
	static No &test(...);

	public:
		static constexpr bool exists = sizeof(test(makeOne<From>())) == sizeof(Yes);

};

class Type
{
	public:
		Type(const std::type_info &i) : info(&i) {
		}

		template<typename T>
		Type(T) : Type(typeid(T)) {
		}


		bool operator==(const Type &t) const {
			return *info == *t.info;
		}

		bool operator!=(const Type &t) const {
			return !operator ==(t);
		}

		bool operator<(const Type &t) const {
			return info->before(*t.info);
		}

		core::String name() const;

	private:
		const std::type_info *info;
};


template<typename T>
struct TypeInfo
{
	static constexpr bool isPrimitive = !std::is_class<T>::value && !std::is_union<T>::value;
	static constexpr bool isPod = std::is_trivial<T>::value || isPrimitive;
	static constexpr bool isPointer = false;
	static constexpr bool isConst = false;
	static constexpr bool isRef = false;
	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = internal::IsNonConstIterable<T>::value;
	static constexpr bool isIterable = internal::IsConstIterable<T>::value || isNonConstIterable;

	static constexpr bool isDereferencable = internal::IsDereferencable<T, isPod>::value;

	static const Type type;

	typedef T nonRef;
	typedef T nonConst;
	typedef T nonPtr;
};

template<typename T>
struct TypeInfo<T *>
{
	static constexpr bool isPod = true;
	static constexpr bool isPrimitive = true;
	static constexpr bool isPointer = true;
	static constexpr bool isConst = TypeInfo<T>::isConst;
	static constexpr bool isRef = TypeInfo<T>::isRef;

	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = false;
	static constexpr bool isIterable = false;

	static constexpr bool isDereferencable = true;

	static const Type type;

	typedef typename TypeInfo<T>::nonRef *nonRef;
	typedef typename TypeInfo<T>::nonConst *nonConst;
	typedef T nonPtr;
};

template<typename T>
struct TypeInfo<const T>
{
	static constexpr bool isPod = TypeInfo<T>::isPod;
	static constexpr bool isPrimitive = TypeInfo<T>::isPrimitive;
	static constexpr bool isPointer = TypeInfo<T>::isPointer;
	static constexpr bool isConst = true;
	static constexpr bool isRef = TypeInfo<T>::isRef;
	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = false;
	static constexpr bool isIterable = internal::IsConstIterable<T>::value;

	static constexpr bool isDereferencable = TypeInfo<T>::isDereferencable;

	static const Type type;

	typedef const typename TypeInfo<T>::nonRef nonRef;
	typedef T nonConst;
	typedef const typename TypeInfo<T>::nonPtr nonPtr;
};

template<typename T>
struct TypeInfo<T &>
{
	static constexpr bool isPod = TypeInfo<T>::isPod;
	static constexpr bool isPrimitive = TypeInfo<T>::isPrimitive;
	static constexpr bool isPointer = TypeInfo<T>::isPointer;
	static constexpr bool isConst = TypeInfo<T>::isConst;
	static constexpr bool isRef = true;
	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = TypeInfo<T>::isNonConstIterable;
	static constexpr bool isIterable = TypeInfo<T>::isIterable;

	static constexpr bool isDereferencable = TypeInfo<T>::isDereferencable;

	static const Type type;

	typedef T nonRef;
	typedef typename TypeInfo<T>::nonConst &nonConst;
	typedef typename TypeInfo<T>::nonPtr &nonPtr;
};

template<typename T>
struct TypeInfo<T[]>
{
	static constexpr bool isPod = TypeInfo<T>::isPod;
	static constexpr bool isPrimitive = TypeInfo<T>::isPrimitive;
	static constexpr bool isPointer = false;
	static constexpr bool isConst = TypeInfo<T>::isConst;
	static constexpr bool isRef = false;
	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = false;
	static constexpr bool isIterable = false;

	static constexpr bool isDereferencable = true;

	static const Type type;

	typedef T nonRef;
	typedef typename TypeInfo<T>::nonConst &nonConst;
	typedef typename TypeInfo<T>::nonPtr &nonPtr;
};

template<typename T, uint N>
struct TypeInfo<T[N]>
{
	static constexpr bool isPod = TypeInfo<T>::isPod;
	static constexpr bool isPrimitive = TypeInfo<T>::isPrimitive;
	static constexpr bool isPointer = false;
	static constexpr bool isConst = TypeInfo<T>::isConst;
	static constexpr bool isRef = false;
	static const uint baseId;
	static const uint id;

	static constexpr bool isNonConstIterable = false;
	static constexpr bool isIterable = false;

	static constexpr bool isDereferencable = true;

	static const Type type;

	typedef T nonRef;
	typedef typename TypeInfo<T>::nonConst &nonConst;
	typedef typename TypeInfo<T>::nonPtr &nonPtr;
};

template<typename T>
const uint TypeInfo<T>::baseId = typeId++; // dependent on compilation, but NOT on execution flow
template<typename T>
const uint TypeInfo<T>::id = TypeInfo<T>::baseId;
template<typename T>
const Type TypeInfo<T>::type = typeid(T);

template<typename T>
const uint TypeInfo<T *>::id = typeId++;
template<typename T>
const uint TypeInfo<T *>::baseId = TypeInfo<T>::baseId;
template<typename T>
const Type TypeInfo<T *>::type = typeid(T *);

template<typename T>
const uint TypeInfo<const T>::id = typeId++;
template<typename T>
const uint TypeInfo<const T>::baseId = TypeInfo<T>::baseId;
template<typename T>
const Type TypeInfo<const T>::type = typeid(const T);

template<typename T>
const uint TypeInfo<T &>::id = typeId++;
template<typename T>
const uint TypeInfo<T &>::baseId = TypeInfo<T>::baseId;
template<typename T>
const Type TypeInfo<T &>::type = typeid(T &);

template<typename T>
const uint TypeInfo<T[]>::id = typeId++;
template<typename T>
const uint TypeInfo<T[]>::baseId = TypeInfo<T>::baseId;
template<typename T>
const Type TypeInfo<T[]>::type = typeid(T[]);

template<typename T, uint N>
const uint TypeInfo<T[N]>::id = typeId++;
template<typename T, uint N>
const uint TypeInfo<T[N]>::baseId = TypeInfo<T>::baseId;
template<typename T, uint N>
const Type TypeInfo<T[N]>::type = typeid(T[N]);

template<typename T>
struct TypeContent
{
	typedef typename internal::TypeContentInternal<T, TypeInfo<T>::isPod || !TypeInfo<T>::isDereferencable>::type type;
};


} //n

#endif // NTYPES_H

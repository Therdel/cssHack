//
// Created by therdel on 06.09.19.
//
#pragma once

struct OmittedType {
};

template<typename OptionalType = OmittedType,
		typename TypeWhenGiven = OmittedType,
		typename TypeWhenOmitted = OmittedType,
		std::enable_if_t<!std::is_same_v<TypeWhenGiven, OmittedType> &&
		                 !std::is_same_v<TypeWhenOmitted, OmittedType>, int> = 0>
struct conditional_type_given {
	using type = typename std::conditional_t<
			std::is_same<OptionalType, OmittedType>::value,
			TypeWhenOmitted,
			TypeWhenGiven>;
};

template<typename OptionalType = OmittedType,
		typename TypeWhenGiven = OmittedType,
		typename TypeWhenOmitted = OmittedType>
using conditional_type_given_t = typename conditional_type_given<OptionalType, TypeWhenOmitted, TypeWhenGiven>::type;
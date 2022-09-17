#pragma once

#include <map>
#include <string>

#include "define.h"

class Function;
/*
 * 类型信息
 */
struct Attr {
	struct Field {
		std::string field_name;
	};
	/*
	 * 类型的方法
	 */
	struct Method {
		TypeId		constraint_tid; // 方法所属constraint. 如果不属于constraint, 则为TYPE_ID_NONE
		std::string method_name;	// 方法名
		Function*	fn;				// 方法的实现
	};

	void InitMethod(TypeId contraint_tid, std::string method_name, Function* fn);

	bool   is_field;
	Field  field;
	Method method;
};

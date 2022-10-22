#pragma once

#include "define.h"
#include "function_obj.h"
#include "type.h"
#include "type_array.h"
#include "type_str.h"

#include <bits/stdint-intn.h>
#include <map>
#include <vector>

class ExecuteContext;
class FunctionObj;
class AstNodeConstraint;
class AstNodeComplexFnDef;

/*
 * 提供一个简单的变量存储功能. 目前不考虑内存占用性能等问题
 */
class Variable {
public:
	Variable(TypeId tid);
	Variable(int32_t value);
	Variable(int64_t value);
	Variable(float value);
	Variable(bool value);
	Variable(std::string value);
	Variable(FunctionObj fnobj);
	Variable(AstNodeConstraint* astnode);
	Variable(AstNodeComplexFnDef* astnode);
	Variable(TypeId array_tid, std::vector<Variable*> array); // array

	static Variable* CreateTypeVariable(TypeId tid);
	static Variable* CreateTypeTuple(TypeId tuple_tid, std::vector<Variable*> elements);

	TypeId GetTypeId() const { return m_tid; }

	TypeId				 GetValueTid() const;
	int32_t				 GetValueInt32() const;
	int64_t				 GetValueInt64() const;
	float				 GetValueFloat() const;
	bool				 GetValueBool() const;
	const char*			 GetValueStr() const;
	int					 GetValueStrSize() const;
	FunctionObj*		 GetValueFunctionObj() const;
	AstNodeConstraint*	 GetValueConstraint() const;
	AstNodeComplexFnDef* GetValueComplexFn() const;

	const int GetValueArraySize() const;
	Variable* GetValueArrayElement(int idx);
	void	  SetValueArrayElement(int idx, Variable* element);

	bool IsConst() const { return m_is_const; }

	bool IsTmp() const { return m_is_tmp; }
	void SetTmp(bool tmp) { m_is_tmp = tmp; }

	/*
	 * 找不到method, panic
	 */
	Variable* CallMethod(ExecuteContext& ctx, MethodIndex method_idx, std::vector<Variable*> args);

	std::string ToString() const;

	Variable* GetMethodValue(MethodIndex method_idx);
	Variable* GetFieldValue(std::string field_name);
	void	  SetFieldValue(std::string field_name, Variable* v);

	void Assign(Variable* tmp);
	void InitField(std::map<std::string, Variable*> fields);

private:
	void set_default_value();

private:
	TypeId m_tid;
	bool   m_is_const;
	bool   m_is_tmp; // 是否是临时变量.

	// reference type
	FunctionObj*					  m_value_fnobj; // function

	union Value {
		// value type
		TypeId	value_tid;	 // type
		int32_t value_int32; // i32
		int64_t value_int64; // i64
		float	value_float; // float
		bool	value_bool;	 // bool

		// reference type
		TypeInfoArray::MemStructure* value_array;  // array
		TypeInfoStr::MemStructure*	 value_str;	   // str
		uint8_t*					 value_fields; // class/tuple
	};
	Value m_value;

	// 编译阶段使用
	AstNodeConstraint*	 m_value_constraint;
	AstNodeComplexFnDef* m_value_complex_fn;
};

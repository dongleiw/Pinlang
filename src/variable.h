#pragma once

#include "define.h"
#include "function_obj.h"
#include "type.h"
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
	Variable(int value);
	Variable(float value);
	Variable(bool value);
	Variable(std::string value);
	Variable(FunctionObj fnobj);
	Variable(AstNodeConstraint* astnode);
	Variable(AstNodeComplexFnDef* astnode);
	Variable(TypeId array_tid, std::vector<Variable*> array);

	static Variable* CreateTypeVariable(TypeId tid);

	TypeId GetTypeId() const { return m_tid; }

	TypeId						 GetValueTid() const;
	int							 GetValueInt() const;
	float						 GetValueFloat() const;
	bool						 GetValueBool() const;
	std::string					 GetValueStr() const;
	FunctionObj					 GetValueFunctionObj() const;
	AstNodeConstraint*			 GetValueConstraint() const;
	AstNodeComplexFnDef*		 GetValueComplexFn() const;
	const std::vector<Variable*> GetValueArray() const;

	bool IsConst() const { return m_is_const; }

	/*
	 * 找不到method, panic
	 */
	Variable* CallMethod(ExecuteContext& ctx, MethodIndex method_idx, std::vector<Variable*> args);

	std::string ToString() const;

	Variable* GetMethodValue(MethodIndex method_idx);
	Variable* GetFieldValue(std::string field_name);

protected:
	TypeId m_tid;
	bool   m_is_const;

	TypeId				   m_value_tid;
	int					   m_value_int;
	float				   m_value_float;
	bool				   m_value_bool;
	std::string			   m_value_str;
	FunctionObj			   m_value_fnobj;
	AstNodeConstraint*	   m_value_constraint;
	AstNodeComplexFnDef*   m_value_complex_fn;
	std::vector<Variable*> m_value_array;

	std::map<std::string, Variable*> m_fields;
};

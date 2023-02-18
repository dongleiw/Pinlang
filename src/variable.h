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
	Variable(float value);
	Variable(bool value);
	Variable(std::string value);
	Variable(TypeId fn_tid, FunctionObj fnobj);
	Variable(AstNodeConstraint* astnode);
	Variable(AstNodeComplexFnDef* astnode);
	Variable(TypeId array_tid, std::vector<Variable*> array); // array

	static Variable* CreateTypeVariable(TypeId tid);
	static Variable* CreateTypeTuple(TypeId tuple_tid, std::vector<Variable*> elements);

	TypeId GetTypeId() const { return m_tid; }

	TypeId				 GetValueTid() const;
	float				 GetValueFloat() const;
	bool				 GetValueBool() const;
	FunctionObj*		 GetValueFunctionObj() const;
	AstNodeConstraint*	 GetValueConstraint() const;
	AstNodeComplexFnDef* GetValueComplexFn() const;

	Variable* GetValueArrayElement(int idx);
	void	  SetValueArrayElement(int idx, Variable* element);

	Variable(int8_t value);
	Variable(int16_t value);
	Variable(int32_t value);
	Variable(int64_t value);
	Variable(uint8_t value);
	Variable(uint16_t value);
	Variable(uint32_t value);
	Variable(uint64_t value);
	int8_t	 GetValueInt8() const;
	int16_t	 GetValueInt16() const;
	int32_t	 GetValueInt32() const;
	int64_t	 GetValueInt64() const;
	uint8_t	 GetValueUInt8() const;
	uint16_t GetValueUInt16() const;
	uint32_t GetValueUInt32() const;
	uint64_t GetValueUInt64() const;

	char*		GetValueStr();
	const char* GetValueStr() const;
	int			GetValueStrSize() const;

	std::string ToString() const;

	Variable* GetFieldValue(std::string field_name);
	void	  SetFieldValue(std::string field_name, Variable* v);

	void Assign(Variable* tmp);
	void InitField(std::map<std::string, Variable*> fields);

private:
	void set_default_value();
	void construct_init(TypeId tid, const void* data);

private:
	TypeId m_tid;

	// 变量的数据
	uint8_t* m_data;

	// 编译阶段使用
	AstNodeConstraint*	 m_value_constraint;
	AstNodeComplexFnDef* m_value_complex_fn;
};

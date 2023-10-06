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
	Variable(double value);
	Variable(bool value);
	Variable(std::string value);
	Variable(TypeId fn_tid, FunctionObj fnobj);
	Variable(AstNodeConstraint* astnode);
	Variable(AstNodeComplexFnDef* astnode);
	Variable(TypeId array_tid, std::vector<Variable*> array); // array

	static Variable* CreateTypeVariable(TypeId tid);
	static Variable* CreateTypeTuple(TypeId tuple_tid, std::vector<Variable*> elements);
	static Variable* CreateTypePointer(TypeId tuple_tid, std::vector<Variable*> elements);

	TypeId GetTypeId() const { return m_tid; }

	TypeId				 GetValueTid() const;
	uint64_t			 GetValueU64() const;
	AstNodeConstraint*	 GetValueConstraint() const;
	AstNodeComplexFnDef* GetValueComplexFn() const;

	Variable(int8_t value);
	Variable(int16_t value);
	Variable(int32_t value);
	Variable(int64_t value);
	Variable(uint8_t value);
	Variable(uint16_t value);
	Variable(uint32_t value);
	Variable(uint64_t value);

	void InitField(std::map<std::string, Variable*> fields);

private:
	void set_default_value();
	void construct_init(TypeId tid, const void* data);

private:
	TypeId m_tid;

	// 编译阶段使用
	TypeId				 m_value_tid;
	uint64_t			 m_value_u64;
	AstNodeConstraint*	 m_value_constraint;
	AstNodeComplexFnDef* m_value_complex_fn;
};

#pragma once

#include "define.h"
#include "type.h"
#include <vector>

class ExecuteContext;
class Function;
class AstNodeRestriction;
class AstNodeGenericFnDef;

class Variable {
public:
	Variable(TypeId tid);
	Variable(int value);
	Variable(float value);
	Variable(std::string value);
	Variable(Function* fn);
	Variable(AstNodeRestriction* astnode);
	Variable(AstNodeGenericFnDef* astnode);

	static Variable* CreateTypeVariable(TypeId tid);

	TypeId GetTypeId() const { return m_tid; }

	TypeId				 GetValueTid() const;
	int					 GetValueInt() const;
	float				 GetValueFloat() const;
	std::string			 GetValueStr() const;
	Function*			 GetValueFunction() const;
	AstNodeRestriction*	 GetValueRestriction() const;
	AstNodeGenericFnDef* GetValueGenericFnDef() const;

	bool IsConst() const { return m_is_const; }

	/*
	 * 找不到method, panic
	 */
	Variable* CallMethod(ExecuteContext& ctx, int method_idx, std::vector<Variable*> args);

	std::string ToString() const;

protected:
	TypeId				 m_tid;
	bool				 m_is_const;
	TypeId				 m_value_tid;
	int					 m_value_int;
	float				 m_value_float;
	std::string			 m_value_str;
	Function*			 m_value_fn;
	AstNodeRestriction*	 m_value_restriction;
	AstNodeGenericFnDef* m_value_generic_fn;
};

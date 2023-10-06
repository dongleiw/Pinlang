#pragma once

#include <llvm-12/llvm/IR/Type.h>
#include <map>
#include <string>
#include <utility>

#include "define.h"
#include "fntable.h"

class CompileContext;
class VerifyContext;
class AstNodeComplexFnDef;

/*
 * 类型信息
 */
class TypeInfo {
public:
	struct Constraint {
		ConstraintInstance	constraint_instance;
		std::vector<Method> methods;
	};

public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_NONE), m_is_value_type(false) {
	}

	TypeId		GetTypeId() const { return m_typeid; }
	void		SetTypeId(TypeId tid);
	std::string GetName() { return m_name; }
	void		SetName(std::string name) { m_name = name; }
	void		SetOriginalName(std::string name) { m_original_name = name; }
	std::string GetOriginalName() const { return m_original_name; }

	bool IsValueType() const { return m_is_value_type; }

	virtual void InitBuiltinMethods(VerifyContext& ctx) {}

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }

	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsArray() const { return m_typegroup_id == TYPE_GROUP_ID_ARRAY; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsConstraint() const { return m_typegroup_id == TYPE_GROUP_ID_CONSTRAINT; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_VIRTUAL_GTYPE; }
	bool IsPrimaryType() const { return m_typegroup_id == TYPE_GROUP_ID_PRIMARY; }
	bool IsClass() const { return m_typegroup_id == TYPE_GROUP_ID_CLASS; }
	bool IsTuple() const { return m_typegroup_id == TYPE_GROUP_ID_TUPLE; }
	bool IsPointer() const { return m_typegroup_id == TYPE_GROUP_ID_POINTER; }

	void AddConstraint(ConstraintInstance constraint_instance, std::vector<AstNodeComplexFnDef*> methods);
	bool MatchConstraint(ConstraintInstance constraint_instance) const;
	/*
	 * 根据constraint的名字 + 方法名字 + 方法参数信息搜索方法. 返回所有匹配的函数位置信息
	 */
	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name, std::vector<TypeId> method_params_tid, TypeId return_tid);
	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name, TypeId tid);
	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name);

	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string method_name, std::vector<TypeId> method_params_tid, TypeId return_tid);
	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string method_name, TypeId tid);
	std::vector<std::string> GetConstraintMethod(VerifyContext& ctx, std::string method_name);

	std::vector<std::string> GetConstructor(VerifyContext& ctx, std::string method_name, std::vector<TypeId> params_tid);
	std::vector<std::string> GetConstructor(VerifyContext& ctx, std::string method_name);
	bool					 IsSimpleConstrcutor(std::string method_name) const;
	bool					 IsSimpleMethod(std::string method_name) const;
	bool					 HasConstructor() const;

	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx);

	// 构造所有field.
	virtual void ConstructFields(CompileContext& cctx, llvm::Value* obj);
	// 缺省构造
	virtual void ConstructDefault(CompileContext& cctx, llvm::Value* obj);

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;
	std::string m_original_name;
	bool		m_is_value_type;

	/*
	 * 该类型实现的约束列表.
	 */
	std::vector<Constraint> m_constraints;
};

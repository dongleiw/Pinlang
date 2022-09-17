#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "function_obj.h"
#include "type.h"
#include "type_fn.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 泛型函数定义
 * fn sum[T Add[T]](a T, b T) T{ 
 *		return a+b; 
 * }
 */
class AstNodeGenericFnDef : public AstNode {
public:
	struct Instance {
		std::vector<TypeId> gparams_tid;
		std::string			instance_name;
		FunctionObj			fnobj;
	};
	// 推导结果
	struct InstantiateParam {
		std::map<std::string, TypeId> map_gparams_tid;
		std::vector<TypeId>			  vec_gparams_tid;
		std::vector<TypeId>			  params_tid;
		TypeId						  return_tid;
	};

public:
	AstNodeGenericFnDef(std::string fn_name, std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*	 DeepClone() override { return DeepCloneT(); }
	AstNodeGenericFnDef* DeepCloneT();

	/*
	 * 根据参数和返回值来推导出泛型参数, 然后实例化出来
	 */
	Instance Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid);
	/*
	 * 根据函数类型来推导出泛型参数, 然后实例化出来
	 */
	Instance Instantiate(VerifyContext& ctx, TypeId tid);
	/*
	 * 根据泛参的实际类型实例化
	 */
	Instance Instantiate(VerifyContext& ctx, std::vector<TypeId> gparams_tid);

private:
	AstNodeGenericFnDef() {}
	/*
	 * 根据实际的泛型类型实例化
	 */
	Instance instantiate(VerifyContext& ctx, InstantiateParam instantiate_param);

	// 将实例添加到vt的合适位置
	void add_instance_to_vt(VerifyContext& ctx, std::string name, FunctionObj fnobj) const;
	// 校验泛型的实际类型是否满足约束
	void verify_constraint(VerifyContext& ctx, std::vector<TypeId> concrete_generic_params) const;
	void verify_body(VerifyContext& ctx);

	bool get_instance(std::vector<TypeId> concrete_gparams, Instance* instance) const;

	// 根据参数类型和返回类型推导
	InstantiateParam infer_by_param_type_and_return_type(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) const;
	InstantiateParam infer_by_gparams(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const;
	InstantiateParam infer_by_typeid(VerifyContext& ctx, TypeId tid) const;

	bool is_generic_param(std::string name) const;

private:
	std::string						m_fnname;
	std::vector<ParserGenericParam> m_generic_params;
	std::vector<ParserParameter>	m_params;
	AstNodeType*					m_return_type;
	AstNodeBlockStmt*				m_body;

	std::vector<Instance> m_instances;
	//std::map<std::string, Function*> m_instances;
};

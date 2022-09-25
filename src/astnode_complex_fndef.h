#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 函数的复杂定义
 * 规则:
 *		如果有多个参数个数相同的定义, 这些定义必须都是非泛型定义.
 */
class AstNodeComplexFnDef : public AstNode {
public:
	class Instance;
	// 函数的一个定义
	struct Implement {
		Implement(std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body, BuiltinFnCallback builtin_callback) {
			m_generic_params   = generic_params;
			m_params		   = params;
			m_return_type	   = return_type;
			m_body			   = body;
			m_builtin_callback = builtin_callback;
			m_return_tid	   = TYPE_ID_NONE;
		}
		std::vector<ParserGenericParam> m_generic_params;
		std::vector<ParserParameter>	m_params;
		AstNodeType*					m_return_type;
		AstNodeBlockStmt*				m_body;
		BuiltinFnCallback				m_builtin_callback;

		// 如果该实现不是泛型(也就是参数类型和返回值类型是固定的), 则根据上下文推导出以下两个信息
		std::vector<TypeId> m_params_tid;
		TypeId				m_return_tid;

		void				Verify(VerifyContext& ctx);
		Implement			DeepClone();
		bool				is_generic_param(std::string name) const;
		std::vector<TypeId> infer_gparams_by_param_return(std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) const;
		bool				satisfy_constraint(VerifyContext&ctx, std::vector<TypeId> gparams_tid) const;
		TypeId				infer_return_type_by_gparams(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const;

	private:
		Implement() {}
	};
	// 函数的一个实例
	struct Instance {
		const Implement*	implement;
		std::vector<TypeId> gparams_tid;
		std::vector<TypeId> params_tid;
		TypeId				return_tid;
		std::string			instance_name;
		FunctionObj			fnobj;
	};

public:
	AstNodeComplexFnDef(std::string fn_name, std::vector<Implement> implements);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*	 DeepClone() override { return DeepCloneT(); }
	AstNodeComplexFnDef* DeepCloneT();

	/*
	 * 实例化
	 * {参数, 返回值} => 推导出泛型参数, 校验泛参是否满足约束, 实例化
	 */
	Instance Instantiate_param_return(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid);
	/*
	 * 实例化
	 * {函数类型} => 推导出泛型参数, 校验泛参是否满足约束, 实例化
	 */
	Instance Instantiate_type(VerifyContext& ctx, TypeId fn_tid);
	/*
	 * 实例化
	 * {泛参实际类型, 参数, 返回值} => 推导出泛型参数, 校验泛参是否满足约束, 实例化
	 */
	Instance Instantiate_gparam_param_return(VerifyContext& ctx, std::vector<TypeId> gparams_tid, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid);
	/*
	 * 实例化
	 * {泛参实际类型, 函数类型} => 推导出泛型参数, 校验泛参是否满足约束, 实例化
	 */
	Instance Instantiate_gparam_type(VerifyContext& ctx, std::vector<TypeId> gparams_tid, TypeId fn_tid);

private:
	AstNodeComplexFnDef() {}
	void infer_by_param_return(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const;
	void infer_by_type(VerifyContext& ctx, TypeId fn_tid, Instance& instance) const;
	void infer_by_gparam_param_return(VerifyContext& ctx, std::vector<TypeId> gparams_tid, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const;
	void infer_by_gparam_type(VerifyContext& ctx, std::vector<TypeId> gparams_tid, TypeId fn_tid, Instance& instance) const;

	void instantiate(VerifyContext& ctx, Instance& instance);

	bool get_instance(Instance& instance) const;

	// 将实例添加到vt的合适位置
	void add_instance_to_vt(VerifyContext& ctx, std::string name, FunctionObj fnobj) const;

private:
	std::string			   m_fnname;
	std::vector<Implement> m_implements;

	std::vector<Instance> m_instances;
};

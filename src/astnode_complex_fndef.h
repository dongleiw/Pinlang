#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 函数的复杂定义
 * 规则:
 *		1. 如果有多个参数个数相同的实现, 这些实现必须都不是泛型.
 */
class AstNodeComplexFnDef : public AstNode {
public:
	class Instance;
	// 函数的一个定义
	struct Implement {
		Implement(std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body) {
			m_generic_params = generic_params;
			m_params		 = params;
			m_return_type	 = return_type;
			m_return_tid	 = TYPE_ID_NONE;
			m_body			 = body;
			m_verify_cb		 = nullptr;
			m_execute_cb	 = nullptr;
		}
		Implement(std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, BuiltinFnVerifyCallback verify_cb, BuiltinFnExecuteCallback execute_cb) {
			m_generic_params = generic_params;
			m_params		 = params;
			m_return_type	 = return_type;
			m_return_tid	 = TYPE_ID_NONE;
			m_body			 = nullptr;
			m_verify_cb		 = verify_cb;
			m_execute_cb	 = execute_cb;
		}

		void					 Verify(VerifyContext& ctx);
		Implement				 DeepClone();
		bool					 is_generic_param(std::string name) const;
		std::vector<TypeId>		 infer_gparams_by_param_return(std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) const;
		bool					 satisfy_constraint(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const;
		TypeId					 infer_return_type_by_gparams(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const;
		std::vector<std::string> GetGParamsName() const;
		bool					 is_generic() const { return !m_generic_params.empty(); };

	public:
		std::vector<ParserGenericParam> m_generic_params;
		std::vector<ParserParameter>	m_params;
		AstNodeType*					m_return_type;
		AstNodeBlockStmt*				m_body;
		BuiltinFnVerifyCallback			m_verify_cb;
		BuiltinFnExecuteCallback		m_execute_cb;

		// 如果该实现不是泛型(也就是参数类型和返回值类型是固定的), 则根据上下文推导出以下两个信息
		std::vector<TypeId> m_params_tid;
		TypeId				m_return_tid;
	};
	// 函数的一个实例
	struct Instance {
		const Implement*	implement;
		std::vector<TypeId> gparams_tid;
		std::vector<TypeId> params_tid;
		TypeId				return_tid;
		std::string			instance_name;
		// Function*			fn;
		FnAddr fn_addr;
	};

public:
	AstNodeComplexFnDef(std::string fn_name, std::vector<Implement> implements);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*	 DeepClone() override { return DeepCloneT(); }
	AstNodeComplexFnDef* DeepCloneT();

	std::string GetName() const { return m_fnname; }
	void		SetObjTypeId(TypeId obj_tid) { m_obj_tid = obj_tid; }

	/*
	 * 实例化
	 * {参数, 返回值} => 推导出泛型参数, 校验泛参是否满足约束, 实例化
	 * concrete_return_tid 函数实际返回类型, 可以为TYPE_ID_INFER
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
	/*
	 * 实例化
	 * 由于没有提供参数,返回值等信息, 无法进行类型推导. 如果有多个implement, 或者有一个泛型implement时, 会panic
	 */
	Instance Instantiate(VerifyContext& ctx);

private:
	AstNodeComplexFnDef() {}
	void infer_by_param_return(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const;
	void infer_by_type(VerifyContext& ctx, TypeId fn_tid, Instance& instance) const;
	void infer_by_gparam_param_return(VerifyContext& ctx, std::vector<TypeId> gparams_tid, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const;
	void infer_by_gparam_type(VerifyContext& ctx, std::vector<TypeId> gparams_tid, TypeId fn_tid, Instance& instance) const;

	void instantiate(VerifyContext& ctx, Instance& instance);

	bool get_instance(Instance& instance) const;

	// 将实例添加到vt的合适位置
	void add_instance_to_vt(VerifyContext& ctx, std::string name, TypeId fn_tid, FnAddr fn_addr) const;

private:
	std::string			   m_fnname;
	std::vector<Implement> m_implements;

	TypeId				  m_obj_tid;
	std::vector<Instance> m_instances;
};

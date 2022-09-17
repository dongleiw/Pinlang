#include "astnode_generic_fndef.h"
#include "astnode_blockstmt.h"
#include "astnode_restriction.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_virtual_gtype.h"
#include "type_mgr.h"
#include "type_restriction.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>

AstNodeGenericFnDef::AstNodeGenericFnDef(std::string fn_name, std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body) {
	m_fnname		 = fn_name;
	m_generic_params = generic_params;
	m_params		 = params;
	m_return_type	 = return_type;
	m_body			 = body;
}
VerifyContextResult AstNodeGenericFnDef::Verify(VerifyContext& ctx) {
	log_debug("begin to verify generic fndef: fnname[%s]", m_fnname.c_str());

	verify_body(ctx);

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_fnname, new Variable(this));
	m_result_typeid = TYPE_ID_GENERIC_FN;
	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeGenericFnDef::Execute(ExecuteContext& ctx) {
	for (auto iter : m_instances) {
		ctx.GetCurStack()->GetCurVariableTable()->AddVariable(iter.first, new Variable(iter.second));
	}
	return nullptr;
}
std::string AstNodeGenericFnDef::Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) {
	if (concrete_params_tid.size() != m_params.size()) {
		panicf("concrete params number != generic params number");
	}

	std::map<std::string, TypeId> infer_result;

	// 根据参数类型推断
	for (size_t i = 0; i < concrete_params_tid.size(); i++) {
		TypeId param_tid = concrete_params_tid.at(i);
		for (auto infer : m_params.at(i).type->InferType(param_tid)) {
			auto found = infer_result.find(infer.first);
			if (found == infer_result.end()) {
				infer_result[infer.first] = infer.second;
			} else {
				if (found->second != infer.second) {
					panicf("failed to infer argument[%lu:%s]: generic param[%s] infered as[%s] and [%s]", i + 1, m_params.at(i).name.c_str(), infer.first.c_str(), GET_TYPENAME_C(found->second), GET_TYPENAME_C(infer.second));
				}
			}
		}
	}

	// 根据返回值类型推断
	if (m_return_type != nullptr && concrete_return_tid != TYPE_ID_INFER) {
		for (auto infer : m_return_type->InferType(concrete_return_tid)) {
			auto found = infer_result.find(infer.first);
			if (found == infer_result.end()) {
				infer_result[infer.first] = infer.second;
			} else {
				if (found->second != infer.second) {
					panicf("failed to infer return: generic param[%s] infered as[%s] and [%s]", infer.first.c_str(), GET_TYPENAME_C(found->second), GET_TYPENAME_C(infer.second));
				}
			}
		}
	}

	// 检查所有泛型参数是否都已经推断出来了
	for (auto generic_param : m_generic_params) {
		auto found = infer_result.find(generic_param.type_name);
		if (found == infer_result.end()) {
			panicf("failed to infer generic_param[%s]", generic_param.type_name.c_str());
		}
		log_debug("infer: generic_param[%s] -> concrete_type[%d:%s]", generic_param.type_name.c_str(), found->second, GET_TYPENAME_C(found->second));
	}

	std::vector<TypeId> concrete_gparams;
	for (auto generic_param : m_generic_params) {
		concrete_gparams.push_back(infer_result[generic_param.type_name]);
	}
	return instantiate(ctx, concrete_gparams);
}
void AstNodeGenericFnDef::verify_restriction(VerifyContext& ctx, std::vector<TypeId> concrete_gparams) const {
	assert(concrete_gparams.size() == m_generic_params.size());
	// TODO 检查所有泛型参数的实际类型是否满足约束
	for (size_t i = 0; i < concrete_gparams.size(); i++) {
		const TypeId			  concrete_gparam_tid = concrete_gparams.at(i);
		const ParserGenericParam& generic_param		  = m_generic_params.at(i);

		Variable* v = ctx.GetCurStack()->GetVariable(generic_param.restriction_name);
		if (v->GetTypeId() == TYPE_ID_GENERIC_RESTRICTION) {
			// restriction本身也是泛型. 需要先实例化restriction

			// 创建vt, 将(泛型名=>实际类型id)定义到vt中
			VariableTable* vt = new VariableTable();
			for (size_t j = 0; j < m_generic_params.size(); j++) {
				vt->AddVariable(m_generic_params.at(j).type_name, Variable::CreateTypeVariable(concrete_gparams.at(j)));
			}

			// 推导出该约束的实际类型
			std::vector<TypeId> restriction_concrete_gparams;
			ctx.GetCurStack()->EnterBlock(vt);
			for (auto iter : generic_param.restriction_generic_params) {
				restriction_concrete_gparams.push_back(iter->Verify(ctx).GetResultTypeId());
			}
			ctx.GetCurStack()->LeaveBlock();

			AstNodeRestriction* astnode_restriction = v->GetValueRestriction();
			TypeId				restriction_tid		= astnode_restriction->Instantiate(ctx, restriction_concrete_gparams);

			TypeInfo* ti = g_typemgr.GetTypeInfo(concrete_gparam_tid);
			if (!ti->MatchRestriction(restriction_tid)) {
				panicf("type[%d:%s] not implement restriction[%s]", concrete_gparam_tid, GET_TYPENAME_C(concrete_gparam_tid), generic_param.restriction_name.c_str());
			}
		} else {
			panicf("not implemented");
		}
	}
}
std::string AstNodeGenericFnDef::instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_gparams) {
	assert(concrete_gparams.size() == m_generic_params.size());

	verify_restriction(ctx, concrete_gparams);

	std::vector<TypeId> params;
	VariableTable*		vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i).type_name, Variable::CreateTypeVariable(concrete_gparams.at(i)));
	}

	ctx.GetCurStack()->EnterBlock(vt);
	std::vector<std::string> params_name;
	for (auto iter : m_params) {
		params.push_back(iter.type->Verify(ctx).GetResultTypeId());
		params_name.push_back(iter.name);
	}
	TypeId return_tid = m_return_type->Verify(ctx).GetResultTypeId();
	ctx.GetCurStack()->LeaveBlock();

	TypeId fn_tid = g_typemgr.GetOrAddTypeFn(params, return_tid);

	Function* fn = new Function(fn_tid, params_name, m_body);

	std::string fn_instance_name = TypeInfoFn::GetUniqFnName(m_fnname, concrete_gparams, params);

	m_instances[fn_instance_name] = fn;

	add_instance_to_vt(ctx, fn_instance_name, fn);
	return fn_instance_name;
}
void AstNodeGenericFnDef::add_instance_to_vt(VerifyContext& ctx, std::string name, Function* fn) const {
	VariableTable* vt = ctx.GetCurStack()->GetVariableTableByVarName(m_fnname);
	if (vt == nullptr) {
		panicf("generic_fn[%s] not found", m_fnname.c_str());
	}
	vt->AddVariable(name, new Variable(fn));
}
void AstNodeGenericFnDef::verify_body(VerifyContext& ctx) {
	log_debug("begin to verify body of generic function name[%s]", m_fnname.c_str());

	// 创建虚拟类型
	std::map<std::string, TypeInfoVirtualGType*> virtual_gparams;
	for (auto iter : m_generic_params) {
		TypeInfoVirtualGType* ti			= new TypeInfoVirtualGType(iter.type_name);
		virtual_gparams[iter.type_name] = ti;
		g_typemgr.AddTypeInfo(ti);
	}

	// 填充虚拟类型
	for (auto generic_param : m_generic_params) {
		// 创建vt, 将(泛型名=>虚拟类型id)定义到vt中
		VariableTable* vt = new VariableTable();
		for (auto virtual_gparam : virtual_gparams) {
			vt->AddVariable(virtual_gparam.first, Variable::CreateTypeVariable(virtual_gparam.second->GetTypeId()));
		}

		// 推导出该约束的实际类型
		std::vector<TypeId> restriction_concrete_gparams;
		ctx.GetCurStack()->EnterBlock(vt);
		for (auto iter : generic_param.restriction_generic_params) {
			restriction_concrete_gparams.push_back(iter->Verify(ctx).GetResultTypeId());
		}
		ctx.GetCurStack()->LeaveBlock();

		// 实例化约束
		Variable*			restriction_v			 = ctx.GetCurStack()->GetVariable(generic_param.restriction_name);
		AstNodeRestriction* astnode_restriction		 = restriction_v->GetValueRestriction();
		TypeId				restriction_instance_tid = astnode_restriction->Instantiate(ctx, restriction_concrete_gparams);

		// 根据restriction实例, 填充虚拟类型的方法, 使得虚拟类型满足该约束
		TypeInfoRestriction* restriction_instance = dynamic_cast<TypeInfoRestriction*>(g_typemgr.GetTypeInfo(restriction_instance_tid));
		restriction_instance->FillVirtualType(*virtual_gparams[generic_param.type_name]);
	}

	// 根据虚拟类型, 推导出方法的参数类型
	std::vector<TypeId> fn_params_tid;
	for (auto iter : m_params) {
		// 创建vt, 将(泛型名=>虚拟类型id)定义到vt中
		VariableTable* vt = new VariableTable();
		for (auto virtual_gparam : virtual_gparams) {
			vt->AddVariable(virtual_gparam.first, Variable::CreateTypeVariable(virtual_gparam.second->GetTypeId()));
		}
		ctx.GetCurStack()->EnterBlock(vt);
		TypeId param_tid = iter.type->Verify(ctx).GetResultTypeId();
		ctx.GetCurStack()->LeaveBlock();

		fn_params_tid.push_back(param_tid);
	}
	assert(fn_params_tid.size() == m_params.size());

	// 根据虚拟类型, 推导出函数的返回类型
	TypeId return_tid = TYPE_ID_NONE;
	{
		// 创建vt, 将(泛型名=>虚拟类型id)定义到vt中
		VariableTable* vt = new VariableTable();
		for (auto virtual_gparam : virtual_gparams) {
			vt->AddVariable(virtual_gparam.first, Variable::CreateTypeVariable(virtual_gparam.second->GetTypeId()));
		}
		ctx.GetCurStack()->EnterBlock(vt);
		return_tid = m_return_type->Verify(ctx).GetResultTypeId();
		ctx.GetCurStack()->LeaveBlock();
	}

	// 创建参数vt, 将参数定义到vt中
	VariableTable* params_vt = new VariableTable();
	for (size_t i = 0; i < m_params.size(); i++) {
		params_vt->AddVariable(m_params.at(i).name, new Variable(fn_params_tid.at(i)));
	}

	// 校验body
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(params_vt);
	ctx.GetParam().Clear();
	ctx.GetParam().SetReturnTid(return_tid);
	m_body->Verify(ctx);
	ctx.PopSTack();
}

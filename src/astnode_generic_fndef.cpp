#include "astnode_generic_fndef.h"
#include "astnode_blockstmt.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_virtual_gtype.h"
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

	m_body->SetParent(this);
}
VerifyContextResult AstNodeGenericFnDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("begin to verify generic fndef: fnname[%s]", m_fnname.c_str());

	verify_body(ctx);

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_fnname, new Variable(this));
	m_result_typeid = TYPE_ID_GENERIC_FN;
	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeGenericFnDef::Execute(ExecuteContext& ctx) {
	for (auto instance : m_instances) {
		ctx.GetCurStack()->GetCurVariableTable()->AddVariable(instance.instance_name, new Variable(instance.fnobj));
	}
	return nullptr;
}

AstNodeGenericFnDef::InstantiateParam AstNodeGenericFnDef::infer_by_param_type_and_return_type(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) const {
	if (concrete_params_tid.size() != m_params.size()) {
		panicf("concrete params number != generic params number");
	}

	InstantiateParam infer_result;

	// 根据参数类型推断
	for (size_t i = 0; i < concrete_params_tid.size(); i++) {
		TypeId param_tid = concrete_params_tid.at(i);
		for (auto infer : m_params.at(i).type->InferType(param_tid)) {
			if (!is_generic_param(infer.first))
				continue;
			auto found = infer_result.map_gparams_tid.find(infer.first);
			if (found == infer_result.map_gparams_tid.end()) {
				infer_result.map_gparams_tid[infer.first] = infer.second;
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
			if (!is_generic_param(infer.first))
				continue;
			auto found = infer_result.map_gparams_tid.find(infer.first);
			if (found == infer_result.map_gparams_tid.end()) {
				infer_result.map_gparams_tid[infer.first] = infer.second;
			} else {
				if (found->second != infer.second) {
					panicf("failed to infer return: generic param[%s] infered as[%s] and [%s]", infer.first.c_str(), GET_TYPENAME_C(found->second), GET_TYPENAME_C(infer.second));
				}
			}
		}
	}

	// 检查所有泛型参数是否都已经推断出来了
	for (auto generic_param : m_generic_params) {
		auto found = infer_result.map_gparams_tid.find(generic_param.type_name);
		if (found == infer_result.map_gparams_tid.end()) {
			panicf("failed to infer generic_param[%s]", generic_param.type_name.c_str());
		}
		log_debug("infer: generic_param[%s] -> concrete_type[%d:%s]", generic_param.type_name.c_str(), found->second, GET_TYPENAME_C(found->second));
	}

	// 保存泛型类型的实际类型
	for (auto iter : infer_result.map_gparams_tid) {
		infer_result.vec_gparams_tid.push_back(iter.second);
	}

	infer_result.params_tid = concrete_params_tid;

	// 注意传入的返回类型可能需要推导
	if (concrete_return_tid != TYPE_ID_INFER) {
		infer_result.return_tid = concrete_return_tid;
	} else {
		// 根据虚拟类型, 推导出函数的返回类型
		VariableTable* vt = new VariableTable();
		for (size_t i = 0; i < m_generic_params.size(); i++) {
			vt->AddVariable(m_generic_params.at(i).type_name, Variable::CreateTypeVariable(infer_result.vec_gparams_tid.at(i)));
		}
		ctx.GetCurStack()->EnterBlock(vt);
		if (m_return_type == nullptr) {
			infer_result.return_tid = TYPE_ID_NONE;
		} else {
			infer_result.return_tid = m_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}
		ctx.GetCurStack()->LeaveBlock();
	}

	return infer_result;
}
AstNodeGenericFnDef::InstantiateParam AstNodeGenericFnDef::infer_by_gparams(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const {
	assert(gparams_tid.size() == m_generic_params.size());

	InstantiateParam infer_result;

	// 创建vt, 将(泛型名=>虚拟类型id)定义到vt中
	VariableTable* vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i).type_name, Variable::CreateTypeVariable(gparams_tid.at(i)));
	}

	ctx.GetCurStack()->EnterBlock(vt);
	{
		// 根据泛型类型, 推导出方法的参数类型
		for (auto iter : m_params) {
			TypeId param_tid = iter.type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
			infer_result.params_tid.push_back(param_tid);
		}

		// 根据虚拟类型, 推导出函数的返回类型
		if (m_return_type == nullptr) {
			infer_result.return_tid = TYPE_ID_NONE;
		} else {
			infer_result.return_tid = m_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}
	}
	ctx.GetCurStack()->LeaveBlock();

	for (size_t i = 0; i < gparams_tid.size(); i++) {
		infer_result.map_gparams_tid[m_generic_params.at(i).type_name] = gparams_tid.at(i);
	}
	infer_result.vec_gparams_tid = gparams_tid;
	return infer_result;
}
AstNodeGenericFnDef::InstantiateParam AstNodeGenericFnDef::infer_by_typeid(VerifyContext& ctx, TypeId tid) const {
	// 根据函数类型获取参数类型和返回类型
	TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(tid));

	std::vector<TypeId> params_tid;
	for (size_t i = 0; i < ti->GetParamNum(); i++) {
		params_tid.push_back(ti->GetParamType(i));
	}
	return infer_by_param_type_and_return_type(ctx, params_tid, ti->GetReturnTypeId());
}
AstNodeGenericFnDef::Instance AstNodeGenericFnDef::Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) {
	if (concrete_params_tid.size() != m_params.size()) {
		panicf("concrete params number != generic params number");
	}

	InstantiateParam instantiate_param = infer_by_param_type_and_return_type(ctx, concrete_params_tid, concrete_return_tid);

	return instantiate(ctx, instantiate_param);
}
AstNodeGenericFnDef::Instance AstNodeGenericFnDef::Instantiate(VerifyContext& ctx, TypeId tid) {
	InstantiateParam instantiate_param = infer_by_typeid(ctx, tid);

	return instantiate(ctx, instantiate_param);
}
void AstNodeGenericFnDef::verify_constraint(VerifyContext& ctx, std::vector<TypeId> concrete_gparams) const {
	assert(concrete_gparams.size() == m_generic_params.size());
	// TODO 检查所有泛型参数的实际类型是否满足约束
	for (size_t i = 0; i < concrete_gparams.size(); i++) {
		const TypeId			  concrete_gparam_tid = concrete_gparams.at(i);
		const ParserGenericParam& generic_param		  = m_generic_params.at(i);

		Variable* v = ctx.GetCurStack()->GetVariable(generic_param.constraint_name);
		if (v->GetTypeId() == TYPE_ID_GENERIC_CONSTRAINT) {
			// constraint本身也是泛型. 需要先实例化constraint

			// 创建vt, 将(泛型名=>实际类型id)定义到vt中
			VariableTable* vt = new VariableTable();
			for (size_t j = 0; j < m_generic_params.size(); j++) {
				vt->AddVariable(m_generic_params.at(j).type_name, Variable::CreateTypeVariable(concrete_gparams.at(j)));
			}

			// 推导出该约束的实际类型
			std::vector<TypeId> constraint_concrete_gparams;
			ctx.GetCurStack()->EnterBlock(vt);
			for (auto iter : generic_param.constraint_generic_params) {
				constraint_concrete_gparams.push_back(iter->Verify(ctx, VerifyContextParam()).GetResultTypeId());
			}
			ctx.GetCurStack()->LeaveBlock();

			AstNodeConstraint* astnode_constraint = v->GetValueConstraint();
			TypeId			   constraint_tid	  = astnode_constraint->Instantiate(ctx, constraint_concrete_gparams);

			TypeInfo* ti = g_typemgr.GetTypeInfo(concrete_gparam_tid);
			if (!ti->MatchConstraint(constraint_tid)) {
				panicf("type[%d:%s] not implement constraint[%s]", concrete_gparam_tid, GET_TYPENAME_C(concrete_gparam_tid), generic_param.constraint_name.c_str());
			}
		} else {
			panicf("not implemented");
		}
	}
}
bool AstNodeGenericFnDef::get_instance(std::vector<TypeId> concrete_gparams, Instance* out_instance) const {
	assert(concrete_gparams.size() == m_generic_params.size());

	for (auto instance : m_instances) {
		bool equal = true;
		for (size_t i = 0; i < concrete_gparams.size(); i++) {
			if (instance.gparams_tid.at(i) != concrete_gparams.at(i)) {
				equal = false;
				break;
			}
		}
		if (equal) {
			if (out_instance != nullptr)
				*out_instance = instance;
			return true;
		}
	}
	return false;
}
AstNodeGenericFnDef::Instance AstNodeGenericFnDef::instantiate(VerifyContext& ctx, InstantiateParam instantiate_param) {
	assert(instantiate_param.vec_gparams_tid.size() == m_generic_params.size());

	Instance instance;
	if (get_instance(instantiate_param.vec_gparams_tid, &instance)) {
		return instance;
	}

	verify_constraint(ctx, instantiate_param.vec_gparams_tid);

	std::vector<std::string> params_name;
	for (auto iter : m_params) {
		params_name.push_back(iter.name);
	}

	instance.gparams_tid   = instantiate_param.vec_gparams_tid;
	instance.instance_name = TypeInfoFn::GetUniqFnName(m_fnname, instantiate_param.vec_gparams_tid, instantiate_param.params_tid);
	TypeId	  fn_tid	   = g_typemgr.GetOrAddTypeFn(instantiate_param.params_tid, instantiate_param.return_tid);
	Function* fn		   = new Function(fn_tid, params_name, m_body->DeepCloneT());
	instance.fnobj		   = FunctionObj(nullptr, fn);

	fn->Verify(ctx);

	m_instances.push_back(instance);
	add_instance_to_vt(ctx, instance.instance_name, instance.fnobj);
	return instance;
}
void AstNodeGenericFnDef::add_instance_to_vt(VerifyContext& ctx, std::string name, FunctionObj fnobj) const {
	VariableTable* vt = ctx.GetCurStack()->GetVariableTableByVarName(m_fnname);
	if (vt == nullptr) {
		panicf("generic_fn[%s] not found", m_fnname.c_str());
	}
	vt->AddVariable(name, new Variable(fnobj));
}
void AstNodeGenericFnDef::verify_body(VerifyContext& ctx) {
	log_debug("begin to verify body of generic function name[%s]", m_fnname.c_str());

	// 创建虚拟类型
	std::map<std::string, TypeInfoVirtualGType*> virtual_gparams;
	for (auto iter : m_generic_params) {
		TypeInfoVirtualGType* ti		= new TypeInfoVirtualGType(iter.type_name);
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
		std::vector<TypeId> constraint_concrete_gparams;
		ctx.GetCurStack()->EnterBlock(vt);
		for (auto iter : generic_param.constraint_generic_params) {
			constraint_concrete_gparams.push_back(iter->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		ctx.GetCurStack()->LeaveBlock();

		// 实例化约束
		Variable*		   constraint_v			   = ctx.GetCurStack()->GetVariable(generic_param.constraint_name);
		AstNodeConstraint* astnode_constraint	   = constraint_v->GetValueConstraint();
		TypeId			   constraint_instance_tid = astnode_constraint->Instantiate(ctx, constraint_concrete_gparams);

		// 根据constraint实例, 填充虚拟类型的方法, 使得虚拟类型满足该约束
		TypeInfoConstraint* constraint_instance = dynamic_cast<TypeInfoConstraint*>(g_typemgr.GetTypeInfo(constraint_instance_tid));
		constraint_instance->FillVirtualType(*virtual_gparams[generic_param.type_name]);
	}

	std::vector<TypeId> vec_gparams_tid;
	for (auto iter : virtual_gparams) {
		vec_gparams_tid.push_back(iter.second->GetTypeId());
	}
	InstantiateParam instantiate_param = infer_by_gparams(ctx, vec_gparams_tid);

	// 创建参数vt, 将参数定义到vt中
	VariableTable* params_vt = new VariableTable();
	for (size_t i = 0; i < m_params.size(); i++) {
		params_vt->AddVariable(m_params.at(i).name, new Variable(instantiate_param.params_tid.at(i)));
	}

	// 校验body
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(params_vt);
	m_body->Verify(ctx, VerifyContextParam().SetReturnTid(instantiate_param.return_tid));
	ctx.PopSTack();
}
AstNodeGenericFnDef::Instance AstNodeGenericFnDef::Instantiate(VerifyContext& ctx, std::vector<TypeId> gparams_tid) {
	InstantiateParam instantiate_param = infer_by_gparams(ctx, gparams_tid);
	return instantiate(ctx, instantiate_param);
}
AstNodeGenericFnDef* AstNodeGenericFnDef::DeepCloneT() {
	AstNodeGenericFnDef* newone = new AstNodeGenericFnDef();

	newone->m_fnname = m_fnname;
	for (auto iter : m_generic_params) {
		newone->m_generic_params.push_back(iter.DeepClone());
	}
	for (auto iter : m_params) {
		newone->m_params.push_back(iter.DeepClone());
	}
	if (m_return_type != nullptr)
		newone->m_return_type = m_return_type->DeepCloneT();
	newone->m_body = m_body->DeepCloneT();

	return newone;
}
bool AstNodeGenericFnDef::is_generic_param(std::string name) const {
	for (auto iter : m_generic_params) {
		if (iter.type_name == name)
			return true;
	}
	return false;
}

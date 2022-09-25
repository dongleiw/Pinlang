#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

AstNodeComplexFnDef::AstNodeComplexFnDef(std::string fn_name, std::vector<Implement> implements) {
	m_fnname	 = fn_name;
	m_implements = implements;
}
AstNodeComplexFnDef* AstNodeComplexFnDef::DeepCloneT() {
	AstNodeComplexFnDef* newone = new AstNodeComplexFnDef();

	for (auto iter : m_implements) {
		newone->m_implements.push_back(iter.DeepClone());
	}

	return newone;
}
AstNodeComplexFnDef::Implement AstNodeComplexFnDef::Implement::DeepClone() {
	Implement implement;

	for (auto iter : m_generic_params) {
		implement.m_generic_params.push_back(iter.DeepClone());
	}
	for (auto iter : m_params) {
		implement.m_params.push_back(iter.DeepClone());
	}
	if (m_return_type != nullptr) {
		implement.m_return_type = m_return_type->DeepCloneT();
	}
	implement.m_body			 = m_body == nullptr ? nullptr : m_body->DeepCloneT();
	implement.m_builtin_callback = m_builtin_callback;

	return implement;
}
VerifyContextResult AstNodeComplexFnDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	std::map<int, std::vector<const Implement*>> map_param_num_2_list;
	for (const Implement& iter : m_implements) {
		auto found = map_param_num_2_list.find(iter.m_params.size());
		if (found == map_param_num_2_list.end()) {
			std::vector<const Implement*> vec;
			vec.push_back(&iter);
			map_param_num_2_list[iter.m_params.size()] = vec;
		} else {
			found->second.push_back(&iter);
		}
	}

	//检查: 如果有多个参数个数相同的定义, 这些定义必须都是非泛型定义
	for (auto list : map_param_num_2_list) {
		if (list.second.size() > 1) {
			for (auto implement : list.second) {
				if (!implement->m_generic_params.empty()) {
					panicf("rule is not satisfied");
				}
			}
		}
	}

	// TODO 检查: 参数个数相同的多个实现, 相互之间参数类型是不同的

	for (auto& iter : m_implements) {
		iter.Verify(ctx);
	}

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_fnname, new Variable(this));

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeComplexFnDef::Execute(ExecuteContext& ctx) {
	for (auto instance : m_instances) {
		ctx.GetCurStack()->GetCurVariableTable()->AddVariable(instance.instance_name, new Variable(instance.fnobj));
	}
	return nullptr;
}
AstNodeComplexFnDef::Instance AstNodeComplexFnDef::Instantiate_param_return(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) {
	Instance instance;
	infer_by_param_return(ctx, concrete_params_tid, concrete_return_tid, instance);
	instantiate(ctx, instance);
	return instance;
}
AstNodeComplexFnDef::Instance AstNodeComplexFnDef::Instantiate_type(VerifyContext& ctx, TypeId fn_tid) {
	Instance instance;
	infer_by_type(ctx, fn_tid, instance);
	instantiate(ctx, instance);
	return instance;
}
AstNodeComplexFnDef::Instance AstNodeComplexFnDef::Instantiate_gparam_param_return(VerifyContext& ctx, std::vector<TypeId> gparams_tid, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) {
	Instance instance;
	infer_by_gparam_param_return(ctx, gparams_tid, concrete_params_tid, concrete_return_tid, instance);
	instantiate(ctx, instance);
	return instance;
}
AstNodeComplexFnDef::Instance AstNodeComplexFnDef::Instantiate_gparam_type(VerifyContext& ctx, std::vector<TypeId> gparams_tid, TypeId fn_tid) {
	Instance instance;
	infer_by_gparam_type(ctx, gparams_tid, fn_tid, instance);
	instantiate(ctx, instance);
	return instance;
}
void AstNodeComplexFnDef::infer_by_param_return(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const {
	// 找到所有参数数量相等的implement
	std::vector<const Implement*> param_number_match_list;
	for (auto& implement : m_implements) {
		if (implement.m_params.size() == concrete_params_tid.size()) {
			param_number_match_list.push_back(&implement);
		}
	}
	if (param_number_match_list.empty()) {
		panicf("no candidate match");
	}

	if (param_number_match_list.size() == 1 && !param_number_match_list.at(0)->m_generic_params.empty()) {
		// 匹配到一个且是泛型
		const Implement*	implement	= param_number_match_list.at(0);
		std::vector<TypeId> gparams_tid = implement->infer_gparams_by_param_return(concrete_params_tid, concrete_return_tid);
		if (!implement->satisfy_constraint(ctx, gparams_tid)) {
			panicf("not satisfy constraint");
		}
		TypeId implement_return_tid = implement->infer_return_type_by_gparams(ctx, gparams_tid);
		if (concrete_return_tid == TYPE_ID_INFER || concrete_return_tid == implement_return_tid) {
		} else {
			panicf("not match");
		}

		instance.gparams_tid = gparams_tid;
		instance.params_tid	 = concrete_params_tid;
		instance.return_tid	 = implement_return_tid;
		instance.implement	 = implement;
		return;
	} else {
		// 由于规则限制, 不可能有泛型了
		for (auto implement : param_number_match_list) {
			// 如果和传入的参数类型返回类型匹配, 则match成功
			if (is_vec_typeid_equal(implement->m_params_tid, concrete_params_tid) && (concrete_return_tid == TYPE_ID_INFER || concrete_return_tid == implement->m_return_tid)) {
				instance.gparams_tid.clear();
				instance.params_tid = implement->m_params_tid;
				instance.return_tid = implement->m_return_tid;
				instance.implement	= implement;
				return;
			}
		}
	}
	panicf("no candidate match");
}
std::vector<TypeId> AstNodeComplexFnDef::Implement::infer_gparams_by_param_return(std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid) const {
	// 根据参数类型推断
	std::map<std::string, TypeId> map_gparams_tid;
	for (size_t i = 0; i < concrete_params_tid.size(); i++) {
		TypeId param_tid = concrete_params_tid.at(i);
		for (auto infer : m_params.at(i).type->InferType(param_tid)) {
			if (!is_generic_param(infer.first))
				continue;
			auto found = map_gparams_tid.find(infer.first);
			if (found == map_gparams_tid.end()) {
				map_gparams_tid[infer.first] = infer.second;
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
			auto found = map_gparams_tid.find(infer.first);
			if (found == map_gparams_tid.end()) {
				map_gparams_tid[infer.first] = infer.second;
			} else {
				if (found->second != infer.second) {
					panicf("failed to infer return: generic param[%s] infered as[%s] and [%s]", infer.first.c_str(), GET_TYPENAME_C(found->second), GET_TYPENAME_C(infer.second));
				}
			}
		}
	}

	// 检查所有泛型参数是否都已经推断出来了
	for (auto generic_param : m_generic_params) {
		auto found = map_gparams_tid.find(generic_param.type_name);
		if (found == map_gparams_tid.end()) {
			panicf("failed to infer generic_param[%s]", generic_param.type_name.c_str());
		}
		log_debug("infer: generic_param[%s] -> concrete_type[%d:%s]", generic_param.type_name.c_str(), found->second, GET_TYPENAME_C(found->second));
	}

	// 按照顺序
	std::vector<TypeId> gparams_tid;
	for (auto generic_param : m_generic_params) {
		gparams_tid.push_back(map_gparams_tid.find(generic_param.type_name)->second);
	}
	return gparams_tid;
}
void AstNodeComplexFnDef::Implement::Verify(VerifyContext& ctx) {
	assert(m_params_tid.empty());
	if (m_generic_params.empty()) {
		// 得到该实现的参数类型和返回类型
		for (auto param : m_params) {
			m_params_tid.push_back(param.type->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		if (m_return_type != nullptr) {
			m_return_tid = m_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}
	}

	// 检查: 所有泛参的名字和全局变量名不冲突, 和参数名也不冲突
	for (auto iter : m_generic_params) {
		if (ctx.GetGlobalVt()->HasVariable(iter.type_name)) {
			panicf("generic type[%s] conflict with global variable", iter.type_name.c_str());
		}
		for (auto p : m_params) {
			if (p.name == iter.type_name) {
				panicf("generic type[%s] conflict with argument variable", iter.type_name.c_str());
			}
		}
	}
}
bool AstNodeComplexFnDef::Implement::is_generic_param(std::string name) const {
	for (auto iter : m_generic_params) {
		if (iter.type_name == name) {
			return true;
		}
	}
	return false;
}
TypeId AstNodeComplexFnDef::Implement::infer_return_type_by_gparams(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const {
	if (m_return_type == nullptr) {
		return TYPE_ID_NONE;
	}

	// 创建vt, 将(泛型名=>虚拟类型id)定义到vt中
	VariableTable* vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i).type_name, Variable::CreateTypeVariable(gparams_tid.at(i)));
	}
	ctx.GetCurStack()->EnterBlock(vt);
	TypeId return_tid = m_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	ctx.GetCurStack()->LeaveBlock();
	return return_tid;
}
bool AstNodeComplexFnDef::Implement::satisfy_constraint(VerifyContext& ctx, std::vector<TypeId> gparams_tid) const {
	assert(gparams_tid.size() == m_generic_params.size());
	// 检查所有泛型参数的实际类型是否满足约束
	for (size_t i = 0; i < gparams_tid.size(); i++) {
		const TypeId			  concrete_gparam_tid = gparams_tid.at(i);
		const ParserGenericParam& generic_param		  = m_generic_params.at(i);

		Variable* v = ctx.GetCurStack()->GetVariable(generic_param.constraint_name);
		if (v->GetTypeId() == TYPE_ID_GENERIC_CONSTRAINT) {
			// constraint本身也是泛型. 需要先实例化constraint

			// 创建vt, 将(泛型名=>实际类型id)定义到vt中
			VariableTable* vt = new VariableTable();
			for (size_t j = 0; j < m_generic_params.size(); j++) {
				vt->AddVariable(m_generic_params.at(j).type_name, Variable::CreateTypeVariable(concrete_gparam_tid));
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
	return true;
}
void AstNodeComplexFnDef::infer_by_type(VerifyContext& ctx, TypeId fn_tid, Instance& instance) const {
	TypeInfoFn*			tifn	   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
	std::vector<TypeId> params_tid = tifn->GetParmsTid();
	TypeId				return_tid = tifn->GetReturnTypeId();

	infer_by_param_return(ctx, params_tid, return_tid, instance);
}
void AstNodeComplexFnDef::infer_by_gparam_param_return(VerifyContext& ctx, std::vector<TypeId> gparams_tid, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid, Instance& instance) const {
	infer_by_param_return(ctx, concrete_params_tid, concrete_return_tid, instance);

	if (!is_vec_typeid_equal(gparams_tid, instance.gparams_tid)) {
		panicf("ambigous");
	}
}
void AstNodeComplexFnDef::infer_by_gparam_type(VerifyContext& ctx, std::vector<TypeId> gparams_tid, TypeId fn_tid, Instance& instance) const {
	TypeInfoFn*			tifn	   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
	std::vector<TypeId> params_tid = tifn->GetParmsTid();
	TypeId				return_tid = tifn->GetReturnTypeId();

	infer_by_gparam_param_return(ctx, gparams_tid, params_tid, return_tid, instance);
}
bool AstNodeComplexFnDef::get_instance(Instance& instance) const {
	for (auto iter : m_instances) {
		if (iter.implement == instance.implement && is_vec_typeid_equal(iter.gparams_tid, instance.gparams_tid) && is_vec_typeid_equal(iter.params_tid, instance.params_tid) && iter.return_tid == instance.return_tid) {
			instance = iter;
			return true;
		}
	}
	return false;
}
void AstNodeComplexFnDef::instantiate(VerifyContext& ctx, Instance& instance) {
	if (get_instance(instance)) {
		return;
	}

	std::vector<std::string> params_name;
	for (auto iter : instance.implement->m_params) {
		params_name.push_back(iter.name);
	}

	std::vector<ConcreteGParam> concrete_gparams;
	for (size_t i = 0; i < instance.gparams_tid.size(); i++) {
		concrete_gparams.push_back(ConcreteGParam{.gparam_name = instance.implement->m_generic_params.at(i).type_name, .gparam_tid = instance.gparams_tid.at(i)});
	}

	instance.instance_name = TypeInfoFn::GetUniqFnName(m_fnname, instance.gparams_tid, instance.params_tid);
	TypeId	  fn_tid	   = g_typemgr.GetOrAddTypeFn(instance.params_tid, instance.return_tid);
	Function* fn		   = nullptr;
	if (instance.implement->m_body != nullptr) {
		fn = new Function(fn_tid, concrete_gparams, params_name, instance.implement->m_body->DeepCloneT());
	} else {
		fn = new Function(fn_tid, concrete_gparams, instance.implement->m_builtin_callback);
	}
	instance.fnobj = FunctionObj(nullptr, fn);

	fn->Verify(ctx);

	m_instances.push_back(instance);
	add_instance_to_vt(ctx, instance.instance_name, instance.fnobj);
}
void AstNodeComplexFnDef::add_instance_to_vt(VerifyContext& ctx, std::string name, FunctionObj fnobj) const {
	VariableTable* vt = ctx.GetCurStack()->GetVariableTableByVarName(m_fnname);
	if (vt == nullptr) {
		panicf("generic_fn[%s] not found", m_fnname.c_str());
	}
	vt->AddVariable(name, new Variable(fnobj));
}
std::vector<std::string> AstNodeComplexFnDef::Implement::GetGParamsName() const {
	std::vector<std::string> gparams_name;
	for (auto iter : m_generic_params) {
		gparams_name.push_back(iter.type_name);
	}
	return gparams_name;
}
